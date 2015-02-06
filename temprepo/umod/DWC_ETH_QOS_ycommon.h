#ifndef __DWC_ETH_QOS__COMMON__H__

#define __DWC_ETH_QOS__COMMON__H__

#define DWC_ETH_QOS_BIND_NAME_SIZE 24

#define TX_DESC_CNT 512 //This count should be same kmode driver count
#define RX_DESC_CNT 512 //This count should be same kmode driver count

/* uncommnet this to check device time to transmit one packet */
//#define TS_CAPTURE

/*
 * @private_data - pointer to user mode driver private data structure
 * @vendor_id - pci vendor id
 * @device_id - pci device id
 * @domain - pci domain id
 * @bus - pci bus id
 * @dev - pci device id
 * @func - pci function id
 */
struct device {
	void *private_data;
	u_int16_t vendor_id;
	u_int16_t device_id;
	u_int16_t domain;
	u_int8_t bus;
	u_int8_t dev;
	u_int8_t func;
};

/*
 * @paddr - physical address of buffer
 * @mmap_size - total size
 */
struct resources {
	u_int64_t paddr;
	u_int32_t mmap_size;
};

/*
 * Structure which holds all the details of packet to
 * be transmitted.
 *
 * @map - pointer to struct resources
 * @offset - offset in the total buffer space
 * @vaddr - pointer to data
 * @len - length of data in bytes
 * @flags - flags
 * @attime -
 * @dmatime -
 * @next - pointer to next packet
 */
struct DWC_ETH_QOS_packet {
	struct resources map;
	unsigned int offset;
	void *vaddr;
	u_int32_t len;
	u_int32_t flags;
	u_int32_t attime;
	u_int32_t dmatime;
#ifdef TS_CAPTURE
        u_int64_t submit_time;
        u_int64_t transmitted_time;
        u_int64_t pkt_no1;
        u_int64_t pkt_no2;
#endif
	struct DWC_ETH_QOS_packet *next;
};


/*
 * Common structure between user and kernel for getting
 * data buffers and descriptor address.
 *
 * @phys_addr - dma/physical address of allocated buffer.
 * @mmap_size - total size of allocated memory.
 * @qInx - MTL queue or DMA channel number which is used for AV traffic
 * @alloc_size - no of bytes to be allocated
 * * */
struct DWC_ETH_QOS_user_buff {
	u_int64_t dma_addr;
	void *addr;
	unsigned int mmap_size;
	unsigned int alloc_size;
};


/* List of library functions which are exposed to application */
int DWC_ETH_QOS_probe(struct device *pdev);
int DWC_ETH_QOS_attach(char *dev_path, struct device *pdev);
int DWC_ETH_QOS_detach(struct device *pdev);
int DWC_ETH_QOS_suspend(struct device *pdev);
int DWC_ETH_QOS_resume(struct device *pdev);
int DWC_ETH_QOS_init(struct device *pdev);
int DWC_ETH_QOS_exit(struct device *pdev);
int DWC_ETH_QOS_test_reg_read(struct device *pdev);
int DWC_ETH_QOS_test_reg_write(struct device *pdev);
int DWC_ETH_QOS_get_buffer(struct device *pdev,
			struct DWC_ETH_QOS_user_buff *ubuff);
int DWC_ETH_QOS_free_buffer(struct device *pdev,
			struct DWC_ETH_QOS_user_buff *ubuff);
int DWC_ETH_QOS_start_xmit(struct device *pdev, unsigned int qInx,
			struct DWC_ETH_QOS_packet *packet);
int DWC_ETH_QOS_tx_buffer_cleanup(struct device *pdev,
				unsigned int qInx,
				struct DWC_ETH_QOS_packet **cleaned_packet);
struct DWC_ETH_QOS_packet * DWC_ETH_QOS_read(struct device *pdev,
					unsigned int qInx,
					unsigned int quota);
int DWC_ETH_QOS_read_done(struct device *pdev, unsigned int qInx);

unsigned long long get_wallclock();
int DWC_ETH_QOS_program_CBS_alogorithm(struct device *pdev,
          u_int8_t qInx, unsigned int bw);
void set_tx_queue_operating_mode(u_int8_t qInx, unsigned int q_mode);
void set_avb_algorithm(u_int8_t qInx, unsigned char avb_algo);
void config_credit_control(u_int8_t qInx, unsigned int cc);
void config_send_slope(u_int8_t qInx, unsigned int sendSlope);
void config_idle_slope(u_int8_t qInx, unsigned int idleSlope);
void config_high_credit(u_int8_t qInx, unsigned int hiCredit);
void config_low_credit(u_int8_t qInx, unsigned int lowCredit);
#endif
