/* Send Multicast Datagram code example. */

#include <unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <errno.h>
#include <ncurses.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <netinet/in.h>

#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <sys/select.h>
#include <linux/net_tstamp.h>
#include <linux/sockios.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#define MAX_FRAMESIZE 1400
#define MAX_DELAYCOUNT 10000
#define MIN_FRAMESIZE 150
#define COUNT 50
unsigned int framesize = 700; 
unsigned int delaycount; 
int percent = 0;
int quit = 0;



void showticker(int percent1, int percent2 )
{
        int i,j;
        fprintf(stderr, "|");
        if(delaycount > 0)
        {
                percent2 = percent2/10;
                percent2 = 10 - percent2;
                for (i = 0; i < (percent2 * 2); i++)
                        fprintf(stderr,"=");
                for (j = 0; j < ((10-percent2)*2); j++)
                        fprintf(stderr,".");
        }
        else{
                for (i = 0; i < 20; i++)
                        fprintf(stderr,"=");
        }

        percent1 = (percent1 / 10);
        fprintf(stderr, "|");
        for (i = 0; i < (percent1 * 2); i++)
                fprintf(stderr,"=");
        for (j = 0; j < ((10-percent1)*2); j++)
                fprintf(stderr,".");
        fprintf(stderr, "|");
        if (delaycount > 0)
                fprintf(stderr, "%4d percent\r", percent2);
        else
                fprintf(stderr, "%4d percent\r", percent1*10);

        usleep(200);
}

void *modify_bandwidth(void *arg)
{
        int ch;
        int percent1, percent2;

        system("clear");

        initscr();
        raw();
        keypad(stdscr, TRUE);
        noecho();

        printw("Non AVB bandwidth: UP-increase DOWN-decrease q-quit\n");

        while((ch = getch()) != 'q') {
                switch(ch) {
                        case KEY_UP:
                                if (delaycount > 0) {
                                        delaycount -= 1000;
                                        break;
                                }

                                framesize = framesize + COUNT;
                                if (framesize > MAX_FRAMESIZE) 
                                        framesize = MAX_FRAMESIZE;
                                break;
                        case KEY_DOWN:
                                framesize = framesize - COUNT; 
                                if(framesize <= MIN_FRAMESIZE) { 
                                        framesize = MIN_FRAMESIZE;
                                        delaycount += 1000;
                                        if (delaycount > MAX_DELAYCOUNT)
                                                delaycount = MAX_DELAYCOUNT;
                                }
                                break;
                        default:    
                                break;
                }
                
                percent1 = 100 * framesize / MAX_FRAMESIZE;
                percent2 = 100 * delaycount/ MAX_DELAYCOUNT;
                showticker(percent1, percent2);
                
        }
        endwin();
        quit = 1;

}

int main (int argc, char *argv[ ])
{
        struct ifreq device;
        int err;
        unsigned int local_delaycount;
        struct sockaddr_ll ifsock_addr;
        struct packet_mreq mreq;
        int ifindex;
        int sd_event;
        unsigned char payload[MAX_FRAMESIZE];
        int length;
        pthread_t tid;
        unsigned char DEST_ADDR[] = { 0x00, 0x55, 0x7b, 0xb5, 0x7d, 0xf7 };
        char *ifname = "eth1";

        if (argc < 2) {
                printf("Usage : %s <interface_name>\n",argv[0]);
                return -1;
        }
        ifname = strdup(argv[1]);

        sd_event = socket(AF_PACKET, SOCK_RAW, htons(0x1234));
        if (sd_event == -1) {
                printf("failed to open event socket: %s \n", strerror(errno));
                return -1;
        }

        memset(&device, 0, sizeof(device));
        memcpy(device.ifr_name, ifname, IFNAMSIZ);
        err = ioctl(sd_event, SIOCGIFHWADDR, &device);
        if (err == -1) {
                printf("Failed to get interface address for %s : %s", ifname, strerror(errno));
                return -1;
        }

        err = ioctl(sd_event, SIOCGIFINDEX, &device);
        if (err == -1) {
                printf("Failed to get interface index: %s", strerror(errno));
                return -1;
        }

        ifindex = device.ifr_ifindex;
        memset(&mreq, 0, sizeof(mreq));
        mreq.mr_ifindex = ifindex;
        mreq.mr_type = PACKET_MR_MULTICAST;
        mreq.mr_alen = 6;
        memcpy(mreq.mr_address, DEST_ADDR, mreq.mr_alen);
        err = setsockopt(sd_event, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
        if (err == -1) {
                printf ("Unable to add PTP multicast addresses to port id: %u", ifindex);
                return -1;
        }

        memset(&ifsock_addr, 0, sizeof(ifsock_addr));
        ifsock_addr.sll_family = AF_PACKET;
        ifsock_addr.sll_ifindex = ifindex;
        ifsock_addr.sll_protocol = htons(0x1234);
        err = bind(sd_event, (struct sockaddr *) & ifsock_addr, sizeof(ifsock_addr));
        if (err == -1) {
                printf("Call to bind() failed: %s", strerror(errno));
                return -1;
        }

        memset(payload, 0, sizeof(payload));
        memcpy(payload, DEST_ADDR, sizeof(DEST_ADDR));
        memcpy(&(payload[6]), device.ifr_hwaddr.sa_data, 6);
        payload[12] = 0x12;
        payload[13] = 0x34;

        err = pthread_create(&tid, NULL, modify_bandwidth, NULL);
        if (err) {
                fprintf(stderr,"pthread_create() fail :[%s]", strerror(err));
                return -1;
        }


        while (!quit) {
                local_delaycount = delaycount;
                length = framesize;
                err = sendto(sd_event, payload, length, 0, (struct sockaddr *) &ifsock_addr, sizeof(ifsock_addr));
                if (err < 0)
                        printf("send fail : %s\n", strerror(errno));

                while(local_delaycount--);
        }
        

        close(sd_event);
        pthread_exit(NULL);

        return 0;
}
