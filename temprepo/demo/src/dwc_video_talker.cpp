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
#include <semaphore.h>
/* Opencv libraries */
#include "opencv2/objdetect/objdetect.hpp" 
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
/* extra library */
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/user.h>
#include <netpacket/packet.h>
#include <netinet/in.h>
#include <net/ethernet.h>
#include <sys/un.h>
#include <poll.h>
#include <cctype>
#include <iostream>
#include <iterator>
extern "C" {
#include <pci/pci.h> /* for pci calls */
#include "DWC_ETH_QOS_ycommon.h" /* for local data structure */
#include "avtp.h"
#include "session_mngr.h"
};

#define MAX_FRAME_SIZE 1500
#define	PAYLOAD_LEN 1440
#define	MAX_LINE_NO 319

/* uncomment this to enbale limited frame transmission */
//#define TX_FRAME_LIMIT

#ifdef TX_FRAME_LIMIT
#define MAX_TX_FRAME_CNT 2
#endif

/* uncomment this to enable video disply */
#define DISPLAY_THREAD

/* global variables */
struct device dwc_eth_qos_pdev;
using namespace cv;
unsigned char dest_addr[] = { 0x91, 0xE0, 0xF0, 0x00, 0x0e, 0x80 }; /* IEEE 1722 reserved address */
unsigned char src_addr[] = {0,0,0,0,0,0};
volatile int halt_tx = 0;
unsigned int no_of_frames = 0;
sem_t lock;

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

typedef struct __attribute__ ((packed))
{
        uint64_t delay; 
        uint16_t blanking;
} header_bt_six56;

#define SHM_SIZE 4*8 + sizeof(pthread_mutex_t) /* 3 - 64 bit and 2 - 32 bits */
#define SHM_NAME  "/ptp"
#define false 0
#define true 1
header_bt_six56 *bt_656 = NULL;
static int shm_fd = -1;
static char *memory_offset_buffer = NULL;

CvCapture* capture = 0;
IplImage *framein, *framedisplay, *frametransmit, *frameout;

pthread_mutex_t t_lock;

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
static int get_hw_address(int8_t *interface)
{
        struct ifreq ifr;
        int sockfd;
        int ret = 0;

        sockfd = socket(PF_PACKET, SOCK_RAW, htons(0x800));
        if (sockfd < 0) {
                DEBUG_PRN("failed to open %s interface\n", interface);
                return -1;
        }

        memset(&ifr, 0, sizeof(struct ifreq));
        strncpy(ifr.ifr_name, (char *)interface, sizeof(ifr.ifr_name));
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
        halt_tx = signum;
}

void *captureframe(void *arg)
{

        while(!halt_tx) {
                framein = cvQueryFrame( capture );
                if (framein == NULL) {
                        DEBUG_PRN( "Failed to get the frames\n");
                        exit(2);
                }

                pthread_mutex_lock(&(t_lock)); 
                cvCopy(framein, frametransmit, NULL);
                pthread_mutex_unlock(&(t_lock)); 

                /* sets the Region of Interest  - rectangle area has to be __INSIDE__ the image */
                cvSetImageROI(framein, cvRect(0, 0, 480, 320));

                /* copy subimage */
                cvCopy(framein, framedisplay, NULL);

                /* always reset the Region of Interest */
                cvResetImageROI(framein);
        }

        pthread_exit(NULL);
        return NULL;
}



#ifdef DISPLAY_THREAD
void *Retreieve_show_frame(void *arg)
{
        cvNamedWindow( "Talker_Side", 1 );

        while(!halt_tx) {

                cvShowImage( "Talker_Side", framedisplay);
                waitKey(1);

        }

        waitKey(0);
        cvDestroyWindow( "Talker_Side");
        pthread_exit(NULL);

        return NULL;
}
#endif /*endof DISPLAY_THREAD */

void init_bt_six56_header(header_bt_six56 *hbt656)
{
        hbt656->delay = 0;
        hbt656->blanking = 0;

        return;
}

int main(int argc, char *argv[])
{
        struct DWC_ETH_QOS_user_buff a_buffer;
        struct DWC_ETH_QOS_packet *tx_tmp_packet = NULL;
        struct DWC_ETH_QOS_packet *tx_free_packets = NULL;
        struct DWC_ETH_QOS_packet *tx_cleaned_packets = NULL;
        unsigned int tx_bytes = 0, transmitted = 0, tx_pkt_queued = 0;
        int8_t *interface = NULL;
        unsigned int i, j = 0;
        int qInx = 0, ret = 0;
        uint32_t frame_size;
        uint8_t seqnum;
        uint32_t total_tx_data_buf_size, total_samples = 0;
        unsigned long long now_local;
        void *frame;
        gPtpTimeData td;
        pthread_t tid, tid_capture;

        uint8_t *data_ptr;
        uint32_t imagesize;
        uint32_t read_bytes = 0, line_no;
        unsigned int avb_bw = 75; /* amount of bw to be allocated */

        if (argc < 2) {
                DEBUG_PRN("\nusage : %s <interface_name> \n\n",argv[0]);
                return -1;
        }

        interface = (int8_t *)strdup(argv[1]);

        DEBUG_PRN("\ndevice init done\n");
        frame_size = PAYLOAD_LEN + sizeof(header_61883_pkt) + sizeof(header_bt_six56) +
                sizeof(header_1722_stream_data) + sizeof(eth_header);
        if (frame_size > MAX_FRAME_SIZE) {
                DEBUG_PRN(stderr, "Invalid argument - data_size(%d)\n",
                                PAYLOAD_LEN);
                return -1;
        }

        ret = pci_connect();
        if (ret) {
                DEBUG_PRN("connect failed(%s) - are you running as root ?\n",
                                strerror(errno));
                return ret;
        }

        halt_tx = 0;
        signal(SIGINT, sigint_handler);

        ret = DWC_ETH_QOS_init(&dwc_eth_qos_pdev);
        if (ret) {
                DEBUG_PRN("%s failed(%s) - is the driver loaded ?\n",
                                __func__, strerror(errno));
                return ret;
        }

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

        ret = get_hw_address(interface);
        if (ret < 0) {
                DEBUG_PRN("failed to get %s hw address\n", interface);
                return -1;
        }
        DEBUG_PRN("got hw/mac address(%x:%x:%x:%x:%x:%x)\n",
                        src_addr[0], src_addr[1], src_addr[2], src_addr[3],
                        src_addr[4], src_addr[5]);

        /* allocate CBS to AVB queue */
        ret = DWC_ETH_QOS_program_CBS_alogorithm(&dwc_eth_qos_pdev, qInx, avb_bw);
        if (ret)
                DEBUG_PRN(stderr, "Failed to allocate CBS\n");
                    
        bt_656 = (header_bt_six56 *)calloc(1, sizeof(header_bt_six56));
        if (bt_656 == NULL) {
                DEBUG_PRN(stderr,"fails to allocate memory for BT-656 structure\n");
                exit(2);
        }

        /* divide the huge data buffer into small data buffer
         * for each packets and create packet list.
         * */
        init_bt_six56_header(bt_656);
        frame = create_avtp_stream_pkt(E_61883_IIDC_SUBTYPE, (PAYLOAD_LEN + sizeof(header_bt_six56)));
        prepare_eth_header(frame, dest_addr, (uint8_t *)interface, 3, 2);
        set_61883_stream_header_vals(frame,SY,SRC_ID,DBS,FN,QPC,DBC,FMT,FDF,SYT);
        add_stream_data_streamid(frame,src_addr,sizeof(src_addr));

        DEBUG_PRN("No of buffer allocated = %d\n", (a_buffer.mmap_size)/frame_size);
        for (i = 0; i < (a_buffer.mmap_size)/frame_size; i++) {
                tx_tmp_packet = (DWC_ETH_QOS_packet *)calloc(1, sizeof(struct DWC_ETH_QOS_packet));
                if (tx_tmp_packet == NULL) {
                        DEBUG_PRN("failed to allocate wrapper packet\n");
                        return errno;
                }

                tx_tmp_packet->map.paddr = a_buffer.dma_addr;
                tx_tmp_packet->map.mmap_size = a_buffer.mmap_size;

                tx_tmp_packet->offset = (i * frame_size);
                tx_tmp_packet->vaddr =(void *)((int)a_buffer.addr + tx_tmp_packet->offset);

                tx_tmp_packet->next = tx_free_packets;

                memset(tx_tmp_packet->vaddr, 0, frame_size);
                memcpy(((char *)tx_tmp_packet->vaddr), frame, frame_size);

                tx_tmp_packet->len = frame_size;
                tx_free_packets = tx_tmp_packet;
        }
#ifdef DEBUG
        DEBUG_PRN("%d packets are ready to transmit\n", i);
#endif

        if (gptpinit() == false) {
                DEBUG_PRN(stderr,"start the gptp daemon\n");
                return -1;
        }
        ret = nice(-20);

#ifdef DISPLAY_THREAD
        ret = pthread_create(&tid, NULL, Retreieve_show_frame, NULL);
        if(ret) {
                DEBUG_PRN(stderr,"pthread_create() fail :[%s]",strerror(ret));
                return -1;
        }
#endif /* end of DISPLAY_THREAD */

        seqnum = 0;
        capture = cvCaptureFromCAM( 0 );
        if (!capture) {
                DEBUG_PRN("capturing from CAM didn't work\n");
                exit(3);
        }

        system("clear");
        pthread_mutex_init(&(t_lock), NULL);

        /* make the image buffers ready */
        framein = cvQueryFrame( capture );
        if (framein == NULL) {
                DEBUG_PRN( "Failed to get the frames\n");
                exit(2);
        }

        frametransmit = cvCreateImage(cvGetSize(framein),  framein->depth, framein->nChannels);
        frameout = cvCreateImage(cvGetSize(framein),  framein->depth, framein->nChannels);
        cvCopy(framein, frametransmit, NULL);

        /* sets the Region of Interest  - rectangle area has to be __INSIDE__ the image */
        cvSetImageROI(framein, cvRect(0, 0, 480, 320));
        framedisplay = cvCreateImage(cvGetSize(framein),  framein->depth, framein->nChannels);
        cvCopy(framein, framedisplay, NULL);
        cvResetImageROI(framein);
        
        ret = pthread_create(&tid_capture, NULL, captureframe, NULL);
        if(ret) {
                DEBUG_PRN(stderr,"pthread_create() fail :[%s]",strerror(ret));
                return -1;
        }
        
        while (!halt_tx) {
#ifdef TX_FRAME_LIMIT
                if (no_of_frames == MAX_TX_FRAME_CNT)
                        break;
#endif
                pthread_mutex_lock(&(t_lock)); 
                cvCopy(frametransmit, frameout, NULL);
                pthread_mutex_unlock(&(t_lock));

                line_no = 0;
                read_bytes = 0;
                imagesize = 0;
                no_of_frames++;

                while (imagesize <= MAX_LINE_NO) {
                        tx_tmp_packet = tx_free_packets;
                        if (tx_tmp_packet == NULL)
                                goto cleanup;

                        frame = ((char *)tx_tmp_packet->vaddr);
                        tx_free_packets = tx_tmp_packet->next;

                        add_stream_data_seq_num(frame, seqnum++);
                        if (seqnum % 4 == 0)
                                set_stream_data_tv_field(frame, 0);
                        else
                                set_stream_data_tv_field(frame, 1);

                        set_61883stream_data_boundary_continuity(frame, total_samples);
                        total_samples += PAYLOAD_LEN;
                        bt_656->blanking = line_no;

                        data_ptr = (uint8_t *)get_61883_stream_payload_ptr(frame);

                        memcpy(&data_ptr[sizeof(header_bt_six56)],
                                        &frameout->imageData[read_bytes],
                                        PAYLOAD_LEN);

                        gptpscaling(&td);
                        now_local = get_wallclock();
                        bt_656->delay = now_local - td.ml_phoffset;

                        memcpy(&data_ptr[0] , bt_656, sizeof(header_bt_six56));
                        add_stream_data_timestamp(frame, (now_local-td.ml_phoffset));
#ifdef TS_CAPTURE
                        tx_tmp_packet->pkt_no1 = line_no;
#endif
                        ret = DWC_ETH_QOS_start_xmit(&dwc_eth_qos_pdev, qInx,
                                        tx_tmp_packet);
                        if (!ret) {
                                tx_pkt_queued++;
                                tx_bytes += tx_tmp_packet->len;
                                DEBUG_PRN("pkt_no= %d line= %d talker_ts= %lld now_local_tx= %lld offset= %lld\n",
                                        j, line_no, bt_656->delay, now_local, td.ml_phoffset);
                                j++;
                                line_no++;
                                imagesize++;
                                read_bytes = read_bytes + PAYLOAD_LEN +
                                        480 /* cropping images to make 480 * 320 size */;
                                usleep(30);
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
                        while (tx_cleaned_packets) {
#ifdef TS_CAPTURE
                                DEBUG_PRN( "[%03lld: %03lld]: submit_time = %lld transmitted_time = %lld dev_xmit_time = %lld\n",
                                                tx_cleaned_packets->pkt_no1, tx_cleaned_packets->pkt_no2,
                                                tx_cleaned_packets->submit_time, tx_cleaned_packets->transmitted_time,
                                                tx_cleaned_packets->transmitted_time - tx_cleaned_packets->submit_time);
#endif
                                transmitted += ret;
                                tx_tmp_packet = tx_cleaned_packets;
                                tx_cleaned_packets = tx_cleaned_packets->next;
                                tx_tmp_packet->next = tx_free_packets;
                                tx_free_packets = tx_tmp_packet;
                        }
                }
        }

        /* wait untill all the queued packets are transmitted
         * by device
         * */
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

        ret = nice(0);
        DEBUG_PRN("Queue[%d] => total pkts queued: %d total bytes queued: %d\n",
                        qInx, tx_pkt_queued, tx_bytes);
        DEBUG_PRN("\n");
        cvReleaseCapture( &capture );
        sleep(1);
        DEBUG_PRN("NUMBER OF FRAMES TRANSMITTED = %d\n",no_of_frames);

        free(bt_656);
        DWC_ETH_QOS_free_buffer(&dwc_eth_qos_pdev, &a_buffer);

        /* disable AVB queue mode and CBS algorithm */
        DWC_ETH_QOS_program_CBS_alogorithm(&dwc_eth_qos_pdev, qInx, 0);

        ret = DWC_ETH_QOS_exit(&dwc_eth_qos_pdev);
        if (ret)
                DEBUG_PRN("%s failed(%s) - is the driver loaded ?\n",
                                __func__, strerror(errno));

        ret = DWC_ETH_QOS_detach(&dwc_eth_qos_pdev);
        if (ret)
                DEBUG_PRN("%s failed(%s) - is the driver loaded ?\n",
                                __func__, strerror(errno));

        gptpdeinit();

#ifdef DISPLAY_THREAD
        //pthread_join(tid, NULL);
#endif /* end of DISPLAY_THREAD */

        DEBUG_PRN("Done !!!!!!!!!!\n");

        return ret;
}
