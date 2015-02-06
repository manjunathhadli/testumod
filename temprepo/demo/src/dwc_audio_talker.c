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
#include <pthread.h>

#include "DWC_ETH_QOS_ycommon.h" /* for local data structure */
#include "avtp.h"
#include "session_mngr.h"

#define MAX_FRAME_SIZE 1500

#define ETH_HDR_LEN 14 /* without VLAN */

/* global variables */
struct device dwc_eth_qos_pdev;
//unsigned char dest_addr[6] = {0x00, 0x55, 0x7b, 0xb5, 0x7d, 0xf9}; //MAC address
unsigned char dest_addr[] = { 0x91, 0xE0, 0xF0, 0x00, 0x0e, 0x80 }; /* IEEE 1722 reserved address */
unsigned char src_addr[] = {0,0,0,0,0,0};
volatile int halt = 0;
volatile uint32_t a_sync = 1;
#define STDIN 0


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
#ifdef DEBUG
		DEBUG_PRN("Attaching to %s\n", dev_path);
#endif
		ret = DWC_ETH_QOS_attach(dev_path, &dwc_eth_qos_pdev);
		if (ret)
			continue;

#ifdef DEBUG
		DEBUG_PRN("Successfully attached\n");
#endif
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
	DEBUG_PRN("got SIGINT, signum = %d\n", signum);
	halt = signum;
}

#define SAMPLE_SIZE 4		/* 4 bytes */
#define SAMPLES_PER_FRAME 6
#define CHANNELS 2
#define MAX_SAMPLE_VALUE ((1U << ((sizeof(int32_t)*8)-1))-1)
#define GAIN (.5)

/* 61883 hardcoded values. */
#define SY 0
#define SRC_ID 0x3F
#define DBS 1 //data block size
#define FN 0 //fraction number
#define QPC 0
#define DBC 0
#define FMT 0
#define FDF 2
#define SYT 0xFFFF

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

        DEBUG_PRN(stderr, "ml_phoffset = %lld, ls_phoffset = %lld\n",
                        td->ml_phoffset, td->ls_phoffset);
        DEBUG_PRN(stderr, "ml_freqffset = %d, ls_freqoffset = %d\n",
                        td->ml_freqoffset, td->ls_freqoffset);

	return true;
}

int main(int argc, char *argv[])
{
	struct DWC_ETH_QOS_user_buff a_buffer;
	struct DWC_ETH_QOS_packet *tx_tmp_packet = NULL;
	struct DWC_ETH_QOS_packet *tx_free_packets = NULL;
	struct DWC_ETH_QOS_packet *tx_cleaned_packets = NULL;
	unsigned int transmitted = 0;
	char *ifname = NULL;
	unsigned int i, j;
	int qInx = 0;
	int ret = 0;
	int32_t frame_size, data_size;
        uint64_t delay = 0;
	uint8_t seqnum;
	int32_t total_tx_data_buf_size, total_samples = 0;
	unsigned long long now_local;
	uint8_t *frame;
	gPtpTimeData td;
	uint8_t *data_ptr;
        unsigned int avb_bw = 75; /* amount of bw to be allocated */

	if (argc < 3) {
		DEBUG_PRN("\nusage : %s <interface_name> <data_size> \n\n",argv[0]);
		return -1;
	}

	ifname = (char *)strdup(argv[1]);
	data_size = atoi(argv[2]);

	frame_size = data_size + sizeof(header_61883_pkt) +
		sizeof(header_1722_stream_data) + sizeof(eth_header) + 8;
	if (frame_size > MAX_FRAME_SIZE) {
		DEBUG_PRN("Invalid argument - data_size(%d)\n",
				data_size);
		return -1;
	}
#ifdef DEBUG
	DEBUG_PRN("frame_size = %d, data_size = %d\n",
			frame_size, data_size);
#endif

	if (gptpinit() == false) {
		DEBUG_PRN("start the gptp daemon\n");
		return -1;
	}
	gptpscaling(&td);

	ret = pci_connect();
	if (ret) {
		DEBUG_PRN("connect failed(%s) - are you running as root ?\n",
				strerror(errno));
		return ret;
	}

	halt = 0;
	signal(SIGINT, sigint_handler);

#ifdef DEBUG
	ret = DWC_ETH_QOS_test_reg_read(&dwc_eth_qos_pdev);
	if (ret) {
		DEBUG_PRN("device register read failed\n");
		return ret;
	}
	DEBUG_PRN("read device register test passed successfully\n");

	ret = DWC_ETH_QOS_test_reg_write(&dwc_eth_qos_pdev);
	if (ret) {
		DEBUG_PRN("device register write failed\n");
		return ret;
	}
	DEBUG_PRN("write device register passed successfully\n");
#endif

	ret = DWC_ETH_QOS_init(&dwc_eth_qos_pdev);
	if (ret) {
		DEBUG_PRN("init failed(%s) - is the driver loaded ?\n",
				strerror(errno));
		return ret;
	}
#ifdef DEBUG
	DEBUG_PRN("device init done\n");
#endif

	total_tx_data_buf_size = frame_size * TX_DESC_CNT;
	a_buffer.alloc_size = total_tx_data_buf_size;
	ret = DWC_ETH_QOS_get_buffer(&dwc_eth_qos_pdev, &a_buffer);
	if (ret) {
		DEBUG_PRN("tx data buffer allocation failed(%s)\n",
				strerror(errno));
		return ret;
	}
#ifdef DEBUG
	DEBUG_PRN("got tx data buffers(%d bytes)\n", a_buffer.mmap_size);
#endif

	ret = get_hw_address(ifname);
	if (ret < 0) {
		DEBUG_PRN("failed to get %s hw address\n", ifname);
		return -1;
	}
#ifdef DEBUG
	DEBUG_PRN("got hw/mac address(%x:%x:%x:%x:%x:%x)\n",
			src_addr[0], src_addr[1], src_addr[2], src_addr[3],
			src_addr[4], src_addr[5]);
#endif

        /* allocate CBS to AVB queue */
        ret = DWC_ETH_QOS_program_CBS_alogorithm(&dwc_eth_qos_pdev, qInx, avb_bw);
        if (ret)
                DEBUG_PRN("Failed to allocate CBS\n");

	/* divide the huge data buffer into small data buffer
	 * for each packets and create packet list.
	 * */
	frame = create_avtp_stream_pkt(E_61883_IIDC_SUBTYPE,data_size + 8);
	prepare_eth_header(frame, dest_addr, (uint8_t *)ifname, 3, 2);
	set_61883_stream_header_vals(frame,SY,SRC_ID,DBS,FN,QPC,DBC,FMT,FDF,SYT);
	add_stream_data_streamid(frame,src_addr,sizeof(src_addr));

	for (i = 0; i < (a_buffer.mmap_size)/frame_size; i++) {
		tx_tmp_packet = calloc(1, sizeof(struct DWC_ETH_QOS_packet));
		if (tx_tmp_packet == NULL) {
			DEBUG_PRN("failed to allocate wrapper packet\n");
			return errno;
		}

		tx_tmp_packet->map.paddr = a_buffer.dma_addr;
		tx_tmp_packet->map.mmap_size = a_buffer.mmap_size;

		tx_tmp_packet->offset = (i * frame_size);
		tx_tmp_packet->vaddr = a_buffer.addr + tx_tmp_packet->offset;
		tx_tmp_packet->next = tx_free_packets;

		memset(tx_tmp_packet->vaddr, 0, frame_size);
		memcpy(((char *)tx_tmp_packet->vaddr), frame, frame_size);

		tx_tmp_packet->len = frame_size;
		tx_free_packets = tx_tmp_packet;
	}
#ifdef DEBUG
	DEBUG_PRN("%d packets are ready to transmit\n", i);
#endif

	a_sync = 0;
	now_local = get_wallclock();
	ret = nice(-20);
	seqnum = 0;
        system("clear");
        fprintf(stderr, "Talker transmitting\n");
	/* transmits packets continuously untill tx is stopped */
	while (!halt) {
		tx_tmp_packet = tx_free_packets;
		if (tx_tmp_packet == NULL)
			goto cleanup;


                if (j % 1000 == 0)
                        fprintf(stderr, "-");

		frame = ((uint8_t *)tx_tmp_packet->vaddr);
		tx_free_packets = tx_tmp_packet->next;
		add_stream_data_seq_num(frame, seqnum++);
		if (seqnum % 4 == 0)
			set_stream_data_tv_field(frame, 0);
		else
			set_stream_data_tv_field(frame, 1);
		set_61883stream_data_boundary_continuity(frame, total_samples);
		data_ptr = (uint8_t *)get_61883_stream_payload_ptr(frame);

		total_samples += read(STDIN, data_ptr, data_size);

		gptpscaling(&td);
		now_local = get_wallclock();
		delay = now_local- td.ml_phoffset;
                DEBUG_PRN("pkt_no=%d, talker_ts=%lld, now_local_tx=%lld, offset=%lld\n ",j,delay,now_local,td.ml_phoffset);
		memcpy(data_ptr+data_size, &delay, sizeof(delay));
		add_stream_data_timestamp(frame, (now_local-td.ml_phoffset));
		ret = DWC_ETH_QOS_start_xmit(&dwc_eth_qos_pdev, qInx,
				tx_tmp_packet);
		if (!ret) {
			j++;
			continue;
		}

		if (ret == ENOSPC) {
			/* add it back to list now */
			tx_tmp_packet->next = tx_free_packets;
			tx_free_packets = tx_tmp_packet;
		}

cleanup:
		tx_cleaned_packets = NULL;
		ret = DWC_ETH_QOS_tx_buffer_cleanup(&dwc_eth_qos_pdev,
				qInx, &tx_cleaned_packets);
		if (ret == EINVAL) {
			DEBUG_PRN("Invaild argument passed\n");
			break;
		} else if (ret < 0) {
			DEBUG_PRN("device transmitted all packets\n");
			break;
		} else {
			transmitted += ret;
			while (tx_cleaned_packets) {
				tx_tmp_packet = tx_cleaned_packets;
				tx_cleaned_packets = tx_cleaned_packets->next;
				tx_tmp_packet->next = tx_free_packets;
				tx_free_packets = tx_tmp_packet;
			}
		}
	}

#if 0
        /* By enabling this piece of code, it will help in transmitting 
         * the packet which are queued before killing the app
         * just kept it for usefulnes in some corner cases. ideally
         * not required.
         */
	while (transmitted <= tx_pkt_queued) {
		tx_cleaned_packets = NULL;
		ret = DWC_ETH_QOS_tx_buffer_cleanup(&dwc_eth_qos_pdev,
				qInx, &tx_cleaned_packets);
		if (ret == EINVAL) {
			DEBUG_PRN("Invaild argument passed\n");
			break;
		} else if (ret < 0) {
			DEBUG_PRN("device transmitted all packets\n");
			break;
		} else {
			transmitted += ret;
		}
	}
#endif

	sleep(1);
	ret = nice(0);
	DWC_ETH_QOS_free_buffer(&dwc_eth_qos_pdev, &a_buffer);

        /* disable AVB queue mode and CBS algorithm */
        DWC_ETH_QOS_program_CBS_alogorithm(&dwc_eth_qos_pdev, qInx, 0);
        
        ret = DWC_ETH_QOS_exit(&dwc_eth_qos_pdev);
        if (ret)
                DEBUG_PRN("%s failed(%s) - is the driver loaded ?\n",
                                __func__, strerror(errno));

	ret = DWC_ETH_QOS_detach(&dwc_eth_qos_pdev);
	gptpdeinit();

	return ret;
}
