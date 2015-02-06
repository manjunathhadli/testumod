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


#define TX_DATA_BUF_SIZE 4096
#define PKT_SIZE 100
#define ETH_HDR_LEN 14 /* without VLAN */

/* global variables */
struct device dwc_eth_qos_pdev;
unsigned char src_addr[] = {0,0,0,0,0,0};
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
	struct DWC_ETH_QOS_packet *tmp_packet = NULL;
	unsigned int /*packets = 0, bytes = 0,*/ received = 0;
	char *ifname = NULL;
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

	//int i = 0;
	while (!halt) {
		tmp_packet  =  DWC_ETH_QOS_read(&dwc_eth_qos_pdev, qInx, 100);
		while (tmp_packet) {
      printf("received pkt\n");
			//for (i=0; i < 18; i++) {
			//	fprintf(stderr,"%#x ",((char *)tmp_packet->vaddr)[i]);
			//}
			//fprintf(stderr,"\n");
			//use received data ...
			received++;
			tmp_packet = tmp_packet->next;
		}

		DWC_ETH_QOS_read_done(&dwc_eth_qos_pdev, qInx);
	}

	printf("\n\nReceived %d packet(s)\n\n", received);

	ret = DWC_ETH_QOS_detach(&dwc_eth_qos_pdev);

	printf("Done !!!!!!!!!!\n");

	return ret;
}
