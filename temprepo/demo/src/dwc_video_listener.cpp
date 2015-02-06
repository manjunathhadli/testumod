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
#include <sys/mman.h> /* gptp */
#include <sys/stat.h> /* gptp: For mode constants */
#include <fcntl.h> /* gptp */
#include <pthread.h>
#include <cctype>
#include <iostream>
#include <iterator>
#include <pcap/pcap.h>
#include <semaphore.h>
/* Opencv libraries */
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
extern "C" {
#include <pci/pci.h> /* for pci calls */
#include "DWC_ETH_QOS_ycommon.h" /* for local data structure */
#include "avtp.h"
#include "session_mngr.h"
}

#define IMAGE_WIDTH 480
#define IMAGE_HEIGHT 320
#define BYTES_PER_PIXEL 3
#define FRAME_SIZE (IMAGE_WIDTH * IMAGE_HEIGHT * BYTES_PER_PIXEL)

#define MAX_FRAME_SIZE 1500
#define	MAX_LINE_NO 319
#define PAYLOAD_LEN 1440

/* Length of local circular buffer size */
#define CIR_BUF_RING_LEN 6

/* uncomment this to print in multiline */
//#define MULTILINE_PRINT 

/* uncomment this to use pcap filter on AVB muticast MAC add */
//#define PCAP_FILTER 

/* uncomment this to open pcap in promiscuous mode */
#define PCAP_PROM_MODE

/* uncomment this to enable video disply */
#define DISPLAY_THREAD


/* global variables */
struct device dwc_eth_qos_pdev;
volatile int halt = 0;
uint32_t imagepoint = 0;
pcap_t* handle;
unsigned long long now_local;
uint8_t z, k; 
char *dataptr[CIR_BUF_RING_LEN];
sem_t lock;
unsigned int no_of_frames;
uint64_t p2p_delay;
unsigned long long total_avb_bytes;
unsigned long long total_non_avb_bytes;

typedef struct { 
        int64_t ml_phoffset;
        int64_t ls_phoffset;
        int32_t ml_freqoffset;
        int32_t ls_freqoffset;
        int64_t local_time;
} gPtpTimeData;

typedef struct __attribute__ ((packed))
{
        uint64_t delay;
        uint16_t blanking;
} header_bt_six56;

#define SHM_SIZE 4*8 + sizeof(pthread_mutex_t) /* 3 - 64 bit and 2 - 32 bits */
#define SHM_NAME  "/ptp"
#define false 0
#define true 1

static int shm_fd = -1;
static char *memory_offset_buffer = NULL;
header_bt_six56 *bt_656 = NULL;
gPtpTimeData td;

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

void sigint_handler(int signum)
{
        DEBUG_PRN("\ngot SIGINT, signum = %d\n", signum);
        fprintf(stdout, "\n\n");
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

        return true;
}

void init_bt_six56_header(header_bt_six56 *hbt656)
{
        hbt656->delay = 0;
        hbt656->blanking = 0;

        return;
}

#ifdef DISPLAY_THREAD
void *Retrieve_show_frame(void *arg)
{
        IplImage* frame = NULL;

        cvNamedWindow( "Listener Side", 1 );
        frame = (IplImage *)calloc(1, sizeof(IplImage));
        
        system("clear");
        fprintf(stdout,"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
        while(!halt) {
                sem_wait(&lock);

                if (dataptr[k] == NULL) {
                        DEBUG_PRN("NULL dataptr is received\n");
                        continue;
                }

                frame->width = IMAGE_WIDTH;
                frame->height = IMAGE_HEIGHT;
                frame->imageData = (char *)dataptr[k];
                frame->imageDataOrigin = (char *)dataptr[k];
                frame->nSize = 112;
                frame->ID = 0;
                frame->nChannels = 3;
                frame->depth = IPL_DEPTH_8U;
                frame->dataOrder = 0;
                frame->origin = 0;
                frame->roi = NULL;
                frame->maskROI = NULL;
                frame->imageSize = FRAME_SIZE;
                frame->widthStep = IMAGE_WIDTH * BYTES_PER_PIXEL;

                cvShowImage("Listener Side", frame);
                waitKey(1);
        }
        waitKey(0);
        DEBUG_PRN("hlat= %d\n", halt);
        cvDestroyWindow("Listener Side");
        free(frame);
        pthread_exit(NULL);

        return NULL;
}
#endif /* end of DISPLAY_THREAD */

void timer_handler(int) 
{
        unsigned int total_avb_bits = 0;
        unsigned int total_non_avb_bits = 0;

        total_avb_bits = total_avb_bytes * 8;
        total_non_avb_bits = total_non_avb_bytes * 8;

#ifdef  MULTILINE_PRINT
        fprintf(stdout, "AVB BW:%6d Kbps(%3d Mbps)  NON_AVB BW: %6d Kbps(%3d Mbps)  PeerDelay: %6lld us\n",
                        total_avb_bits/1024, total_avb_bits/(1024*1024),
                        total_non_avb_bits/1024, total_non_avb_bits/(1024*1024),
                        p2p_delay/1000);
#else
        fprintf(stdout, "AVB BW:%6d Kbps(%3d Mbps)  NON_AVB BW: %6d Kbps(%3d Mbps)  PeerDelay: %6lld us\r",
                        total_avb_bits/1024, total_avb_bits/(1024*1024),
                        total_non_avb_bits/1024, total_non_avb_bits/(1024*1024),
                        p2p_delay/1000);
        fflush(stdout);
#endif 

        total_avb_bytes = 0;
        total_non_avb_bytes = 0;
}

void pcap_callback(u_char* args, const struct pcap_pkthdr* packet_header, const u_char* packet)
{
        static uint32_t j;
        void *frame = (void *)packet;
        uint64_t l_current_time = 0;
        uint8_t *rcv_data = NULL;
        uint32_t datalen = 0;
#ifndef PCAP_FILTER
        eth_header* header = (eth_header*)(packet);

        if ((header->type[0] == 0x22) && (header->type[1] == 0xf0)) {
#endif /* end of PCAP_FILTER */
                total_avb_bytes += packet_header->len;
                gptpscaling(&td);

                now_local = get_wallclock();
                l_current_time = now_local - td.ml_phoffset;

                rcv_data = get_61883_stream_payload_ptr(frame);
                if(rcv_data == NULL)
                        DEBUG_PRN("rcv_data is NULL");

                memcpy(bt_656 , &rcv_data[0], sizeof(header_bt_six56));
                p2p_delay = (l_current_time - bt_656->delay);


#ifdef DISPLAY_THREAD
                datalen = get_61883_stream_payload_len(frame);

                if (bt_656->blanking == MAX_LINE_NO) {
                        memcpy(&dataptr[z][imagepoint], &rcv_data[sizeof(header_bt_six56)],
                                        (datalen - sizeof(header_bt_six56)));

                        imagepoint = imagepoint + (datalen - sizeof(header_bt_six56));
                        imagepoint = 0;
                        k = z;
                        z++;
                        if (z == CIR_BUF_RING_LEN)
                                z = 0;

                        /* release lock for display function */
                        sem_post(&lock);
                        no_of_frames++;
                } else {
                        memcpy(&dataptr[z][imagepoint],&rcv_data[sizeof(header_bt_six56)],
                                        (datalen - sizeof(header_bt_six56)));
                        imagepoint = imagepoint + (datalen - sizeof(header_bt_six56));
                }
#else
                if (bt_656->blanking == MAX_LINE_NO) {
                        no_of_frames++;
                }
#endif /* end of DISPLAY_THREAD */
                j++;
#ifndef PCAP_FILTER
        }
#endif /* end of PCAP_FILTER */

#ifdef PCAP_PROM_MODE
        else {
                total_non_avb_bytes += packet_header->len;
        }
#endif /* PCAP_PROM_MODE */
}

int main(int argc, char *argv[])
{
        char *ifname = NULL;
        int ret = 0, i;
        struct itimerval timer;

#ifdef DISPLAY_THREAD  
        pthread_t tid;
#endif /* end of DISPLAY_THREAD */

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

        now_local = get_wallclock();
        ret = nice(-20);

        bt_656 = (header_bt_six56 *)calloc(1, sizeof(header_bt_six56));
        if (bt_656 == NULL) {
                DEBUG_PRN("fails to allocate memory for BT-656 structure\n");
                exit(2);
        }

#ifdef PCAP_PROM_MODE
        handle = pcap_open_live(ifname, BUFSIZ, 1, -1, errbuf);
#else
        handle = pcap_open_live(ifname, BUFSIZ, 0, -1, errbuf);
#endif /* PCAP_PROM_MODE */
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

        for(i = 0; i < CIR_BUF_RING_LEN; i++) {
                dataptr[i] = (char *)calloc(FRAME_SIZE, sizeof(char));
                if (dataptr[i] == NULL)
                        DEBUG_PRN( "failled to allocate dataptr[%d]\n", i);
        }

        init_bt_six56_header(bt_656);

        /* create a 1-sec timer for calculating AVB and NON-AVB BW  */
        timer.it_interval.tv_sec = 1;
        timer.it_interval.tv_usec = 0;
        /* set timer for "INTERVAL (10) seconds */
        timer.it_value.tv_sec = 1;
        timer.it_value.tv_usec = 0;
        setitimer(ITIMER_REAL, &timer, 0);
        /* set the Alarm signal capture */
        signal(SIGALRM, timer_handler);
        system("clear"); //clears the screen

#ifdef DISPLAY_THREAD  
        ret = pthread_create(&tid, NULL, Retrieve_show_frame, NULL);
        if (ret) {
                DEBUG_PRN("pthread_create() fail :[%s]", strerror(ret));
                return -1;
        }
#endif /* end of DISPLAY_THREAD */

        
        /* loop forever and call callback-function for every received packet */
        pcap_loop(handle, -1, pcap_callback, NULL);

        DEBUG_PRN("NUMBER OF FRAMES RECEIVED = %d\n", no_of_frames);
        ret = nice(0);
        free(bt_656);

        for(i = 0; i < CIR_BUF_RING_LEN; i++)
                free(dataptr[i]);

        ret = DWC_ETH_QOS_detach(&dwc_eth_qos_pdev);
        if (ret)
                DEBUG_PRN("%s failed(%s) - is the driver loaded ?\n",
                                __func__, strerror(errno));

        gptpdeinit();

        return 0;
}
