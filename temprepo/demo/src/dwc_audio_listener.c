#include <stdio.h> /* for printf */
#include <string.h> /* for string operation */
#include <stdlib.h> /* for exit */
#include <math.h>
#include <signal.h> /* for signal */
#include <errno.h> /* for errno, stderr */
#include <syslog.h>
#include <unistd.h> /* for getopt*/
#include <arpa/inet.h> /* for htons */

#include <sys/ioctl.h> /* for ioctl */
#include <sys/socket.h> /* for socket */
#include <linux/if.h> /* for ioctl number */
#include <pci/pci.h> /* for pci calls */

#include <sys/mman.h> /* gptp */
#include <sys/stat.h> /* gptp: For mode constants */
#include <fcntl.h> /* gptp */
#include <pthread.h> /* posix threads */
#include <pcap/pcap.h> /* pcap library */

#include "DWC_ETH_QOS_ycommon.h" /* for local data structure */
#include "avtp.h"
#include "session_mngr.h"

#define MAX_FRAME_SIZE 1500
#define DWC_ETH_QOS_BIND_NAME_SIZE 24

/* Uncomment this to apply filter */
//#define PCAP_FILTER

/* global variables */
struct device dwc_eth_qos_pdev;
volatile int halt = 0;
uint8_t src_addr[] = {0,0,0,0,0,0};
pcap_t* handle;
unsigned long long now_local;
uint64_t total_delay = 0;
unsigned long long avb_bytes = 0;
unsigned long long non_avb_bytes = 0;
unsigned long long last_avb_bytes = 0;
unsigned long long last_non_avb_bytes = 0;

typedef struct { 
	int64_t ml_phoffset;
	int64_t ls_phoffset;
	int32_t ml_freqoffset;
	int32_t ls_freqoffset;
	int64_t local_time;
} gPtpTimeData;
#define SHM_SIZE 4*8 + sizeof(pthread_mutex_t) /* 3 - 64 bit and 2 - 32 bits */
#define SHM_NAME  "/ptp"
typedef enum { false = 0, true = 1 } bool;
static int shm_fd = -1;
static char *memory_offset_buffer = NULL;
gPtpTimeData td;

#define STDOUT 1

/* Uncomment this to print in multi-line */
//#define MULTILINE_PRINT

/*
 * This api scan all pci device and attach user space driver with
 * the scanned device. The user space driver is written for
 * pci device with vendor_id => 0x700 and device_id =>0x1018.
 *
 * returns zero on success else +ve error no on failure.
 * */
static int pci_connect()
{
	struct pci_access *pacc;
	struct pci_dev *pdev;
	char dev_path[DWC_ETH_QOS_BIND_NAME_SIZE];
	int ret = ENXIO;

	memset(&dwc_eth_qos_pdev, 0, sizeof(struct device));
	pacc = pci_alloc();
	pci_init(pacc);
	pci_scan_bus(pacc);

	for (pdev = pacc->devices; pdev; pdev = pdev->next) {
		pci_fill_info(pdev,
				PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_CLASS);
		dwc_eth_qos_pdev.vendor_id = pdev->vendor_id;
		dwc_eth_qos_pdev.device_id = pdev->device_id;
		dwc_eth_qos_pdev.domain = pdev->domain;
		dwc_eth_qos_pdev.bus = pdev->bus;
		dwc_eth_qos_pdev.dev = pdev->dev;
		dwc_eth_qos_pdev.func = pdev->func;
		snprintf(dev_path, DWC_ETH_QOS_BIND_NAME_SIZE,
				"%04x:%02x:%02x.%d", pdev->domain, pdev->bus,
				pdev->dev, pdev->func);
		ret = DWC_ETH_QOS_probe(&dwc_eth_qos_pdev);
		if (ret)
			continue;
		DEBUG_PRN("Attaching to %s\n", dev_path);
		ret = DWC_ETH_QOS_attach(dev_path, &dwc_eth_qos_pdev);
		if (ret)
			continue;

		DEBUG_PRN("Successfully attached\n");
		goto out;
	}
	pci_cleanup(pacc);

	return ENXIO;

out:
	pci_cleanup(pacc);

	return 0;
}


/* API to get given interface hw/mac address and store
 * it in global variable.
 *
 * return zero on success and -1 on failure
 * */
static int get_hw_address(char *ifname)
{
	struct ifreq ifr;
	int sockfd;
	int ret = 0;

	sockfd = socket(PF_PACKET, SOCK_RAW, htons(0x800));
	if (sockfd < 0) {
		DEBUG_PRN("failed to open %s interface\n", ifname);
		return -1;
	}

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	ret = ioctl(sockfd, SIOCGIFHWADDR, &ifr);
	if (ret < 0) {
		close(sockfd);
		DEBUG_PRN("ioctl to get hw address failed\n");
		return -1;
	}
	memcpy(src_addr, ifr.ifr_hwaddr.sa_data, sizeof(src_addr));
	close(sockfd);

	return ret;
}

void sigint_handler(int signum)
{
	DEBUG_PRN("\n\n\n\ngot SIGINT, signum = %d\n", signum);
	if (NULL != handle) {
		pcap_breakloop(handle);
		pcap_close(handle);
	}
	halt = signum;
}

int gptpinit(void)
{
	shm_fd = shm_open(SHM_NAME, O_RDWR, 0);
	if (shm_fd == -1) {
		perror("shm_open()");
		return false;
	}
	memory_offset_buffer =
		(char *)mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
				shm_fd, 0);
	if (memory_offset_buffer == (char *)-1) {
		perror("mmap()");
		memory_offset_buffer = NULL;
		shm_unlink(SHM_NAME);
		return false;
	}
	return true;
}

void gptpdeinit(void)
{
	if (memory_offset_buffer != NULL) {
		munmap(memory_offset_buffer, SHM_SIZE);
	}
	if (shm_fd != -1) {
		close(shm_fd);
	}
}

int gptpscaling(gPtpTimeData * td)
{
	pthread_mutex_lock((pthread_mutex_t *) memory_offset_buffer);
	memcpy(td, memory_offset_buffer + sizeof(pthread_mutex_t), sizeof(*td));
	pthread_mutex_unlock((pthread_mutex_t *) memory_offset_buffer);

	DEBUG_PRN( "ml_phoffset = %lld, ls_phoffset = %lld\n",
			td->ml_phoffset, td->ls_phoffset);
	DEBUG_PRN( "ml_freqffset = %d, ls_freqoffset = %d\n",
			td->ml_freqoffset, td->ls_freqoffset);

	return true;
}

int calculate_bandwidth(int bytes)
{
	unsigned int kilo/*, mega*/, bits;

	kilo = bytes / 1024;
	bits = kilo * 8;
	DEBUG_PRN("bytes=%d kilo= %d bits=%d\n",bytes,kilo, bits);

	return bits;
}

void timer_handler (int arg) 
{
	unsigned long long diff_avb = 0, diff_non_avb = 0;
	int avb_bw, non_avb_bw;

	diff_avb = avb_bytes - last_avb_bytes;
	diff_non_avb = non_avb_bytes - last_non_avb_bytes;
	avb_bw = calculate_bandwidth(diff_avb);
	non_avb_bw = calculate_bandwidth(diff_non_avb);
#ifdef MULTILINE_PRINT
	fprintf(stderr, "AVB bandwidth:%6d Kbps(%3d Mbps)  NON_AVB bandwidth:%6d Kbps(%3d Mbps)  PeerDelay:%6lld us\n",
						avb_bw, (avb_bw/1024), non_avb_bw, (non_avb_bw/1024), (total_delay/1000));
#else
	fprintf(stderr, "AVB bandwidth:%6d Kbps(%3d Mbps)  NON_AVB bandwidth:%6d Kbps(%3d Mbps)  PeerDelay:%6lld us\r",
						avb_bw, (avb_bw/1024), non_avb_bw, (non_avb_bw/1024), (total_delay/1000));
#endif
	last_avb_bytes = last_avb_bytes + diff_avb;
	last_non_avb_bytes = last_non_avb_bytes + diff_non_avb;
}

void pcap_callback(u_char* args, const struct pcap_pkthdr* packet_header, const u_char* packet)
{
	static uint32_t j, k;
	void *frame;
	eth_header* header;
	uint64_t temp, delay = 0;
	int datalen;
	uint8_t *dataptr = NULL;

	header = (eth_header*)(packet);

	frame = (void*)packet;
	if ((header->type[0] == 0x22) && (header->type[1] == 0xf0)) {
		avb_bytes = avb_bytes + packet_header->len;
		gptpscaling(&td);
		now_local = get_wallclock();
		delay = now_local - td.ml_phoffset;
		frame = (void *)packet;
		dataptr = get_61883_stream_payload_ptr(frame);
		datalen = get_61883_stream_payload_len(frame);
		memcpy(&temp,&dataptr[128],sizeof(delay));	

		if (j % 1000 == 0)
			total_delay = (delay-temp);
		DEBUG_PRN("fn=%d, rx=%lld, tx=%lld delay=%lld, total_delay=%lld\n",j, delay, temp,(delay - temp),total_delay);
		write(STDOUT, dataptr, (datalen - 8));
		j++;
	} else {
		non_avb_bytes = non_avb_bytes + packet_header->len;
		k++;
  }
}

int main(int argc, char *argv[])
{
        struct itimerval timer;
        char *ifname = NULL;
        int ret = 0;

        char errbuf[PCAP_ERRBUF_SIZE];
#ifdef PCAP_FILTER
        struct bpf_program comp_filter_exp;		/* The compiled filter expression */
        char filter_exp[] = "ether dst 91:E0:F0:00:0e:80";	/* The filter expression */
#endif

        if (argc < 2) {
                DEBUG_PRN("\nusage : %s <interface_name>\n\n",argv[0]);
                return -1;
        }

        ifname = strdup(argv[1]);
        if (gptpinit() == false) {
                DEBUG_PRN("start the gptp daemon\n");
                return -1;
        }

        ret = pci_connect();
        if (ret) {
                DEBUG_PRN("connect failed(%s) - are you running as root ?\n",
                                strerror(errno));
                return ret;
        }
        halt = 0;
        signal(SIGINT, sigint_handler);

        ret = DWC_ETH_QOS_init(&dwc_eth_qos_pdev);
        if (ret) {
                DEBUG_PRN("init failed(%s) - is the driver loaded ?\n",
                                strerror(errno));
                return ret;
        }
        DEBUG_PRN("device init done\n");
        ret = get_hw_address(ifname);
        if (ret < 0) {
                DEBUG_PRN("failed to get %s hw address\n", ifname);
                return -1;
        }
        DEBUG_PRN("got hw/mac address(%x:%x:%x:%x:%x:%x)\n",
                        src_addr[0], src_addr[1], src_addr[2], src_addr[3],
                        src_addr[4], src_addr[5]);

        now_local = get_wallclock();
        ret = nice(-20);

        /* take promiscuous vs. non-promiscuous sniffing? (0 or 1) */
        handle = pcap_open_live(ifname, BUFSIZ, 1, -1, errbuf);
        if (NULL == handle) {
                DEBUG_PRN( "Could not open device %s: %s\n", ifname, errbuf);
                return -1;
        }

#ifdef PCAP_FILTER
        /* compile and apply filter */
        if (-1 == pcap_compile(handle, &comp_filter_exp, filter_exp, 0, PCAP_NETMASK_UNKNOWN)) {
                DEBUG_PRN( "Could not parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
                return -1;
        }

        if (-1 == pcap_setfilter(handle, &comp_filter_exp)) {
                DEBUG_PRN( "Could not install filter %s: %s\n", filter_exp, pcap_geterr(handle));
                return -1;
        }
#endif

        timer.it_interval.tv_sec = 1;
        timer.it_interval.tv_usec = 0;
        /* set timer for "INTERVAL (10) seconds */
        timer.it_value.tv_sec = 1;
        timer.it_value.tv_usec = 0;
        setitimer(ITIMER_REAL, &timer, 0);
        signal(SIGALRM, timer_handler);

        /** loop forever and call callback-function for every received packet */
        system("clear");
        fprintf(stderr, "\n\n\n\n\n\n");
        pcap_loop(handle, -1, pcap_callback, NULL);

        ret = nice(0);
        DWC_ETH_QOS_detach(&dwc_eth_qos_pdev);
        gptpdeinit();

        return 0;
}
