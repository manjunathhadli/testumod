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


#define TX_DATA_BUF_SIZE_PER_DESC 1500
#define TOTAL_TX_DATA_BUF_SIZE (TX_DATA_BUF_SIZE_PER_DESC * TX_DESC_CNT)

#define ETH_HDR_LEN 14 /* without VLAN */

/* global variables */
struct device dwc_eth_qos_pdev;
unsigned char dest_addr[] = {0x00,0x55,0x7b,0xb5,0x7d,0xf9}; //TODO: assign one destination hw/mac address
//unsigned char dest_addr[] = {0x00,0x16,0xd3,0x2e,0x29,0xb3}; //TODO: assign one destination hw/mac address
unsigned char src_addr[] = {0,0,0,0,0,0};
unsigned char eth_type[2] = {0x80, 0};
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


void prepare_pkts(void *pkt_data, int data)
{
	int payload;

	/* reset data buffer pointer */
	memset(pkt_data, 0, TX_DATA_BUF_SIZE_PER_DESC);

	/* add MAC header -
	 * [dst_addr(6bytes) + src_addr(6bytes) + type(2bytes)]
	 * */
	memcpy(pkt_data, dest_addr, sizeof(dest_addr));
	memcpy(pkt_data + 6, src_addr, sizeof(src_addr));
	memcpy(pkt_data + 12, eth_type, sizeof(eth_type));

	/* add payload */
	for (payload = 0; payload < (TX_DATA_BUF_SIZE_PER_DESC - ETH_HDR_LEN); payload++) {
		((char *)pkt_data)[ETH_HDR_LEN + payload] = data;
	}
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
	struct DWC_ETH_QOS_packet *tx_tmp_packet = NULL;
	struct DWC_ETH_QOS_packet *tx_free_packets = NULL;
	struct DWC_ETH_QOS_packet *tx_cleaned_packets = NULL;
	unsigned int tx_bytes = 0, transmitted = 0, tx_pkt_queued = 0;
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

	a_buffer.alloc_size = TOTAL_TX_DATA_BUF_SIZE;
	ret = DWC_ETH_QOS_get_buffer(&dwc_eth_qos_pdev, &a_buffer);
	if (ret) {
		printf("tx data buffer allocation failed(%s)\n",
			strerror(errno));
		return ret;
	}
	printf("got tx data buffers(%d bytes)\n", a_buffer.mmap_size);

	ret = get_hw_address(ifname);
	if (ret < 0) {
		printf("failed to get %s hw address\n", ifname);
		usage();
	}
	printf("got hw/mac address(%x:%x:%x:%x:%x:%x)\n",
		src_addr[0], src_addr[1], src_addr[2], src_addr[3],
		src_addr[4], src_addr[5]);

	/* divide the huge data buffer into small data buffer
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

		prepare_pkts(tx_tmp_packet->vaddr, i);

		tx_tmp_packet->len = TX_DATA_BUF_SIZE_PER_DESC;

		tx_free_packets = tx_tmp_packet;
	}
	printf("%d packets are ready to transmit\n", i);

	/* transmits packets continuously untill tx is stopped */
	while(!halt) {
		tx_tmp_packet = tx_free_packets;
		if (tx_tmp_packet == NULL)
			goto cleanup;

		tx_free_packets = tx_tmp_packet->next;

		ret = DWC_ETH_QOS_start_xmit(&dwc_eth_qos_pdev, qInx,
				tx_tmp_packet);
		if (!ret) {
			tx_pkt_queued++;
			tx_bytes += tx_tmp_packet->len;
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
			printf("Invaild argument passed\n");
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

	/* wait untill all the queued packets are transmitted
	 * by device
	 * */
	while (transmitted <= tx_pkt_queued) {
		tx_cleaned_packets = NULL;
		ret = DWC_ETH_QOS_tx_buffer_cleanup(&dwc_eth_qos_pdev,
				qInx, &tx_cleaned_packets);
		if (ret == EINVAL) {
			printf("Invaild argument passed\n");
			break;
		} else if (ret < 0) {
			printf("device transmitted all packets\n");
			break;
		} else {
			transmitted += ret;
		}
	}

	printf("\n");
	printf("Queue[%d] => total pkts queued: %d total bytes queued: %d\n",
		qInx, tx_pkt_queued, tx_bytes);
	printf("\n");

	DWC_ETH_QOS_free_buffer(&dwc_eth_qos_pdev, &a_buffer);

	ret = DWC_ETH_QOS_detach(&dwc_eth_qos_pdev);

	printf("Done !!!!!!!!!!\n");

	return ret;
}
