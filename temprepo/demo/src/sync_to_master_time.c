#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <sys/user.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <netpacket/packet.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <sys/un.h>
#include <pthread.h>
#include <poll.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pci/pci.h>
#include "DWC_ETH_QOS_ycommon.h" /* for local data structure */
#include "avtp.h"
#include "session_mngr.h"

typedef struct { 
	int64_t ml_phoffset;
	int64_t ls_phoffset;
	int32_t ml_freqoffset;
	int32_t ls_freqoffset;
	int64_t local_time;
} gPtpTimeData;


/* global variables */
static int shm_fd = -1;
struct device dwc_eth_qos_pdev;
static char *memory_offset_buffer = NULL;
/* IEEE 1722 reserved address */
typedef enum { false = 0, true = 1 } bool;

#define SHM_SIZE 4*8 + sizeof(pthread_mutex_t) /* 3 - 64 bit and 2 - 32 bits */
#define SHM_NAME  "/ptp"

#define MAX_SAMPLE_VALUE ((1U << ((sizeof(int32_t)*8)-1))-1)
#define SRC_CHANNELS (2)
#define SAMPLES_PER_SECOND (48000)
#define FREQUENCY (480)
#define SAMPLES_PER_CYCLE (SAMPLES_PER_SECOND/FREQUENCY)
#define GAIN (.5)

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
		pci_fill_info(pdev, PCI_FILL_IDENT |
                                PCI_FILL_BASES | PCI_FILL_CLASS);
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
	DEBUG_PRN("\ngot SIGINT\n");
}

int main(int argc, char *argv[])
{
	gPtpTimeData td;
        uint64_t delay;
	uint64_t now_local;
	int8_t *interface = NULL;
        time_t time_to_set;
	int err;

	if (argc < 2) {
		DEBUG_PRN("%s < AVB interface name >\n", argv[0]);
		return -1;
	}

	interface = (int8_t *)strdup(argv[1]);

	err = pci_connect();
	if (err) {
		DEBUG_PRN("connect failed (%s) - are you running as root?\n",
                                strerror(errno));
		return (errno);
	}

	signal(SIGINT, sigint_handler);

        gptpinit();

        gptpscaling(&td);
        now_local = get_wallclock();
        delay = now_local - td.ml_phoffset;
        delay = delay/1000000000;
        time_to_set = delay;
        struct timeval tv = { time_to_set, 0 };

        settimeofday(&tv, 0);

	sleep(1);

        err = DWC_ETH_QOS_detach(&dwc_eth_qos_pdev);
        if (err)
                DEBUG_PRN("%s failed(%s) - is the driver loaded ?\n",
                                __func__, strerror(errno));

	gptpdeinit();

	return (0);
}
