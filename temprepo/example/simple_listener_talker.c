#include <stdio.h> /* for printf */
#include <string.h> /* for string operation */
#include <stdlib.h> /* for exit */
#include <signal.h> /* for signal */
#include <errno.h> /* for errno, stderr */
#include <syslog.h>
#include <unistd.h> /* for getopt*/
#include <arpa/inet.h> /* for htons */

#include <sys/ioctl.h> /* for ioctl */
#include <sys/socket.h> /* for socket */
#include <linux/if.h> /* for ioctl number */
#include <pci/pci.h> /* for pci calls */

#include "DWC_ETH_QOS_ycommon.h" /* for local data structure */

#define TX_DATA_BUF_SIZE_PER_DESC 1518
#define TOTAL_TX_DATA_BUF_SIZE (TX_DATA_BUF_SIZE_PER_DESC * TX_DESC_CNT)

/* global variables */
struct device dwc_eth_qos_pdev;
//unsigned char dest_addr[] = {0xb8,0x70,0xf4,0xdb,0x01,0xa3};
unsigned char dest_addr[] = {0x00,0x16,0xd3,0x2e,0x29,0xb3};
unsigned char src_addr[] = {0,0,0,0,0,0};
unsigned char eth_type[2] = {0x80, 0};
unsigned int budget = 15;
volatile int halt = 0;

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
		printf("Attaching to %s\n", dev_path);
		ret = DWC_ETH_QOS_attach(dev_path, &dwc_eth_qos_pdev);
		if (ret)
			continue;

		printf("Successfully attached\n");
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
		printf("failed to open %s interface\n", ifname);
		return -1;
	}

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	ret = ioctl(sockfd, SIOCGIFHWADDR, &ifr);
	if (ret < 0) {
		close(sockfd);
		printf("ioctl to get hw address failed\n");
		return -1;
	}

	memcpy(src_addr, ifr.ifr_hwaddr.sa_data, sizeof(src_addr));

	close(sockfd);

	return ret;
}


void prepare_pkts(char *src, char *dst, int len)
{
	/* reset data buffer pointer */
	memset(src, 0, TX_DATA_BUF_SIZE_PER_DESC);

	/* exchange both src and dst address while transmitting */
	memcpy(src, &dst[6], 6);
	memcpy(&src[6], dst, 6);

	/* copy remaining payload */
	memcpy(&src[12], &dst[12], (len - 12));
}


void sigint_handler(int signum)
{
	printf("got SIGINT, signum = %d\n", signum);
	halt = signum;
}


static void usage(void)
{
	fprintf(stderr, "\n"
		"usage: simple_talker [-h] -i interface-name\n"
		"options:\n"
		"       -h show help\n"
		"       -i specify interface name for AVB connection\n");
	exit(1);
}


int main(int argc, char *argv[])
{
	struct DWC_ETH_QOS_user_buff a_buffer;
	struct DWC_ETH_QOS_packet *rx_tmp_packet = NULL;
	struct DWC_ETH_QOS_packet *tx_free_packets = NULL;
	struct DWC_ETH_QOS_packet	*tx_tmp_packet = NULL;
	struct DWC_ETH_QOS_packet	*tx_cleaned_packets = NULL;
	unsigned int tx_bytes = 0, transmitted = 0, tx_pkt_queued = 0;
	unsigned int rx_bytes = 0, received = 0;
	char *ifname = NULL;
	unsigned int i;
	int qInx = 0;
	int ret = 0;

	for (;;) {
		ret = getopt(argc, argv, "hi:");
		if (ret < 0)
			break;

		switch (ret) {
		case 'h':
			usage();
			break;
		case 'i':
			if (ifname) {
				printf("only one interface per daemon\
					is supported\n");
				usage();
			}
			ifname = strdup(optarg);
			break;
		}
	}
	if ((optind < argc) || (ifname == NULL))
		usage();

	ret = pci_connect();
	if (ret) {
		printf("connect failed(%s) - are you running as root ?\n",
			strerror(errno));
		return ret;
	}

	halt = 0;
	signal(SIGINT, sigint_handler);

	ret = DWC_ETH_QOS_test_reg_read(&dwc_eth_qos_pdev);
	if (ret) {
		printf("device register read failed\n");
		return ret;
	}
	printf("read device register test passed successfully\n");

	ret = DWC_ETH_QOS_test_reg_write(&dwc_eth_qos_pdev);
	if (ret) {
		printf("device register write failed\n");
		return ret;
	}
	printf("write device register passed successfully\n");

	ret = DWC_ETH_QOS_init(&dwc_eth_qos_pdev);
	if (ret) {
		printf("init failed(%s) - is the driver loaded ?\n",
			strerror(errno));
		return ret;
	}
	printf("device init done\n");

	ret = get_hw_address(ifname);
	if (ret < 0) {
		printf("failed to get %s hw address\n", ifname);
		usage();
	}
	printf("got hw/mac address(%x:%x:%x:%x:%x:%x)\n",
		src_addr[0], src_addr[1], src_addr[2], src_addr[3],
		src_addr[4], src_addr[5]);

	a_buffer.alloc_size = TOTAL_TX_DATA_BUF_SIZE;
	ret = DWC_ETH_QOS_get_buffer(&dwc_eth_qos_pdev, &a_buffer);
	if (ret) {
		printf("tx data buffer allocation failed(%s)\n",
			strerror(errno));
		return ret;
	}
	printf("got tx data buffers(%d bytes)\n", a_buffer.mmap_size);

	/* divide the huge tx data buffer into small data buffer
	 * for each packets and create packet list.
	 * */
	for (i = 0; i < (a_buffer.mmap_size)/TX_DATA_BUF_SIZE_PER_DESC; i++) {
		tx_tmp_packet = calloc(1, sizeof(struct DWC_ETH_QOS_packet));
		if (tx_tmp_packet == NULL) {
			printf("failed to allocate wrapper packet\n");
			return errno;
		}

		tx_tmp_packet->map.paddr = a_buffer.dma_addr;
		tx_tmp_packet->map.mmap_size = a_buffer.mmap_size;

		tx_tmp_packet->offset = (i * TX_DATA_BUF_SIZE_PER_DESC);
		tx_tmp_packet->vaddr = a_buffer.addr + tx_tmp_packet->offset;
		tx_tmp_packet->next = tx_free_packets;

		tx_free_packets = tx_tmp_packet;
	}

	printf("start Tx and Rx !!!!!!!!!\n");

	while (!halt) {
		rx_tmp_packet  =  DWC_ETH_QOS_read(&dwc_eth_qos_pdev, qInx, 100);

		while (rx_tmp_packet) {
			/* transmits received data */
			tx_tmp_packet = tx_free_packets;
			if (tx_tmp_packet == NULL)
				goto cleanup;

			tx_free_packets = tx_tmp_packet->next;
			prepare_pkts(tx_tmp_packet->vaddr, rx_tmp_packet->vaddr,
						rx_tmp_packet->len);
			tx_tmp_packet->len = rx_tmp_packet->len;

			ret = DWC_ETH_QOS_start_xmit(&dwc_eth_qos_pdev, qInx,
					tx_tmp_packet);
			if (!ret) {
				tx_pkt_queued++;
				tx_bytes += tx_tmp_packet->len;
				goto next_pkt;
			} else if (ret == ENOSPC) {
				/* add it back to list now */
				tx_tmp_packet->next = tx_free_packets;
				tx_free_packets = tx_tmp_packet;
				goto cleanup;
			}

next_pkt:
			received++;
			rx_bytes += rx_tmp_packet->len;
			rx_tmp_packet = rx_tmp_packet->next;
			continue;

cleanup:
			ret += DWC_ETH_QOS_tx_buffer_cleanup(&dwc_eth_qos_pdev,
				qInx, &tx_cleaned_packets);
			if (ret == EINVAL) {
				printf("Invalid arguments passed\n");
				break;
			} else if (ret < 0) {
				printf("device transmitted all packets\n");
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

		DWC_ETH_QOS_read_done(&dwc_eth_qos_pdev, qInx);
	}

	/* wait untill all the queued packets are transmitted
	 * by device
	 * */
	while (transmitted <= tx_pkt_queued) {
		tx_cleaned_packets = NULL;
		ret = DWC_ETH_QOS_tx_buffer_cleanup(&dwc_eth_qos_pdev,
						qInx, &tx_cleaned_packets);
		if (ret == EINVAL) {
			printf("Invalid arguments passed\n");
			break;
		} else if (ret < 0) {
			printf("device transmitted all packets\n");
			break;
		} else {
			transmitted += ret;
		}
	}

	printf("\n");
	printf("Queue[%d] TX => total pkts queued   : %d total bytes queued    : %d\n"
			   "          RX => total pkts received : %d, total bytes received : %d\n",
				 qInx, tx_pkt_queued, tx_bytes, received, rx_bytes);
	printf("\n");

	
	DWC_ETH_QOS_free_buffer(&dwc_eth_qos_pdev, &a_buffer);

	ret = DWC_ETH_QOS_detach(&dwc_eth_qos_pdev);

	printf("Done !!!!!!!!!!\n");

	return ret;
}
