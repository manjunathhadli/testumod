
//TODO: add header

/*!@file: DWC_ETH_QOS_avb.c
 * @brief: Driver functions.
 */
#include "DWC_ETH_QOS_yheader.h"

/* add new entry into the list */
static void DWC_ETH_QOS_add_ubuff_entry(struct DWC_ETH_QOS_prv_data *pdata,
					struct DWC_ETH_QOS_user_buff *ubuff)
{
	DBGPR_CHDRV("-->DWC_ETH_QOS_add_ubuff_entry\n");

	/*
	 * This is how all data bffers are stored in doubly link list
	 *
	 *	0[next, prev, ...]
	 *	   +	 |
	 *	   |	 +
	 *	1[next, prev, ...]
	 *	   +     |
	 *	   |	 +
	 *	2[next, prev, ...]
	 *	   +     |
	 *	   |	 +
	 *	3[next, prev, ...]
	 *
	 * ubuff->prev points to next node
	 * uuff->next points to previous node and
	 * pdata->uuff points to last/latest buffer allocated.
	 * */
	if (pdata->ubuff == NULL) {
		pdata->ubuff = ubuff;
	} else {
		ubuff->next = pdata->ubuff;
		pdata->ubuff->prev = ubuff;
		pdata->ubuff = ubuff;
	}

	DBGPR_CHDRV("<--DWC_ETH_QOS_add_ubuff_entry\n");
}


/* take out entry from the list */
static void DWC_ETH_QOS_delet_ubuff_entry(struct DWC_ETH_QOS_prv_data *pdata,
					struct DWC_ETH_QOS_user_buff *ubuff)
{
	DBGPR_CHDRV("-->DWC_ETH_QOS_delet_ubuff_entry\n");

	if (ubuff->prev)
		ubuff->prev->next = ubuff->next;

	if (ubuff->next)
		ubuff->next->prev = ubuff->prev;

	if (ubuff == pdata->ubuff)
		pdata->ubuff = ubuff->next;

	vfree(ubuff);

	DBGPR_CHDRV("<--DWC_ETH_QOS_delet_ubuff_entry\n");
}


/* This api dose following,
 * a. search for private data structure using req.pdev_name and
 *    updates valid pdata in file->private_data. This stored pdata
 *    is used later in the driver for hanlding other IOCTLs.
 * b. gets the device CSR space and return it to user space through
 *    req.mmap_size.
 *
 * return zero on success and -ve error number on failure
 * */
static int DWC_ETH_QOS_bind(struct file *file, void __user *argp)
{
	struct DWC_ETH_QOS_prv_data *pdata = NULL;
	struct DWC_ETH_QOS_bind_cmd req;
	int ret = 0;

	DBGPR_CHDRV("-->DWC_ETH_QOS_bind\n");

	if (copy_from_user(&req, argp, sizeof(req))) {
		printk(KERN_ALERT "copy_from_user() failed\n");
		return -EFAULT;
	}

	/* get pdata */
	pdata = DWC_ETH_QOS_search_pdata(req.pdev_name);
	if (pdata == NULL) {
		printk(KERN_ALERT "failed to get valid pdata\n");
		ret = -EFAULT;
		goto pdata_failed;
	}

	file->private_data = pdata;

	/* update device CSR space */
	req.mmap_size = pci_resource_len(pdata->pdev, pdata->bar_no);

	if (copy_to_user(argp, &req, sizeof(req))) {
		printk(KERN_ALERT "copy_to_user() failed\n");
		ret = -EFAULT;
		goto copy_failed;
	}

	DBGPR_CHDRV("<--DWC_ETH_QOS_bind\n");

	return 0;

copy_failed:
	file->private_data = NULL;

pdata_failed:
	return ret;
}


/* Release driver pdata structure.
 *
 * return zero on success and -ve error number on failure
 * */
static int DWC_ETH_QOS_unbind(struct file *file)
{
	struct DWC_ETH_QOS_prv_data *pdata = file->private_data;

	if (pdata == NULL)
		return -EBADFD;

	file->private_data = NULL;

	return 0;
}


/* return zero on success and -ve error number on failure */
static int DWC_ETH_QOS_get_link_param(struct file *file, void __user *arg)
{
	struct DWC_ETH_QOS_prv_data *pdata = file->private_data;
	struct DWC_ETH_QOS_link_cmd req;

	DBGPR_CHDRV("-->DWC_ETH_QOS_get_link_param\n");

	if (pdata == NULL) {
		printk(KERN_ALERT "map to unbound device\n");
		return -ENOENT;
	}

	/* get all link parameters from PHYLIB */
	req.up = pdata->phydev->link;
	req.speed = pdata->phydev->speed;
	req.duplex = pdata->phydev->duplex;

	if (copy_to_user(arg, &req, sizeof(req))) {
		printk(KERN_ALERT "copy_to_user() failed\n");
		return -EFAULT;
	}

	DBGPR_CHDRV("<--DWC_ETH_QOS_get_link_param\n");

	return 0;
}


/* return zero on success and -ve error number on failure */
static long DWC_ETH_QOS_map_tx_desc(struct DWC_ETH_QOS_prv_data *pdata,
				struct DWC_ETH_QOS_buf_cmd *req)
{
	int qInx = req->qInx;

	DBGPR_CHDRV("-->DWC_ETH_QOS_map_tx_desc\n");

	if (qInx > pdata->max_tx_queue_cnt) {
		printk(KERN_ALERT "Invalid queue(%d) specified\n"
			"Max number of Tx queue supported by device = %d\n",
			qInx, pdata->max_tx_queue_cnt);
		return -EINVAL;
	}

	if (pdata->tx_avb_queue & (1 << qInx)) {
		printk(KERN_ALERT "Tx descriptor memory for this queue(%d) is\
			alreday in allocated\n", qInx);
		return -EBUSY;
	}

	/* allocate descriptor memory */
	GET_TX_DESC_PTR(qInx, 0) = dma_alloc_coherent(&(pdata->pdev->dev),
					(sizeof(struct s_TX_NORMAL_DESC) * TX_DESC_CNT),
					&(GET_TX_DESC_DMA_ADDR(qInx, 0)),
					GFP_KERNEL);
	if (GET_TX_DESC_PTR(qInx, 0) == NULL) {
		return -ENOMEM;
	}

	pdata->tx_avb_queue |= (1 << qInx);

	/* get required fields for umode driver */
	req->phys_addr = GET_TX_DESC_DMA_ADDR(qInx, 0);
	req->mmap_size = (TX_DESC_CNT * sizeof(struct s_TX_NORMAL_DESC));

	DBGPR_CHDRV("<--DWC_ETH_QOS_map_tx_desc\n");

	return 0;
}


/* return zero on success and -ve error number on failure */
static long DWC_ETH_QOS_map_rx_desc(struct DWC_ETH_QOS_prv_data *pdata,
				struct DWC_ETH_QOS_buf_cmd *req)
{
	int qInx = req->qInx;

	DBGPR_CHDRV("-->DWC_ETH_QOS_map_rx_desc\n");

	if (qInx > pdata->max_rx_queue_cnt) {
		printk(KERN_ALERT "Invalid queue(%d) specified\n"
			"Max number of Rx queue supported by device = %d\n",
			qInx, pdata->max_rx_queue_cnt);
		return -EINVAL;
	}

	if (pdata->rx_avb_queue & (1 << qInx)) {
		printk(KERN_ALERT "Rx descriptor memory for this queue(%d) is\
			alreday in allocated\n", qInx);
		return -EBUSY;
	}

	/* allocate descriptor memory */
	GET_RX_DESC_PTR(qInx, 0) = dma_alloc_coherent(&(pdata->pdev->dev),
					(sizeof(struct s_RX_NORMAL_DESC) * RX_DESC_CNT),
					&(GET_RX_DESC_DMA_ADDR(qInx, 0)),
					GFP_KERNEL);
	if (GET_RX_DESC_PTR(qInx, 0) == NULL) {
		return -ENOMEM;
	}

	pdata->rx_avb_queue |= (1 << qInx);

	/* get required fields for umode driver */
	req->phys_addr = GET_RX_DESC_DMA_ADDR(qInx, 0);
	req->mmap_size = (RX_DESC_CNT * sizeof(struct s_RX_NORMAL_DESC));

	DBGPR_CHDRV("<--DWC_ETH_QOS_map_rx_desc\n");

	return 0;
}


/* return zero on success and -ve error number on failure */
static long DWC_ETH_QOS_map_desc(struct file *file,
					void __user *arg,
					int cmd)
{
	struct DWC_ETH_QOS_prv_data *pdata = file->private_data;
	struct DWC_ETH_QOS_buf_cmd req;
	int ret = 0;

	DBGPR_CHDRV("-->DWC_ETH_QOS_map_desc\n");

	if (pdata == NULL) {
		printk(KERN_ALERT "map to unbound device\n");
		return -ENOENT;
	}

	if (copy_from_user(&req, arg, sizeof(req))) {
		printk(KERN_ALERT "copy_from_user() failed\n");
		return -EFAULT;
	}

	if (cmd == DWC_ETH_QOS_MAP_TX_DESC)
		ret = DWC_ETH_QOS_map_tx_desc(pdata, &req);
	else
		ret = DWC_ETH_QOS_map_rx_desc(pdata, &req);

	if (copy_to_user(arg, &req, sizeof(req))) {
		printk(KERN_ALERT "copy_to_user() failed\n");
		return -EFAULT;
	}

	DBGPR_CHDRV("<--DWC_ETH_QOS_map_desc\n");

	return ret;
}


/* return zero on success and -ve error number on failure */
static int DWC_ETH_QOS_unmap_tx_desc(struct DWC_ETH_QOS_prv_data *pdata,
					struct DWC_ETH_QOS_buf_cmd *req)
{
	int qInx = req->qInx;

	DBGPR_CHDRV("-->DWC_ETH_QOS_unmap_tx_desc\n");

	if (qInx > pdata->max_tx_queue_cnt) {
		printk(KERN_ALERT "Invalid queue(%d) specified\n"
			"Max number of tx queue supported by device = %d\n",
			qInx, pdata->max_tx_queue_cnt);
		return -EINVAL;
	}

	if ((pdata->tx_avb_queue & (1 << qInx)) == 0) {
		printk(KERN_ALERT "Tx descriptor memory for this queue(%d) is\
			alreday in freed\n", qInx);
		return -EBUSY;
	}

	/* free descriptor memory */
	if (GET_TX_DESC_PTR(qInx, 0)) {
		dma_free_coherent(&(pdata->pdev->dev),
				  (sizeof(struct s_TX_NORMAL_DESC) * TX_DESC_CNT),
				  GET_TX_DESC_PTR(qInx, 0),
				  GET_TX_DESC_DMA_ADDR(qInx, 0));
		GET_TX_DESC_PTR(qInx, 0) = NULL;
	}

	pdata->tx_avb_queue &= ~(1 << qInx);

	DBGPR_CHDRV("<--DWC_ETH_QOS_unmap_tx_desc\n");

	return 0;
}


/* return zero on success and -ve error number on failure */
static int DWC_ETH_QOS_unmap_rx_desc(struct DWC_ETH_QOS_prv_data *pdata,
					struct DWC_ETH_QOS_buf_cmd *req)
{
	int qInx = req->qInx;

	DBGPR_CHDRV("-->DWC_ETH_QOS_unmap_rx_desc\n");

	if (qInx > pdata->max_rx_queue_cnt) {
		printk(KERN_ALERT "Invalid queue(%d) specified\n"
			"Max number of rx queue supported by device = %d\n",
			qInx, pdata->max_rx_queue_cnt);
		return -EINVAL;
	}

	if ((pdata->rx_avb_queue & (1 << qInx)) == 0) {
		printk(KERN_ALERT "Rx descriptor memory for this queue(%d) is\
			alreday in freed\n", qInx);
		return -EBUSY;
	}

	/* free descriptor memory */
	if (GET_RX_DESC_PTR(qInx, 0)) {
		dma_free_coherent(&(pdata->pdev->dev),
				  (sizeof(struct s_RX_NORMAL_DESC) * RX_DESC_CNT),
				  GET_RX_DESC_PTR(qInx, 0),
				  GET_RX_DESC_DMA_ADDR(qInx, 0));
		GET_RX_DESC_PTR(qInx, 0) = NULL;
	}

	pdata->rx_avb_queue &= ~(1 << qInx);

	DBGPR_CHDRV("<--DWC_ETH_QOS_unmap_rx_desc\n");

	return 0;
}


/* return zero on success and -ve error number on failure */
static int DWC_ETH_QOS_unmap_desc(struct file *file,
				void __user *arg,
				int cmd)
{
	struct DWC_ETH_QOS_prv_data *pdata = file->private_data;
	struct DWC_ETH_QOS_buf_cmd req;
	int ret = 0;

	DBGPR_CHDRV("-->DWC_ETH_QOS_unmap_desc\n");

	if (pdata == NULL) {
		printk(KERN_ALERT "map to unbound device\n");
		return -ENOENT;
	}

	if (copy_from_user(&req, arg, sizeof(req))) {
		printk(KERN_ALERT "copy_from_user() failed\n");
		return -EFAULT;
	}

	if (cmd == DWC_ETH_QOS_UNMAP_TX_DESC)
		ret = DWC_ETH_QOS_unmap_tx_desc(pdata, &req);
	else
		ret = DWC_ETH_QOS_unmap_rx_desc(pdata, &req);

	DBGPR_CHDRV("<--DWC_ETH_QOS_unmap_desc\n");

	return ret;
}


/* This API will allocate "arg->alloc_size" bytes of data
 * for application and fills other parameters of arg.
 *
 * return zero on success and -ve error number on failure
 * */
static int DWC_ETH_QOS_map_buf(struct file *file,
				void __user *arg)
{
	struct DWC_ETH_QOS_prv_data *pdata = file->private_data;
	struct DWC_ETH_QOS_user_buff *ubuff = NULL;
	struct DWC_ETH_QOS_buf_cmd req;
	void *addr = NULL;
	dma_addr_t dma_addr;
	u32 size = 0;
	int ret = 0;

	DBGPR_CHDRV("-->DWC_ETH_QOS_map_buf\n");

	if (pdata == NULL) {
		printk(KERN_ALERT "map to unbound device\n");
		return -ENOENT;
	}

	if (copy_from_user(&req, arg, sizeof(req))) {
		printk(KERN_ALERT "copy_from_user() failed\n");
		return -EFAULT;
	}

	ubuff = vzalloc(sizeof(struct DWC_ETH_QOS_user_buff));
	if (unlikely(!ubuff)) {
		ret = -ENOMEM;
		printk(KERN_ALERT "failed to allocate user_buff\n");
		goto ubuff_alloc_failed;
	}

	DWC_ETH_QOS_add_ubuff_entry(pdata, ubuff);

	/* get how much memory to be allocated */
	size = req.alloc_size;
	addr = dma_alloc_coherent((&pdata->pdev->dev), size,
				&dma_addr, GFP_KERNEL);
	if (unlikely(!addr)) {
		ret = -ENOMEM;
		printk(KERN_ALERT "failed to allocate user data buffer\n");
		goto dma_alloc_failed;
	}

	pdata->ubuff->addr = addr;
	pdata->ubuff->dma_addr = dma_addr;
	pdata->ubuff->size = size;

	req.phys_addr = dma_addr;
	req.mmap_size = size;

	if (copy_to_user(arg, &req, sizeof(req))) {
		ret = -EFAULT;
		printk(KERN_ALERT "copy_to_user() failed\n");
		goto copy_faliled;
	}

	DBGPR_CHDRV("<--DWC_ETH_QOS_map_buf\n");

	return 0;

copy_faliled:
	dma_free_coherent((&pdata->pdev->dev), size, addr, dma_addr);

dma_alloc_failed:
	DWC_ETH_QOS_delet_ubuff_entry(pdata, ubuff);

ubuff_alloc_failed:
	return ret;
}


/* return zero on success and -ve error number on failure */
static int DWC_ETH_QOS_unmap_buf(struct file *file,
				void __user *arg)
{
	struct DWC_ETH_QOS_prv_data *pdata = file->private_data;
	struct DWC_ETH_QOS_user_buff *ubuff = NULL;
	struct DWC_ETH_QOS_buf_cmd req;
	int ret = 0;

	DBGPR_CHDRV("-->DWC_ETH_QOS_unmap_buf\n");

	if (pdata == NULL) {
		printk(KERN_ALERT "map to unbound device\n");
		return -ENOENT;
	}

	if (copy_from_user(&req, arg, sizeof(req))) {
		printk(KERN_ALERT "copy_from_user() failed\n");
		return -EFAULT;
	}

	ubuff = pdata->ubuff;
	/* find the corresponding buffer and free */
	while (ubuff != NULL) {
		if (req.phys_addr == ubuff->dma_addr)
			break;
		ubuff = ubuff->next;
	}

	if (ubuff == NULL) {
		printk(KERN_ALERT "already freed or not a valid buffer\n");
		return -EINVAL;
	}

	dma_free_coherent((&pdata->pdev->dev), ubuff->size,
			ubuff->addr, ubuff->dma_addr);

	DWC_ETH_QOS_delet_ubuff_entry(pdata, ubuff);

	DBGPR_CHDRV("<--DWC_ETH_QOS_unmap_buf\n");

	return ret;
}


/* return zero on success and -ve error number on failure */
static int DWC_ETH_QOS_rx_getlock(struct file *file, void __user *arg)
{
	struct DWC_ETH_QOS_prv_data *pdata = file->private_data;
	int sem_idx = 0, qInx;
	int ret = 0;

	DBGPR_CHDRV("-->DWC_ETH_QOS_rx_getlock\n");

	if (pdata == NULL) {
		printk(KERN_ALERT "map to unbound device\n");
		return -ENOENT;
	}

	if (copy_from_user(&qInx, arg, sizeof(qInx))) {
		printk(KERN_ALERT "copy_from_user() failed\n");
		return -EFAULT;
	}
	DBGPR_CHDRV("qInx = %d\n", qInx);

	sem_idx = (DWC_ETH_QOS_MAX_AVB_Q_CNT -
			(pdata->max_rx_queue_cnt - qInx));
	/* acquire semaphore */
	ret = down_interruptible(&pdata->rx_sem[sem_idx]);

	DBGPR_CHDRV("<--DWC_ETH_QOS_rx_getlock\n");

	return ret;
}


/* return zero on success and -ve error number on failure */
static int DWC_ETH_QOS_rx_setlock(struct file *file, void __user *arg)
{
	struct DWC_ETH_QOS_prv_data *pdata = file->private_data;
	int sem_idx = 0, qInx;
	int ret = 0;

	DBGPR_CHDRV("-->DWC_ETH_QOS_rx_setlock\n");

	if (pdata == NULL) {
		printk(KERN_ALERT "map to unbound device\n");
		return -ENOENT;
	}

	if (copy_from_user(&qInx, arg, sizeof(qInx))) {
		printk(KERN_ALERT "copy_from_user() failed\n");
		return -EFAULT;
	}
	DBGPR_CHDRV("qInx = %d\n", qInx);

	sem_idx = (DWC_ETH_QOS_MAX_AVB_Q_CNT -
			(pdata->max_rx_queue_cnt - qInx));
	/* release semaphore */
	up(&pdata->rx_sem[sem_idx]);

	DBGPR_CHDRV("<--DWC_ETH_QOS_rx_setlock\n");

	return ret;
}


/* user mode API routines */

static ssize_t DWC_ETH_QOS_read_file(struct file * file,
					char __user * buf,
					size_t count, loff_t *pos)
{
	/* don't support reads for any status or data */
	return -EINVAL;
}


static ssize_t DWC_ETH_QOS_write_file(struct file * file,
					const char __user * buf,
					size_t count, loff_t *pos)
{
	/* don't support writes for any status or data */
	return -EINVAL;
}


static unsigned int DWC_ETH_QOS_poll_file(struct file *file,
					poll_table *wait)
{	/* don't support reads for any status or data */
	return -EINVAL;
}


static int DWC_ETH_QOS_open_file(struct inode *inode,
				struct file *file)
{
	file->private_data = NULL;
	return 0;
}


/* return zero on success and -ve error number on failure */
static int DWC_ETH_QOS_release_file(struct inode *inode,
					struct file *file)
{
	struct DWC_ETH_QOS_prv_data *pdata = file->private_data;
	struct DWC_ETH_QOS_user_buff *ubuff = NULL;
	int ret = 0;
	int qInx;

	DBGPR_CHDRV("-->DWC_ETH_QOS_release_file\n");

	if (pdata == NULL) {
		printk(KERN_ALERT "map to unbound device\n");
		return 0;
	}

	/* free descriptor memory of all used queue */
	for (qInx = pdata->tx_avb_q_idx; qInx < pdata->tx_queue_cnt; qInx++) {
		if (pdata->tx_avb_queue & (1 << qInx)) {
			if (GET_TX_DESC_PTR(qInx, 0)) {
				dma_free_coherent(&(pdata->pdev->dev),
					(sizeof(struct s_TX_NORMAL_DESC) *
						TX_DESC_CNT),
					GET_TX_DESC_PTR(qInx, 0),
					GET_TX_DESC_DMA_ADDR(qInx, 0));
				GET_TX_DESC_PTR(qInx, 0) = NULL;
			}
		}
		pdata->tx_avb_queue &= ~(1 << qInx);
	}

	ubuff = pdata->ubuff;
	/* free data buffers */
	while (ubuff != NULL) {
		dma_free_coherent((&pdata->pdev->dev), ubuff->size,
				ubuff->addr, ubuff->dma_addr);
		DWC_ETH_QOS_delet_ubuff_entry(pdata, ubuff);
		ubuff = pdata->ubuff;
	}

	ret = DWC_ETH_QOS_unbind(file);

	DBGPR_CHDRV("<--DWC_ETH_QOS_release_file\n");

	return ret;
}

static void DWC_ETH_QOS_vm_open(struct vm_area_struct *vma)
{
	/* nothing to do */
}


static void DWC_ETH_QOS_vm_close(struct vm_area_struct *vma)
{
	/* nothing to do */
}


static int DWC_ETH_QOS_vm_fault(struct vm_area_struct *area,
				struct vm_fault *fdata)
{
	/* nothing to do */
	return VM_FAULT_SIGBUS;
}


static struct vm_operations_struct DWC_ETH_QOS_mmap_ops = {
	.open = DWC_ETH_QOS_vm_open,
	.close = DWC_ETH_QOS_vm_close,
	.fault = DWC_ETH_QOS_vm_fault
};


/* return zero on success and -ve error number on failure */
static int DWC_ETH_QOS_mmap_file(struct file *file,
				struct vm_area_struct *vma)
{
	struct DWC_ETH_QOS_prv_data *pdata = file->private_data;
	unsigned long size = vma->vm_end - vma->vm_start;
	dma_addr_t pgoff = vma->vm_pgoff;
	dma_addr_t phys_addr;

	DBGPR_CHDRV("-->DWC_ETH_QOS_mmap_file\n");

	if (pdata == NULL) {
		printk(KERN_ALERT "map to unbound device\n");
		return - ENODEV;
	}

	if (pgoff == 0)
		phys_addr = pci_resource_start(pdata->pdev, pdata->bar_no)
				>> PAGE_SHIFT;
	else
		phys_addr = pgoff;

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	if (remap_pfn_range(vma, vma->vm_start, phys_addr,
			size, vma->vm_page_prot))
		return -EAGAIN;

	vma->vm_ops = &DWC_ETH_QOS_mmap_ops;

	DBGPR_CHDRV("<--DWC_ETH_QOS_mmap_file\n");

	return 0;
}


/* return zero on success and -ve error number on failure */
static long DWC_ETH_QOS_ioctl_file(struct file *file,
				unsigned int cmd,
				unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	long ret;

	DBGPR_CHDRV("-->DWC_ETH_QOS_ioctl_file\n");

	switch(cmd) {
	case DWC_ETH_QOS_BIND:
		ret = DWC_ETH_QOS_bind(file, argp);
		break;
	case DWC_ETH_QOS_UNBIND:
		ret = DWC_ETH_QOS_unbind(file);
		break;
	case DWC_ETH_QOS_LINK_PARAM:
		ret = DWC_ETH_QOS_get_link_param(file, argp);
		break;
	case DWC_ETH_QOS_MAP_TX_DESC:
	case DWC_ETH_QOS_MAP_RX_DESC:
		ret = DWC_ETH_QOS_map_desc(file, argp, cmd);
		break;
	case DWC_ETH_QOS_UNMAP_TX_DESC:
	case DWC_ETH_QOS_UNMAP_RX_DESC:
		ret = DWC_ETH_QOS_unmap_desc(file, argp, cmd);
		break;
	case DWC_ETH_QOS_MAP_BUF:
		ret = DWC_ETH_QOS_map_buf(file, argp);
		break;
	case DWC_ETH_QOS_UNMAP_BUF:
		ret = DWC_ETH_QOS_unmap_buf(file, argp);
		break;
	case DWC_ETH_QOS_RX_GETLOCK:
		ret = DWC_ETH_QOS_rx_getlock(file, argp);
		break;
	case DWC_ETH_QOS_RX_TAKELOCK:
		ret = DWC_ETH_QOS_rx_setlock(file, argp);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	DBGPR_CHDRV("<--DWC_ETH_QOS_ioctl_file\n");

	return ret;
}

static struct file_operations DWC_ETH_QOS_fops = {
	.owner = THIS_MODULE,
	.llseek = no_llseek,
	.read = DWC_ETH_QOS_read_file,
	.write = DWC_ETH_QOS_write_file,
	.poll = DWC_ETH_QOS_poll_file,
	.open = DWC_ETH_QOS_open_file,
	.release = DWC_ETH_QOS_release_file,
	.mmap = DWC_ETH_QOS_mmap_file,
	.unlocked_ioctl = DWC_ETH_QOS_ioctl_file,
};

static struct miscdevice DWC_ETH_QOS_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "dwc_eth_qos_avb",
	.fops = &DWC_ETH_QOS_fops,
};


int DWC_ETH_QOS_misc_register(void)
{
	return misc_register(&DWC_ETH_QOS_miscdev);
}


void DWC_ETH_QOS_misc_unregister(void)
{
	misc_deregister(&DWC_ETH_QOS_miscdev);
}
