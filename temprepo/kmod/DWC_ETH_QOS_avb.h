#ifndef __DWC_ETH_QOS__AVB__

#define __DWC_ETH_QOS__AVB__

/* IOCTL's */
#define DWC_ETH_QOS_BIND		_IOW('E', 200, int)
#define DWC_ETH_QOS_UNBIND		_IOW('E', 201, int)
#define DWC_ETH_QOS_MAP_TX_DESC		_IOW('E', 202, int)
#define DWC_ETH_QOS_UNMAP_TX_DESC	_IOW('E', 203, int)
#define DWC_ETH_QOS_MAP_BUF		_IOW('E', 204, int)
#define DWC_ETH_QOS_UNMAP_BUF		_IOW('E', 205, int)
#define DWC_ETH_QOS_LINK_PARAM		_IOW('E', 206, int)
#define DWC_ETH_QOS_MAP_RX_DESC		_IOW('E', 207, int)
#define DWC_ETH_QOS_UNMAP_RX_DESC	_IOW('E', 208, int)
#define DWC_ETH_QOS_RX_GETLOCK		_IOW('E', 209, int)
#define DWC_ETH_QOS_RX_TAKELOCK		_IOW('E', 210, int)

#define DWC_ETH_QOS_BIND_NAME_SIZE 24

/* data structure between umode driver and kmode driver
 * for handling all ioctl request
 * */

/*
 * Common structure between user and kernel for getting
 * data buffers and descriptor address.
 *
 * @phys_addr - dma/physical address of allocated buffer.
 * @mmap_size - total size of allocated memory.
 * @qInx - MTL queue or DMA channel number which is used for AV traffic
 * @alloc_size - no of bytes to be allocated
 * */
struct DWC_ETH_QOS_buf_cmd {
	u64 phys_addr;
	u32 qInx;
	u32 mmap_size;
	u32 alloc_size;
};

/*
 * @pdev_name - name of pci device
 * @mmap_size - device CSR address space
 * */
struct DWC_ETH_QOS_bind_cmd {
	char pdev_name[DWC_ETH_QOS_BIND_NAME_SIZE];
	u32 mmap_size;
};

/*
 * @up - link is up/down -
 * @speed - device speed- SPEED_10/100/1000/2500/10000/UNKNOWN
 * @duplex - DUPLEX_HALF/FULL/UNKNOWN
 * */
struct DWC_ETH_QOS_link_cmd {
	u32 up;
	u32 speed;
	u32 duplex;
};



/* internal structures for handling umode driver and macros */

#define DWC_ETH_QOS_MAX_AVB_Q_CNT 2

/*
 * structure to hold list of all the buffers allocated
 * for user.
 *
 * @addr - virtual address
 * @dma_addr - dma address
 * @size - size of data buffer in bytes
 * */
struct DWC_ETH_QOS_user_buff {
	struct DWC_ETH_QOS_user_buff *prev;
	struct DWC_ETH_QOS_user_buff *next;
	void *addr;
	dma_addr_t dma_addr;
	u32 size;
};

/*
 * @pdata - pointer to private data structure
 * @pdev_name - name of pci device
 * */
struct DWC_ETH_QOS_pdata_lookup {
	struct DWC_ETH_QOS_prv_data *pdata;
	char *pdev_name;
};


int DWC_ETH_QOS_misc_register(void);
void DWC_ETH_QOS_misc_unregister(void);
struct DWC_ETH_QOS_prv_data *DWC_ETH_QOS_search_pdata(char *id);

#endif
