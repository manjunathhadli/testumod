#include "DWC_ETH_QOS_yheader.h"
#include "DWC_ETH_QOS_yregacc.h"
#include "DWC_ETH_QOS_dev.h"
        
/* global variables */
unsigned long pci_base_addr = 0x0;
u_int64_t pkt_no;

static struct DWC_ETH_QOS_vendor_info DWC_ETH_QOS_vendor_info[] = {
	{SNPS_VENDOR_ID, SNPS_DWC_ETH_QOS_DEVICE_ID},
	{0,0}
};


/* This api compare the given pci vendor_id and device_id
 * with its own device vendor_id and device_id for which the
 * driver is written.
 *
 * return zero if given pdev->vendor_id and pdev->device_id
 * matches with driver supported id's else returns +ve
 * error number
 * */
int DWC_ETH_QOS_probe(struct device *pdev)
{
	struct DWC_ETH_QOS_vendor_info *info = NULL;

	if (pdev == NULL)
		return EINVAL;

	if (pdev->vendor_id != SNPS_VENDOR_ID)
		return (ENXIO);

	info = DWC_ETH_QOS_vendor_info;
	while (info->vendor_id != 0) {
		if ((pdev->vendor_id == info->vendor_id) &&
		    (pdev->device_id == info->device_id))
			return 0;
		info++;
	}

	return (ENXIO);
}


/* This api allocate wrapper queue structure.
 *
 * return zero on success else +ve error number on failure
 * */
static int DWC_ETH_QOS_alloc_tx_queue_struct(struct DWC_ETH_QOS_prv_data *pdata)
{
	int ret = 0;

	DBGPR("-->%s\n", __func__);

	/* allocate tx queue structure */
	pdata->tx_queue = calloc(pdata->tx_avb_q_cnt,
				sizeof(struct DWC_ETH_QOS_tx_queue));
	if (pdata->tx_queue == NULL) {
		printf("Failed to allocate tx queue structure\n");
		ret = ENOMEM;
	}

	DBGPR("<--%s\n", __func__);

	return ret;
}


static void DWC_ETH_QOS_free_tx_queue(struct DWC_ETH_QOS_prv_data *pdata)
{
	DBGPR("-->%s\n", __func__);

	if (pdata->tx_queue != NULL) {
		free(pdata->tx_queue);
		pdata->tx_queue = NULL;
	}

	DBGPR("<--%s\n", __func__);
}


/* This api allocate wrapper queue structure.
 *
 * return zero on success else +ve error number on failure
 * */
static int DWC_ETH_QOS_alloc_rx_queue_struct(struct DWC_ETH_QOS_prv_data *pdata)
{
	int ret = 0;

	DBGPR("-->%s\n", __func__);

	/* allocate rx queue structure */
	pdata->rx_queue = calloc(pdata->rx_avb_q_cnt,
				sizeof(struct DWC_ETH_QOS_rx_queue));
	if (pdata->rx_queue == NULL) {
		printf("Failed to allocate rx queue structure\n");
		ret = ENOMEM;
	}

	DBGPR("<--%s\n", __func__);

	return ret;
}


static void DWC_ETH_QOS_free_rx_queue(struct DWC_ETH_QOS_prv_data *pdata)
{
	DBGPR("-->%s\n", __func__);

	if (pdata->rx_queue != NULL) {
		free(pdata->rx_queue);
		pdata->rx_queue = NULL;
	}

	DBGPR("<--%s\n", __func__);
}


static void DWC_ETH_QOS_free_tx_buff_and_desc_single_q(
			struct DWC_ETH_QOS_prv_data *pdata,
			u_int8_t qInx)
{
	struct DWC_ETH_QOS_buf_cmd ubuff;

	DBGPR("-->%s\n", __func__);

	if (GET_TX_DESC_PTR(qInx, 0) != NULL)
		munmap(GET_TX_DESC_PTR(qInx, 0), GET_TX_DESC_SIZE(qInx));

	ubuff.qInx = (qInx + pdata->tx_avb_q_idx);
	ioctl(pdata->dev, DWC_ETH_QOS_UNMAP_TX_DESC, &ubuff);

	if (GET_TX_BUF_PTR(qInx, 0) != NULL)
		free(GET_TX_BUF_PTR(qInx, 0));

	DBGPR("<--%s\n", __func__);
}


static void DWC_ETH_QOS_free_tx_buff_and_desc(
		struct DWC_ETH_QOS_prv_data *pdata
		)
{
	int qInx;

	for (qInx = 0; qInx < pdata->tx_avb_q_cnt; qInx++)
		DWC_ETH_QOS_free_tx_buff_and_desc_single_q(pdata, qInx);
}


static void DWC_ETH_QOS_free_rx_buff_and_desc_single_q(
			struct DWC_ETH_QOS_prv_data *pdata,
			u_int8_t qInx)
{
	struct DWC_ETH_QOS_user_buff *data_buff = NULL;
	struct DWC_ETH_QOS_buf_cmd ubuff;

	DBGPR("-->%s\n", __func__);

	if (GET_RX_DESC_PTR(qInx, 0) != NULL)
		munmap(GET_RX_DESC_PTR(qInx, 0), GET_RX_DESC_SIZE(qInx));

	ubuff.qInx = (qInx + pdata->rx_avb_q_idx);
	ioctl(pdata->dev, DWC_ETH_QOS_UNMAP_RX_DESC, &ubuff);

	/* free rx wrapper buffer structure */
	if (GET_RX_BUF_PTR(qInx, 0) != NULL)
		free(GET_RX_BUF_PTR(qInx, 0));

	/* free rx data buffer */
	data_buff = GET_RX_DATA_BUF_PTR(qInx, 0);
	DWC_ETH_QOS_free_buffer(pdata->pdev, data_buff);

	DBGPR("<--%s\n", __func__);
}


static void DWC_ETH_QOS_free_rx_buff_and_desc(
		struct DWC_ETH_QOS_prv_data *pdata
		)
{
	int qInx;

	for (qInx = 0; qInx < pdata->rx_avb_q_cnt; qInx++)
		DWC_ETH_QOS_free_rx_buff_and_desc_single_q(pdata, qInx);
}


/* This api release the transmit descriptor memory, wrapper buffer
 * and queue memory
 * */
static void DWC_ETH_QOS_free_tx_resourses(struct DWC_ETH_QOS_prv_data *pdata)
{
	DWC_ETH_QOS_free_tx_buff_and_desc(pdata);
	DWC_ETH_QOS_free_tx_queue(pdata);
}


/* This api release the receive descriptor memory, wrapper buffer
 * and queue memory
 * */
static void DWC_ETH_QOS_free_rx_resourses(struct DWC_ETH_QOS_prv_data *pdata)
{
	DWC_ETH_QOS_free_rx_buff_and_desc(pdata);
	DWC_ETH_QOS_free_rx_queue(pdata);
}


/* This api issue command to get tx descriptor memory from kernel space
 * and also allocates wrapper buffer structures.
 *
 * return zero on success else +ve error number on failure
 * */
static int DWC_ETH_QOS_alloc_tx_buff_and_desc(struct DWC_ETH_QOS_prv_data *pdata)
{
	struct DWC_ETH_QOS_buf_cmd ubuff;
	int ret = 0;
	int qInx;

	DBGPR("-->%s\n", __func__);

	/* allocate tx descriptor and wrapper buffer */
	for (qInx = 0; qInx < pdata->tx_avb_q_cnt; qInx++) {
		ubuff.qInx = qInx + pdata->tx_avb_q_idx;
		ret = ioctl(pdata->dev, DWC_ETH_QOS_MAP_TX_DESC, &ubuff);
		if (ret < 0) {
			printf("tx desc map ioctl failed\n");
			ret = EBUSY;
			goto tx_failed;
		}

		GET_TX_DESC_DMA_ADDR(qInx, 0) = ubuff.phys_addr;
		GET_TX_DESC_SIZE(qInx) = ubuff.mmap_size;
		GET_TX_DESC_PTR(qInx, 0) = mmap(NULL, ubuff.mmap_size,
						PROT_READ | PROT_WRITE,
						MAP_SHARED, pdata->dev,
						ubuff.phys_addr);
		if (GET_TX_DESC_PTR(qInx, 0) == MAP_FAILED) {
			printf("failed to map tx desc\n");
			ret = ENOMEM;
			goto tx_failed;
		}

		/* allocate wrapper tx buffers */
		GET_TX_BUF_PTR(qInx, 0) =
			calloc(TX_DESC_CNT, sizeof(struct DWC_ETH_QOS_tx_buffer));
		if (GET_TX_BUF_PTR(qInx, 0) == NULL) {
			printf("failed to allocate tx wrapper buffer\n");
			ret = ENOMEM;
			goto tx_failed;
		}
	}

	DBGPR("<--%s\n", __func__);

	return 0;

tx_failed:
	for (qInx = 0; qInx < pdata->tx_avb_q_cnt; qInx++)
		DWC_ETH_QOS_free_tx_buff_and_desc_single_q(pdata, qInx);

	return ret;
}


/* This api issue command to get rx descriptor memory and rx data buffer
 * from kernel space, also allocates wrapper buffer structures.
 *
 * return zero on success else +ve error number on failure
 * */
static int DWC_ETH_QOS_alloc_rx_buff_and_desc(struct DWC_ETH_QOS_prv_data *pdata)
{
	struct DWC_ETH_QOS_user_buff *data_buff = NULL;
	struct DWC_ETH_QOS_buf_cmd ubuff;
	int ret = 0;
	int qInx;

	DBGPR("-->%s\n", __func__);

	for (qInx = 0; qInx < pdata->rx_avb_q_cnt; qInx++) {
		/* allocate rx descriptor */
		ubuff.qInx = qInx + pdata->rx_avb_q_idx;
		ret = ioctl(pdata->dev, DWC_ETH_QOS_MAP_RX_DESC, &ubuff);
		if (ret < 0) {
			printf("rx desc map ioctl failed\n");
			ret = EBUSY;
			goto rx_failed;
		}

		GET_RX_DESC_DMA_ADDR(qInx, 0) = ubuff.phys_addr;
		GET_RX_DESC_SIZE(qInx) = ubuff.mmap_size;
		GET_RX_DESC_PTR(qInx, 0) = mmap(NULL, ubuff.mmap_size,
						PROT_READ | PROT_WRITE,
						MAP_SHARED, pdata->dev,
						ubuff.phys_addr);
		if (GET_RX_DESC_PTR(qInx, 0) == MAP_FAILED) {
			printf("failed to map rx desc\n");
			ret = ENOMEM;
			goto rx_failed;
		}

		/* allocate wrapper rx buffers */
		GET_RX_BUF_PTR(qInx, 0) =
			calloc(RX_DESC_CNT, sizeof(struct DWC_ETH_QOS_rx_buffer));
		if (GET_RX_BUF_PTR(qInx, 0) == NULL) {
			printf("failed to allocate rx wrapper buffer\n");
			ret = ENOMEM;
			goto rx_failed;
		}

		/* allocate actual data buffers */
		data_buff = GET_RX_DATA_BUF_PTR(qInx, 0);
		data_buff->alloc_size = (RX_DESC_CNT * RX_BUF_SIZE);
		ret = DWC_ETH_QOS_get_buffer(pdata->pdev, data_buff);
		if (ret) {
			printf("failed to allocate rx data buffer\n");
			ret = ENOMEM;
			goto rx_failed;
		}
	}

	DBGPR("<--%s\n", __func__);

	return 0;

rx_failed:
	for (qInx = 0; qInx < pdata->rx_avb_q_cnt; qInx++)
		DWC_ETH_QOS_free_rx_buff_and_desc_single_q(pdata, qInx);

	return ret;
}


/* This api issue command to get descriptor memory from kernel space
 * and also allocates wrapper buffer structures.
 *
 * return zero on success else +ve error number on failure
 * */
static int DWC_ETH_QOS_alloc_buff_and_desc(struct DWC_ETH_QOS_prv_data *pdata)
{
	struct DWC_ETH_QOS_buf_cmd ubuff;
	int ret = 0;
	int qInx;

	DBGPR("-->%s\n", __func__);

	/* allocate tx descriptor and wrapper buffer */
	for (qInx = 0; qInx < pdata->tx_avb_q_cnt; qInx++) {
		ubuff.qInx = qInx + pdata->tx_avb_q_idx;
		ret = ioctl(pdata->dev, DWC_ETH_QOS_MAP_TX_DESC, &ubuff);
		if (ret < 0) {
			printf("tx desc map ioctl failed\n");
			ret = EBUSY;
			goto tx_failed;
		}

		GET_TX_DESC_DMA_ADDR(qInx, 0) = ubuff.phys_addr;
		GET_TX_DESC_SIZE(qInx) = ubuff.mmap_size;
		GET_TX_DESC_PTR(qInx, 0) = mmap(NULL, ubuff.mmap_size,
						PROT_READ | PROT_WRITE,
						MAP_SHARED, pdata->dev,
						ubuff.phys_addr);
		if (GET_TX_DESC_PTR(qInx, 0) == MAP_FAILED) {
			printf("failed to map tx desc\n");
			ret = ENOMEM;
			goto tx_failed;
		}

		/* allocate wrapper tx buffers */
		GET_TX_BUF_PTR(qInx, 0) =
			calloc(TX_DESC_CNT, sizeof(struct DWC_ETH_QOS_tx_buffer));
		if (GET_TX_BUF_PTR(qInx, 0) == NULL) {
			printf("failed to allocate tx wrapper buffer\n");
			ret = ENOMEM;
			goto tx_failed;
		}
	}

	/* allocate rx descriptor and wrapper buffer */
	for (qInx = 0; qInx < pdata->rx_avb_q_cnt; qInx++) {
		ubuff.qInx = qInx + pdata->rx_avb_q_idx;
		ret = ioctl(pdata->dev, DWC_ETH_QOS_MAP_RX_DESC, &ubuff);
		if (ret < 0) {
			printf("rx desc map ioctl failed\n");
			ret = EBUSY;
			goto rx_failed;
		}

		GET_RX_DESC_DMA_ADDR(qInx, 0) = ubuff.phys_addr;
		GET_RX_DESC_SIZE(qInx) = ubuff.mmap_size;
		GET_RX_DESC_PTR(qInx, 0) = mmap(NULL, ubuff.mmap_size,
						PROT_READ | PROT_WRITE,
						MAP_SHARED, pdata->dev,
						ubuff.phys_addr);
		if (GET_RX_DESC_PTR(qInx, 0) == MAP_FAILED) {
			printf("failed to map tx desc\n");
			ret = ENOMEM;
			goto rx_failed;
		}

		/* allocate wrapper rx buffers */
		GET_RX_BUF_PTR(qInx, 0) =
			calloc(RX_DESC_CNT, sizeof(struct DWC_ETH_QOS_rx_buffer));
		if (GET_RX_BUF_PTR(qInx, 0) == NULL) {
			printf("failed to allocate tx wrapper buffer\n");
			ret = ENOMEM;
			goto rx_failed;
		}
	}
	DBGPR("<--%s\n", __func__);

	return 0;

rx_failed:
	for (qInx = 0; qInx < pdata->rx_avb_q_cnt; qInx++)
		DWC_ETH_QOS_free_rx_buff_and_desc_single_q(pdata, qInx);

tx_failed:
	for (qInx = 0; qInx < pdata->tx_avb_q_cnt; qInx++)
		DWC_ETH_QOS_free_tx_buff_and_desc_single_q(pdata, qInx);

	return ret;
}


static void DWC_ETH_QOS_tx_desc_mang_ds_dump(struct DWC_ETH_QOS_prv_data *pdata)
{
	struct DWC_ETH_QOS_tx_wrapper_descriptor *desc_data = NULL;
	u_int32_t *desc = NULL;
	int qInx, i;

	printf("/**** TX DESC MANAGEMENT DATA STRUCTURE DUMP ****/\n");

	for (qInx = 0; qInx < pdata->tx_avb_q_cnt; qInx++) {
		desc_data = GET_TX_WRAPPER_DESC(qInx);

		printf("DMA CHANNEL = %d\n", qInx);
		printf("\tcur_tx        = %d\n", desc_data->cur_tx);
		printf("\tdirty_tx      = %d\n", desc_data->dirty_tx);
		printf("\tfree_desc_cnt = %d\n", desc_data->free_desc_cnt);
		printf("\ttx_pkt_queued = %d\n", desc_data->tx_pkt_queued);
		printf("\tqueue_stopped = %d\n", desc_data->queue_stopped);
		printf("\tbytes         = %ld\n", desc_data->bytes);
		printf("\tpackets       = %ld\n", desc_data->packets);

		printf("\t[<desc_add> <index>] = <TDES0> : <TDES1> : <TDES2> : <TDES3>\n");
		for (i = 0; i < TX_DESC_CNT; i++) {
			desc = GET_TX_DESC_PTR(qInx, i);
			printf("\t[%4p %d] = %#x : %#x : %#x : %#x\n",
				desc, i, desc[0], desc[1], desc[2], desc[3]);
		}
	}

	printf("/**** TX DESC MANAGEMENT DATA STRUCTURE DUMP ****/\n");
}


static void DWC_ETH_QOS_wrapper_tx_descriptor_init_single_q(
			struct DWC_ETH_QOS_prv_data *pdata,
			u_int8_t qInx)
{
	struct DWC_ETH_QOS_tx_wrapper_descriptor *desc_data =
		GET_TX_WRAPPER_DESC(qInx);
	struct DWC_ETH_QOS_tx_buffer *buffer = GET_TX_BUF_PTR(qInx, 0);
	union DWC_ETH_QOS_tx_desc *desc = GET_TX_DESC_PTR(qInx, 0);
	u_int64_t desc_dma = GET_TX_DESC_DMA_ADDR(qInx, 0);
	int i;

	DBGPR("-->%s\n", __func__);

	for (i = 0; i < TX_DESC_CNT; i++) {
		GET_TX_DESC_PTR(qInx, i) = &desc[i];
		GET_TX_DESC_DMA_ADDR(qInx, i) =
			(desc_dma + sizeof(union DWC_ETH_QOS_tx_desc) * i);
		GET_TX_BUF_PTR(qInx, i) = &buffer[i];

		/* reset wrapper buffer */
		GET_TX_BUF_PTR(qInx, i)->next_eop = -1;
		GET_TX_BUF_PTR(qInx, i)->packet = NULL;

		/* reset descriptor ring and give ownership to driver */
		memset(&desc[i], 0, sizeof(desc));
	}
	desc_data->cur_tx = 0;
	desc_data->dirty_tx = 0;
	desc_data->free_desc_cnt = TX_DESC_CNT;
	desc_data->tx_pkt_queued = 0;
	desc_data->bytes = 0;
	desc_data->packets = 0;

	/* update the total number of Tx descriptors count */
	DMA_TDRLR_RgWr((qInx + pdata->tx_avb_q_idx), (TX_DESC_CNT - 1));
	/* update the starting address of desc chain/ring */
	DMA_TDLAR_RgWr((qInx + pdata->tx_avb_q_idx),
		GET_TX_DESC_DMA_ADDR(qInx, 0));

	DBGPR("<--%s\n", __func__);
}


static void DWC_ETH_QOS_wrapper_tx_descriptor_init(
			struct DWC_ETH_QOS_prv_data *pdata)
{
	u_int8_t qInx;

	for (qInx = 0; qInx < pdata->tx_avb_q_cnt; qInx++)
		DWC_ETH_QOS_wrapper_tx_descriptor_init_single_q(pdata, qInx);

#ifdef YDEBUG
	DWC_ETH_QOS_tx_desc_mang_ds_dump(pdata);
#endif

}


static void DWC_ETH_QOS_rx_desc_mang_ds_dump(struct DWC_ETH_QOS_prv_data *pdata)
{
	struct DWC_ETH_QOS_rx_wrapper_descriptor *desc_data = NULL;
	struct DWC_ETH_QOS_rx_buffer *buff = NULL;
	u_int32_t *desc = NULL;
	int qInx, i;

	printf("/**** RX DESC MANAGEMENT DATA STRUCTURE DUMP ****/\n");

	for (qInx = 0; qInx < pdata->rx_avb_q_cnt; qInx++) {
		desc_data = GET_RX_WRAPPER_DESC(qInx);

		printf("\n");
		printf("DMA CHANNEL = %d\n", qInx);
		printf("\tcur_rx                 = %d\n", desc_data->cur_rx);
		printf("\tdirty_rx               = %d\n", desc_data->dirty_rx);
		printf("\tskb_realloc_idx        = %d\n",
			desc_data->skb_realloc_idx);
		printf("\tbytes                  = %ld\n", desc_data->bytes);
		printf("\tpackets                = %ld\n", desc_data->packets);

		printf("\t[<desc_add> <index>] = <RDES0> : <RDES1> : <RDES2> : <RDES3>\n"
		       "\t                     = <pkt.map.paddr> : <pkt.map.mmap_size> : <pkt.offset> : <pkt.vaddr> : <pkt.len>\n");
		for (i = 0; i < RX_DESC_CNT; i++) {
			desc = GET_RX_DESC_PTR(qInx, i);
			buff = GET_RX_BUF_PTR(qInx, i);

			printf("\t[%4p %d] = %#x : %#x : %#x : %#x\n",
				desc, i, desc[0], desc[1], desc[2], desc[3]);
			printf("\t         = %#llx : %d : %d : %#x : %d\n",
				buff->packet.map.paddr,
				buff->packet.map.mmap_size, buff->packet.offset,
				(u_int32_t *)buff->packet.vaddr,
				buff->packet.len);
		}
	}

	printf("/**** RX DESC MANAGEMENT DATA STRUCTURE DUMP ****/\n");
}


static void DWC_ETH_QOS_wrapper_rx_descriptor_init_single_q(
			struct DWC_ETH_QOS_prv_data *pdata,
			u_int8_t qInx)
{
	struct DWC_ETH_QOS_rx_wrapper_descriptor *desc_data =
		GET_RX_WRAPPER_DESC(qInx);
	struct DWC_ETH_QOS_rx_buffer *buffer = GET_RX_BUF_PTR(qInx, 0);
	struct DWC_ETH_QOS_rx_buffer *tmp_buffer = NULL;
	union DWC_ETH_QOS_rx_desc *desc = GET_RX_DESC_PTR(qInx, 0);
	union DWC_ETH_QOS_rx_desc *tmp_desc = NULL;
	u_int64_t desc_dma = GET_RX_DESC_DMA_ADDR(qInx, 0);
	struct DWC_ETH_QOS_user_buff *data_buff = GET_RX_DATA_BUF_PTR(qInx, 0);
	int i;

	DBGPR("-->%s\n", __func__);

	for (i = 0; i < RX_DESC_CNT; i++) {
		GET_RX_DESC_PTR(qInx, i) = &desc[i];
		GET_RX_DESC_DMA_ADDR(qInx, i) =
			(desc_dma + sizeof(union DWC_ETH_QOS_rx_desc) * i);
		GET_RX_BUF_PTR(qInx, i) = &buffer[i];

		/* init wrapper buffer */
		tmp_buffer = GET_RX_BUF_PTR(qInx, i);
		tmp_buffer->packet.map.paddr = data_buff->dma_addr;
		tmp_buffer->packet.map.mmap_size = data_buff->mmap_size;
		tmp_buffer->packet.offset = (i * RX_BUF_SIZE);
		tmp_buffer->packet.vaddr = data_buff->addr +
					tmp_buffer->packet.offset;
		tmp_buffer->packet.len = 0;

		/* init descriptor ring and give ownership to driver */
		tmp_desc = GET_RX_DESC_PTR(qInx, i);
		memset(tmp_desc, 0, sizeof(tmp_desc));
		/* update buffer 1 address */
		NRB_RXD_RDES0(tmp_desc) = tmp_buffer->packet.map.paddr +
				tmp_buffer->packet.offset;
		/* set OWN, INTE and BUF1V bit */
		NRB_RXD_RDES3_OWN(tmp_desc) = 1;
		NRB_RXD_RDES3_INTE(tmp_desc) = 1;
		NRB_RXD_RDES3_BUF1V(tmp_desc) = 1;
	}
	desc_data->cur_rx = 0;
	desc_data->dirty_rx = 0;
	desc_data->skb_realloc_idx = 0;
	desc_data->bytes = 0;
	desc_data->packets = 0;
	desc_data->pkt_present = 0;

	/* update the total number of Rx descriptors count */
	DMA_RDRLR_RgWr((qInx + pdata->rx_avb_q_idx), (RX_DESC_CNT - 1));
	/* update the Rx Descriptor Tail Pointer */
	DMA_RDTP_RPDR_RgWr((qInx + pdata->rx_avb_q_idx),
		GET_RX_DESC_DMA_ADDR(qInx, (RX_DESC_CNT - 1)));
	/* update the starting address of desc chain/ring */
	DMA_RDLAR_RgWr((qInx + pdata->rx_avb_q_idx),
		GET_RX_DESC_DMA_ADDR(qInx, 0));

	DBGPR("<--%s\n", __func__);
}


static void DWC_ETH_QOS_wrapper_rx_descriptor_init(
			struct DWC_ETH_QOS_prv_data *pdata)
{
	u_int8_t qInx;

	for (qInx = 0; qInx < pdata->rx_avb_q_cnt; qInx++)
		DWC_ETH_QOS_wrapper_rx_descriptor_init_single_q(pdata, qInx);

#ifdef YDEBUG
	DWC_ETH_QOS_rx_desc_mang_ds_dump(pdata);
#endif

}


/*
 * This api will initialize MAC, MTL tx & rx queue and
 * DMA tx & rx channel which are not initialized in kmod driver.
 */
static int DWC_ETH_QOS_hw_init(struct DWC_ETH_QOS_prv_data *pdata)
{
	u_int8_t qInx;

	DBGPR("-->%s\n", __func__);

	/* MTL Rx queue to DMA Rx channel mapping is done
	 * as part of Kmod driver
	 * */
	for (qInx = pdata->tx_avb_q_idx; qInx < pdata->tx_q_cnt; qInx++)
		configure_mtl_queue(pdata, qInx);

	for (qInx = pdata->tx_avb_q_idx; qInx < pdata->tx_q_cnt; qInx++)
		configure_mac(pdata, qInx);

	for (qInx = pdata->tx_avb_q_idx; qInx < pdata->tx_q_cnt; qInx++)
		configure_dma_channel(pdata, qInx);

	DBGPR("<--%s\n", __func__);
}


/*
 * This api does following,
 * a. allocates private data structure
 * b. get the device CSR address space from kernel driver
 * c. allocates descriptors queue structures
 *
 * Return zero on success and +ve error number on failure
 * */
int DWC_ETH_QOS_attach(char *dev_path, struct device *pdev)
{
	struct DWC_ETH_QOS_prv_data *pdata = NULL;
	struct DWC_ETH_QOS_bind_cmd bind;
	int ret = 0;

	DBGPR("-->%s\n", __func__);

	if (pdev == NULL)
		return EINVAL;

	if (pdev->private_data != NULL)
		return EBUSY;

	/* allocate private data structure */
	pdata = calloc(1, sizeof(struct DWC_ETH_QOS_prv_data));
	if (pdata == NULL) {
		printf("Unable to allocate private data structure\n");
		ret = ENXIO;
		goto alloc_failed;
	}

	pdev->private_data = pdata;
	pdata->pdev = pdev;

	pdata->dev = open("/dev/dwc_eth_qos_avb", O_RDWR);
	if (pdata->dev < 0) {
		printf("Unable to open \"/dev/dwc_eth_qos_avb\"");
		ret = ENXIO;
		goto open_failed;
	}

	strncpy(bind.pdev_name, dev_path, DWC_ETH_QOS_BIND_NAME_SIZE - 1);
	ret = ioctl(pdata->dev, DWC_ETH_QOS_BIND, &bind);
	if (ret < 0) {
		printf("bind ioctl failed\n");
		ret = ENXIO;
		goto ioctl_failed;
	}
	pdata->csr_space.paddr = 0;
	pdata->csr_space.mmap_size = bind.mmap_size;

	/* TODO: get hardware and mac info and keep in pdata */


	/* get base address for accessing CSR */
	pdata->base_addr = (u_int8_t *)mmap(NULL, pdata->csr_space.mmap_size,
					PROT_READ | PROT_WRITE,
					MAP_SHARED, pdata->dev, 0);
	if (pdata->base_addr == MAP_FAILED) {
		printf("Failed to get base address\n");
		goto mmap_failed;
		ret = ENXIO;
	}
	pci_base_addr = (unsigned long)pdata->base_addr;
	printf("pci_base_addr = %#lx\n", pci_base_addr);

	/* set the frame limits assuming std ethernet sized frames */
	pdata->max_frame_size = DWC_ETH_QOS_MAX_FRAME_SIZE;
	pdata->min_frame_size = DWC_ETH_QOS_MIN_FRAME_SIZE;

	/*
	 * The last two queue/channel are used for AVB traffic
	 * hence the kmod driver will not use these queues for
	 * data transfer. This user mode driver will do AVB data
	 * tranfer using those queues.
	 * */
	pdata->tx_q_cnt = get_tx_queue_count();
	pdata->rx_q_cnt = get_rx_queue_count();
	if (pdata->tx_q_cnt < (DWC_ETH_QOS_MAX_AVB_Q_CNT + 1)) {
		printf("HW doesn't support sufficient tx queue/channel "
			"(present tx_q_cnt = %d) for AVB\n", pdata->tx_q_cnt);
		ret = ENXIO;
		goto get_hw_q_cnt_failed;
	}
	pdata->tx_avb_q_cnt = DWC_ETH_QOS_MAX_AVB_Q_CNT;
	pdata->tx_avb_q_idx = (pdata->tx_q_cnt -
				DWC_ETH_QOS_MAX_AVB_Q_CNT);

	if (pdata->rx_q_cnt < DWC_ETH_QOS_MAX_AVB_Q_CNT + 1) {
		printf("HW doesn't support sufficient rx queue/channel "
			"(present rx_q_cnt = %d) for AVB\n", pdata->rx_q_cnt);
		ret = ENXIO;
		goto get_hw_q_cnt_failed;
	}
	pdata->rx_avb_q_cnt = DWC_ETH_QOS_MAX_AVB_Q_CNT;
	pdata->rx_avb_q_idx = (pdata->rx_q_cnt -
				DWC_ETH_QOS_MAX_AVB_Q_CNT);

	printf("tx_avb_q_cnt = %d, tx_avb_q_idx = %d, "
	    	"rx_avb_q_cnt = %d, rx_avb_q_idx = %d\n",
		pdata->tx_avb_q_cnt, pdata->tx_avb_q_idx,
		pdata->rx_avb_q_cnt, pdata->rx_avb_q_idx);

	/* allocate and setup queues */
	ret = DWC_ETH_QOS_alloc_tx_queue_struct(pdata);
	if (ret) {
		printf("Failed to allocate tx queue structure\n");
		ret = ENXIO;
		goto alloc_tx_queue_failed;
	}

	ret = DWC_ETH_QOS_alloc_rx_queue_struct(pdata);
	if (ret) {
		printf("Failed to allocate rx queue structure\n");
		ret = ENXIO;
		goto alloc_rx_queue_failed;
	}

	ret = DWC_ETH_QOS_alloc_tx_buff_and_desc(pdata);
	if (ret) {
		printf("Failed to allocate tx buffer and desc structure\n");
		ret = ENXIO;
		goto alloc_tx_desc_failed;
	}

	ret = DWC_ETH_QOS_alloc_rx_buff_and_desc(pdata);
	if (ret) {
		printf("Failed to allocate rx buffer and desc structure\n");
		ret = ENXIO;
		goto alloc_rx_desc_failed;
	}

	DBGPR("<--%s\n", __func__);

	return ret;

alloc_rx_desc_failed:
	DWC_ETH_QOS_free_tx_buff_and_desc(pdata);

alloc_tx_desc_failed:
	DWC_ETH_QOS_free_rx_queue(pdata);

alloc_rx_queue_failed:
	DWC_ETH_QOS_free_tx_queue(pdata);

alloc_tx_queue_failed:
	munmap(pdata->base_addr, pdata->csr_space.mmap_size);

get_hw_q_cnt_failed:
	/* nothing todo */

mmap_failed:
	/* nothing todo */

ioctl_failed:
	close(pdata->dev);

open_failed:
	free(pdata);
	pdev->private_data = NULL;

alloc_failed:
	return ret;
}

#ifdef DEBUG_DUMP
static void DWC_ETH_QOS_get_statistics(struct DWC_ETH_QOS_prv_data *pdata)
{
	struct DWC_ETH_QOS_tx_wrapper_descriptor *tx_desc_data = NULL;
	struct DWC_ETH_QOS_rx_wrapper_descriptor *rx_desc_data = NULL;
	int qInx;

	DBGPR("-->%s\n", __func__);

	printf("\n/************** USER MODE DRIVER ******************\n");

	for (qInx = 0; qInx < pdata->tx_avb_q_cnt; qInx++) {
		tx_desc_data = GET_TX_WRAPPER_DESC(qInx);
		printf("Queue[%d(%d)] => total pkts transmitted: %ld total bytes transmitted: %ld\n",
			qInx, (qInx + pdata->tx_avb_q_idx),
			tx_desc_data->packets, tx_desc_data->bytes);
	}

	for (qInx = 0; qInx < pdata->rx_avb_q_cnt; qInx++) {
		rx_desc_data = GET_RX_WRAPPER_DESC(qInx);
		printf("Queue[%d(%d)] => total pkts received: %ld total bytes received: %ld\n",
			qInx, (qInx  + pdata->tx_avb_q_idx),
			rx_desc_data->packets, rx_desc_data->bytes);
	}

	printf("\n/**************************************************\n");

	DBGPR("<--%s\n", __func__);
}
#endif


/*
 * This API will stop the DMA Tx and Rx process
 */
static int DWC_ETH_QOS_hw_exit(struct DWC_ETH_QOS_prv_data *pdata)
{
	u_int32_t ret;
	u_int8_t qInx;

	DBGPR("-->%s\n", __func__);

	for (qInx = pdata->tx_avb_q_idx; qInx < pdata->tx_q_cnt; qInx++) {
		ret = stop_dma_tx(qInx);
		if (ret)
			printf("failed to stop Tx DMA channel %d\n", qInx);

		ret = stop_dma_rx(qInx);
		if (ret)
			printf("failed to stop Rx DMA channel %d\n", qInx);

		reset_mtl_queue(qInx);
		reset_mac(qInx);
		reset_dma_channel(qInx);
	}


	DBGPR("<--%s\n", __func__);
}


/* This api reverse the operation performed at attach() time.
 *
 * return 0 on success and -ve number on failure
 * */
int DWC_ETH_QOS_detach(struct device *pdev)
{
	struct DWC_ETH_QOS_prv_data *pdata = NULL;
	struct DWC_ETH_QOS_tx_wrapper_descriptor *desc_data = NULL;

	DBGPR("-->%s\n", __func__);

	if (pdev == NULL)
		return EINVAL;

	pdata = (struct DWC_ETH_QOS_prv_data *)pdev->private_data;
	if (pdata == NULL) {
		printf("failed to get valid pdata\n");
		return ENXIO;
	}

	DWC_ETH_QOS_free_tx_resourses(pdata);
	DWC_ETH_QOS_free_rx_resourses(pdata);

	munmap(pdata->base_addr, pdata->csr_space.mmap_size);

	close(pdata->dev);
	free(pdata);
	pdev->private_data = NULL;

	DBGPR("<--%s\n", __func__);

	return 0;
}


int DWC_ETH_QOS_suspend(struct device *pdev)
{
	//TODO ??
}


int DWC_ETH_QOS_resume(struct device *pdev)
{
	//TODO ??
}


/* This api will initialize the device and its descriptors structure
 * and make it ready for data transfer.
 *
 * returns zero on success else +ve error number on failure.
 * */
int DWC_ETH_QOS_init(struct device *pdev)
{
	struct DWC_ETH_QOS_prv_data *pdata = NULL;

	if (pdev == NULL)
		return EINVAL;

	pdata = (struct DWC_ETH_QOS_prv_data *)pdev->private_data;
	if (pdata == NULL) {
		printf("failed to get valid pdata\n");
		return ENXIO;
	}

	DBGPR("-->%s\n", __func__);

	DWC_ETH_QOS_wrapper_tx_descriptor_init(pdata);
	DWC_ETH_QOS_wrapper_rx_descriptor_init(pdata);

	DWC_ETH_QOS_hw_init(pdata);

	DBGPR("<--%s\n", __func__);

	return 0;
}

/* This api will reset device and fress its descriptors structure.
 *
 * returns zero on success else +ve error number on failure.
 * */
int DWC_ETH_QOS_exit(struct device *pdev)
{
	struct DWC_ETH_QOS_prv_data *pdata = NULL;

	if (pdev == NULL)
		return EINVAL;

	pdata = (struct DWC_ETH_QOS_prv_data *)pdev->private_data;
	if (pdata == NULL) {
		printf("failed to get valid pdata\n");
		return ENXIO;
	}

	DBGPR("-->%s\n", __func__);

	DWC_ETH_QOS_hw_exit(pdata);

#ifdef DEBUG_DUMP
	DWC_ETH_QOS_get_statistics(pdata);
#endif

	DBGPR("<--%s\n", __func__);

	return 0;
}


/* returns zero on success and +ve error number on failure */
int DWC_ETH_QOS_test_reg_read(struct device *pdev)
{
	struct DWC_ETH_QOS_prv_data *pdata = NULL;
	u_int32_t version = 0;

	if (pdev == NULL)
		return EINVAL;

	pdata = (struct DWC_ETH_QOS_prv_data *)pdev->private_data;
	if (pdata == NULL) {
		printf("failed to get valid pdata\n");
		return ENXIO;
	}

	MAC_VR_RgRd(version);
	if (version == 0x1040)
		return 0;
	else
		return  ENXIO;
}


/* returns zero on success and +ve error number on failure */
int DWC_ETH_QOS_test_reg_write(struct device *pdev)
{
	struct DWC_ETH_QOS_prv_data *pdata = NULL;
	u_int32_t tmp = 0;

	if (pdev == NULL)
		return EINVAL;

	pdata = (struct DWC_ETH_QOS_prv_data *)pdev->private_data;
	if (pdata == NULL) {
		printf("failed to get valid pdata\n");
		return ENXIO;
	}

	DMA_TDLAR_RgWr(pdata->tx_avb_q_idx, 0xffffffff);
	DMA_TDLAR_RgRd(pdata->tx_avb_q_idx, tmp);
	if (tmp == 0xfffffffc)
		return  0;
	else
		return ENXIO;
}


/* This api issue command to get data buffer from kernel space
 * and map it to user space and give all dma address, virtual
 * address and length of data buffer allocated to user.
 *
 * return zero on success and +ve error number on failure
 * */
int DWC_ETH_QOS_get_buffer(struct device *pdev,
			struct DWC_ETH_QOS_user_buff *ubuff)
{
	struct DWC_ETH_QOS_prv_data *pdata = NULL;
	struct DWC_ETH_QOS_buf_cmd req;
	int ret = 0;

	DBGPR("-->%s\n", __func__);

	if (ubuff == NULL || pdev == NULL)
		return EINVAL;

	pdata = (struct DWC_ETH_QOS_prv_data *)pdev->private_data;
	if (pdata == NULL) {
		printf("failed to get valid pdata\n");
		return ENXIO;
	}

	req.alloc_size = ubuff->alloc_size;
	ret = ioctl(pdata->dev, DWC_ETH_QOS_MAP_BUF, &req);
	if (ret < 0) {
		printf("map buf ioctl failed\n");
		return ENOMEM;
	}

	ubuff->dma_addr = req.phys_addr;
	ubuff->mmap_size = req.mmap_size;
	ubuff->addr = mmap(NULL, req.mmap_size,
			PROT_READ | PROT_WRITE, MAP_SHARED,
			pdata->dev, req.phys_addr);
	if (ubuff->addr == MAP_FAILED) {
		printf("Failed to get buffer\n");
		return ENOMEM;
	}

	DBGPR("<--%s\n", __func__);

	return 0;
}


/* This api issue command to kernel driver to release the
 * data buffer.
 *
 * return zero on success and +ve error number on failure
 * */
int DWC_ETH_QOS_free_buffer(struct device *pdev,
			struct DWC_ETH_QOS_user_buff *ubuff)
{
	struct DWC_ETH_QOS_prv_data *pdata = NULL;
	struct DWC_ETH_QOS_buf_cmd req;
	int ret = 0;

	DBGPR("-->%s\n", __func__);

	if (ubuff == NULL || pdev == NULL)
		return EINVAL;

	pdata = (struct DWC_ETH_QOS_prv_data *)pdev->private_data;
	if (pdata == NULL) {
		printf("failed to get valid pdata\n");
		return ENXIO;
	}

	munmap(ubuff->addr, ubuff->mmap_size);
	req.phys_addr = ubuff->dma_addr;

	ret = ioctl(pdata->dev, DWC_ETH_QOS_UNMAP_BUF, &req);
	if (ret < 0) {
		printf("unmap buf ioctl failed\n");
		return ENOMEM;
	}

	ubuff->dma_addr = 0;
	ubuff->addr = NULL;
	ubuff->mmap_size = 0;

	DBGPR("<--%s\n", __func__);

	return ret;
}


/* This api prepare context descriptor of given queue for
 * data transfer.
 *
 * returns number of context descriptor used
 * */
static unsigned int DWC_ETH_QOS_setup_context_desc(
			struct DWC_ETH_QOS_prv_data *pdata,
			struct DWC_ETH_QOS_packet *packet,
			unsigned int qInx)
{
	struct DWC_ETH_QOS_tx_wrapper_descriptor *desc_data =
		GET_TX_WRAPPER_DESC(qInx);
	union DWC_ETH_QOS_tx_desc *desc =
		GET_TX_DESC_PTR(qInx, desc_data->cur_tx);
	struct DWC_ETH_QOS_tx_buffer *buffer =
		GET_TX_BUF_PTR(qInx, desc_data->cur_tx);
	unsigned int count = 0;

	DBGPR("-->%s\n", __func__);

	/* TODO: setup context descriptor */
	//start...


	buffer->packet = NULL;
	buffer->next_eop = -1;
	//...end

	count++;
	INCR_TX_DESC_INDEX(desc_data->cur_tx, 1);

	DBGPR("<--%s\n", __func__);

	return count;
}


static void DWC_ETH_QOS_display_desc(u_int32_t *desc, int tx_rx)
{
	if (tx_rx) {
		printf("tdes0 = %#x, tdes1 = %#x, tdes2 = %#x, tdes3 = %#x\n",
			desc[0], desc[1], desc[2], desc[3]);
	} else {
		printf("rdes0 = %#x, rdes1 = %#x, rdes2 = %#x, rdes3 = %#x\n",
			desc[0], desc[1], desc[2], desc[3]);
	}
}


/* This api prepare normal descriptor of given queue for
 * data transfer.
 *
 * returns number of normal descriptor used
 * */
static unsigned int DWC_ETH_QOS_setup_data_descriptor(
			struct DWC_ETH_QOS_prv_data *pdata,
			struct DWC_ETH_QOS_packet *packet,
			unsigned int qInx,
			unsigned int first)
{
	struct DWC_ETH_QOS_tx_wrapper_descriptor *desc_data =
		GET_TX_WRAPPER_DESC(qInx);
	union DWC_ETH_QOS_tx_desc *desc =
		GET_TX_DESC_PTR(qInx, desc_data->cur_tx);
	struct DWC_ETH_QOS_tx_buffer *buffer =
		GET_TX_BUF_PTR(qInx, desc_data->cur_tx);
	unsigned int count = 0;

	DBGPR("-->%s: qInx = %d, first = %d\n", __func__, qInx, first);

	/* update buffer address, length and other control details */
	NRB_TXD_TDES0(desc) = packet->map.paddr + packet->offset;
	NRB_TXD_TDES2_B1L(desc) = packet->len;
#ifdef TS_CAPTURE
	NRB_TXD_TDES2_TTSE(desc) = 1; /* enable tims stamping */
#endif /* end of TS_CAPTURE */
	NRB_TXD_TDES3_FL(desc) = packet->len; //TODO:write all in one access
	NRB_TXD_TDES3_FD(desc) = 1;
	NRB_TXD_TDES3_LD(desc) = 1;
	NRB_TXD_TDES3_OWN(desc) = 1;

#ifdef YDEBUG
	DWC_ETH_QOS_display_desc((u_int32_t *)desc, 1);
#endif

	buffer->packet = packet;
	buffer->next_eop = -1;

	/* keep track the "index of last descriptor which will
	 * be written back by device" in first wrapper buffer,
	 * which will be used later for descriptor cleanup.
	 * */
	buffer = GET_TX_BUF_PTR(qInx, first);
	buffer->next_eop = desc_data->cur_tx;

	count++;
	INCR_TX_DESC_INDEX(desc_data->cur_tx, 1);

	DBGPR("<--%s: count = %d, cur_tx = %d\n", __func__,
		count, desc_data->cur_tx);

	return count;
}


struct timespec start , end;
long diff_nseconds, diff_seconds;


/* This api initiates the transmission of packet on a give queue.
 *
 * return zero on success and +ve error number on failure.
 * */
int DWC_ETH_QOS_start_xmit(struct device *pdev,
			unsigned int qInx,
			struct DWC_ETH_QOS_packet *packet)
{
	struct DWC_ETH_QOS_prv_data *pdata = NULL;
	struct DWC_ETH_QOS_tx_wrapper_descriptor *desc_data = NULL;
	unsigned int count = 0;
	unsigned int first = 0;
	unsigned int i;
#ifdef TS_CAPTURE
        unsigned long long ns;
#endif

	DBGPR("-->%s: qInx = %d, packet->len = %d\n", __func__,
		qInx, packet->len);

	if (pdev == NULL)
		return EINVAL;

	pdata = (struct DWC_ETH_QOS_prv_data *)pdev->private_data;
	if (pdata == NULL) {
		printf("failed to get valid pdata\n");
		return ENXIO;
	}

	if (qInx > pdata->tx_avb_q_cnt)
		return EINVAL;

	if (packet == NULL)
		return EINVAL;

	packet->next = NULL; /* used for cleanup */

	desc_data = GET_TX_WRAPPER_DESC(qInx);

	/* check no of available free desc ?
	 * Min of two desc required (one for context and one for data
	 * assuming data fit in one desc)
	 * */
	if (desc_data->free_desc_cnt < 2) {
		//printf("No free descriptor available, aborting transfer...\n");
		return ENOSPC;
	}

	first = desc_data->cur_tx;

	DBGPR("cur_tx = %d\n", desc_data->cur_tx);

	/* call this function only if pkt to be transmitted
	 * is VLAN or PTP pkt with one step timestamp enabled
	 * */
	//if (0) //TODO:
	//	count = DWC_ETH_QOS_setup_context_desc(pdata,
	//				packet, qInx);

	/* setup data descriptor */
	count += DWC_ETH_QOS_setup_data_descriptor(pdata, packet,
						qInx, first);

	desc_data->free_desc_cnt -= count;
	desc_data->tx_pkt_queued += count;

	DBGPR("<--%s: free_desc_cnt = %d, tx_pkt_queued = %d\n",
		__func__, desc_data->free_desc_cnt, desc_data->tx_pkt_queued);

#ifdef TS_CAPTURE
        packet->pkt_no2 = pkt_no;
        ns = get_wallclock();
        packet->submit_time = ns;
#endif
	/* trigger DMA by writting index of next immediate
	 * free descriptor address into TAIL reg
	 * */
	DMA_TDTP_TPDR_RgWr((qInx + pdata->tx_avb_q_idx),
		GET_TX_DESC_DMA_ADDR(qInx, desc_data->cur_tx));

#ifdef TS_CAPTURE
        pkt_no++;
        if (pkt_no == 320)
                pkt_no = 0;
#endif

	return 0;
}

#ifdef TS_CAPTURE
static void DWC_ETH_QOS_get_tx_hwtstamp(union DWC_ETH_QOS_tx_desc *desc,
                struct DWC_ETH_QOS_packet *packet)
{
        unsigned long long ts;
        u_int32_t ts_low = 0;
        u_int32_t ts_high = 0;

        if (NWB_TXD_TDES3_TTSS(desc) == 1) {
                ts_low = NWB_TXD_TDES0(desc);
                ts_high = NWB_TXD_TDES1(desc);
                packet->transmitted_time = ts_low + (ts_high * 1000000000ull);
        } else {
                packet->transmitted_time = 0;
                printf("Tx time stamp is not captured for this packet\n");
        }
}
#endif

/* This api check for packet transfer complete and reclaim the
 * descriptor and data buffer pointers.
 *
 * return number of data buffers/packets cleaned on success
 *        and EINVAL on failure and
 *        -1 if no more data present in device for transmission
 * */
int DWC_ETH_QOS_tx_buffer_cleanup(struct device *pdev,
				unsigned int qInx,
				struct DWC_ETH_QOS_packet **cleaned_packet)
{
	struct DWC_ETH_QOS_prv_data *pdata = NULL;
	struct DWC_ETH_QOS_tx_wrapper_descriptor *desc_data = NULL;
	struct DWC_ETH_QOS_packet *last_reclaimed = NULL;
	struct DWC_ETH_QOS_tx_buffer *buffer = NULL;
	union DWC_ETH_QOS_tx_desc *desc = NULL, *eop_desc = NULL;
	unsigned int reclaimed_cnt = 0;
	unsigned int first = 0, last = 0, done = 0;

	DBGPR("-->%s: qInx = %d\n", __func__, qInx);

	if (pdev == NULL)
		return EINVAL;

	pdata = (struct DWC_ETH_QOS_prv_data *)pdev->private_data;
	if (pdata == NULL) {
		printf("failed to get valid pdata\n");
		return EINVAL;
	}

	if (cleaned_packet == NULL)
		return EINVAL;

	if (qInx > pdata->tx_avb_q_cnt)
		return EINVAL;

	*cleaned_packet = NULL; /* nothing is reclaimed yet */

	desc_data = GET_TX_WRAPPER_DESC(qInx);
	if (desc_data->free_desc_cnt == TX_DESC_CNT) {
		printf("Nothing is queued for transmission\n");
		return -1;
	}

	/* get descriptor index to be cleaned */
	first = desc_data->dirty_tx;

	desc = GET_TX_DESC_PTR(qInx, first);
	buffer = GET_TX_BUF_PTR(qInx, first);

	last = buffer->next_eop;
	eop_desc = GET_TX_DESC_PTR(qInx, buffer->next_eop);

	done = INCR_TX_LOCAL_INDEX(last, 1);

	DBGPR("dirty_tx = %d, done = %d\n", desc_data->dirty_tx, done);

	while (!NWB_TXD_TDES3_OWN(eop_desc) && desc_data->tx_pkt_queued) {
#ifdef YDEBUG
		DWC_ETH_QOS_display_desc((u_int32_t *)eop_desc, 1);
#endif
		/* clean all the descriptors within this range */
		while (first != done) {
			if (buffer->packet) {
				//buffer->packet->dmatime = (0xffffffff) & ;
				desc_data->bytes += buffer->packet->len;

				if (*cleaned_packet == NULL)
					*cleaned_packet = buffer->packet;
				else
					last_reclaimed->next = buffer->packet;

				last_reclaimed = buffer->packet;
			}
			/* reset buffer and descriptor */
			buffer->next_eop = -1;
#ifdef TS_CAPTURE
                        DWC_ETH_QOS_get_tx_hwtstamp(desc, buffer->packet);
#endif
			buffer->packet = NULL;
			memset(desc, 0, sizeof(desc));

			desc_data->free_desc_cnt++;
			desc_data->tx_pkt_queued--;

			INCR_TX_DESC_INDEX(first, 1);
			desc = GET_TX_DESC_PTR(qInx, first);
			buffer = GET_TX_BUF_PTR(qInx, first);
		}
		desc_data->packets++;
		reclaimed_cnt++;
		/* see if we can continue to the next packet */
		last = buffer->next_eop;
		if (last != -1) {
			eop_desc = GET_TX_DESC_PTR(qInx, last);
			done = INCR_TX_LOCAL_INDEX(last, 1);
		} else {
			break;
		}
	}
	/* next to clean */
	desc_data->dirty_tx = first;

	DBGPR("<--%s: free_desc_cnt = %d, reclaimed_cnt = %d, "
				"packets = %d, tx_pkt_queued = %d, dirty_tx = %d\n",
				__func__, desc_data->free_desc_cnt, reclaimed_cnt, desc_data->packets,
			 	desc_data->tx_pkt_queued, desc_data->dirty_tx);

	return reclaimed_cnt;
}


void dump_rx_desc(struct DWC_ETH_QOS_prv_data *pdata,
	 				u_int8_t qInx, int desc_idx)
{
	u_int32_t *desc = GET_RX_DESC_PTR(qInx, desc_idx);

	printf("\nRX_NORMAL_DESC[%d %4p %d RECEIVED FROM DEVICE]"\
		" = %#x:%#x:%#x:%#x\n",
		qInx, desc, desc_idx, desc[0], desc[1],
		desc[2], desc[3]);
}


/*
 * @qInx -
 * @quota - maximum packets to be read from device
 *
 * return valid packets on success and NULL on failure
 * */
struct DWC_ETH_QOS_packet * DWC_ETH_QOS_read(struct device *pdev,
					unsigned int qInx,
					unsigned int quota)
{
	struct DWC_ETH_QOS_prv_data *pdata = NULL;
	struct DWC_ETH_QOS_rx_wrapper_descriptor *desc_data = NULL;
	union DWC_ETH_QOS_rx_desc *desc = NULL;
	struct DWC_ETH_QOS_rx_buffer *buffer = NULL;
	struct DWC_ETH_QOS_packet *packets = NULL, *head = NULL;
	unsigned short len = 0;
	unsigned int received = 0;
	int ret = 0, tmp_qInx;

	DBGPR("-->%s: qInx = %d\n", __func__, qInx);

	if (pdev == NULL)
		return NULL;

	pdata = (struct DWC_ETH_QOS_prv_data *)pdev->private_data;
	if (pdata == NULL) {
		printf("failed to get valid pdata\n");
		return NULL;
	}

	if (qInx > pdata->rx_avb_q_cnt) {
		printf("Invalid Rx queue index(%d)\n"
			"Total number of Rx queues = %d\n",
			qInx, pdata->rx_avb_q_cnt);
		return NULL;
	}

	desc_data = GET_RX_WRAPPER_DESC(qInx);

	DBGPR("cur_rx = %d, dirty_rx = %d\n", desc_data->cur_rx,
		desc_data->dirty_rx);

	if (!desc_data->pkt_present) {
		/* enable rx interrupt */
		DMA_IER_RIE_UdfWr((qInx + pdata->rx_avb_q_idx), 0x1);

		DBGPR("going to get rx lock\n");
		/* wait for lock (down/acquire the rx lock) */
		tmp_qInx = (qInx + pdata->rx_avb_q_idx);
		ret = ioctl(pdata->dev, DWC_ETH_QOS_RX_GETLOCK, &tmp_qInx);
		if (ret < 0) {
			printf("failed to get rx lock\n");
			goto rx_getlock_failed;
		}
	} else {
		printf("********* Hurrrr!!!!! Data already present, don't wait for lock ********\n");
	}

	while (received < quota) {
		desc = GET_RX_DESC_PTR(qInx, desc_data->cur_rx);
		buffer = GET_RX_BUF_PTR(qInx, desc_data->cur_rx);

		/* no more data available */
		if (NWB_RXD_RDES3_OWN(desc))
			break;

#ifdef YDEBUG
		dump_rx_desc(pdata, qInx, desc_data->cur_rx);
#endif

		/* check for bad/oversized packets, error is
		 * valid only for last descriptor (OWN + LD bit set).
		 * */
		if (!NWB_RXD_RDES3_ES(desc) && NWB_RXD_RDES3_LD(desc)) {
			/* get packet length */
			len = NWB_RXD_RDES3_PL(desc);
			/* create a packet list for application */
			if (packets == NULL) {
				head = &(buffer->packet);
				packets = head;
				packets->next = NULL;
			} else {
				packets->next = &(buffer->packet);
				packets = &(buffer->packet);
				packets->next = NULL;
			}
			packets->vaddr = buffer->packet.vaddr; //TODO: this may not be required
			packets->len = len;
			packets->flags = 0;

			desc_data->packets++;
			desc_data->bytes += len;
			received++;
		} else {
			desc_data->error_pkts++;
			if (NWB_RXD_RDES3_ES(desc)) {
				printf("Error in received packets\n");
				if (NWB_RXD_RDES3_CE(desc))
					printf("CRC error\n");
				if (NWB_RXD_RDES3_OE(desc))
					printf("Over flow error\n");
				if (NWB_RXD_RDES3_RE(desc))
					printf("Receive error\n");
				if (NWB_RXD_RDES3_DE(desc))
					printf("Dribble bit error\n");
				if (NWB_RXD_RDES3_RWT(desc))
					printf("Receive Watchdog Timeout\n");
				if (NWB_RXD_RDES3_GP(desc))
					printf("Gaint packet\n");
			}

			if (NWB_RXD_RDES3_LD(desc))
				printf("Received oversized packets\n");
		}

		desc_data->dirty_rx++;
		INCR_RX_DESC_INDEX(desc_data->cur_rx, 1);
	}

	if (received == quota) {
		desc = GET_RX_DESC_PTR(qInx, desc_data->cur_rx);
		if (!NWB_RXD_RDES3_OWN(desc)) {
			desc_data->pkt_present = 1;
			printf("###### data is still present in device #######\n");
		} else {
			desc_data->pkt_present = 0;
		}
	}

	DBGPR("cur_rx = %d, dirty_rx = %d\n", desc_data->cur_rx,
		desc_data->dirty_rx);

	DBGPR("<--%s, received = %d\n", __func__, received);

	return head;

rx_getlock_failed:
	/* disable rx interrupt */
	DMA_IER_RIE_UdfWr((qInx + pdata->rx_avb_q_idx), 0x0);

	return NULL;
}


/* return zero on success and +ve error number on failure */
int DWC_ETH_QOS_read_done(struct device *pdev, unsigned int qInx)
{
	struct DWC_ETH_QOS_prv_data *pdata = NULL;
	struct DWC_ETH_QOS_rx_wrapper_descriptor *desc_data = NULL;
	union DWC_ETH_QOS_rx_desc *desc = NULL;
	struct DWC_ETH_QOS_rx_buffer *buffer = NULL;
	struct DWC_ETH_QOS_packet *packets = NULL, *head = NULL;
	u_int32_t tail_idx = 0;
	int i;

	DBGPR("-->%s: qInx = %d\n", __func__, qInx);

	if (pdev == NULL)
		return EINVAL;

	pdata = (struct DWC_ETH_QOS_prv_data *)pdev->private_data;
	if (pdata == NULL) {
		printf("failed to get valid pdata\n");
		return ENXIO;
	}

	if (qInx > pdata->rx_avb_q_cnt) {
		printf("Invalid Rx queue index(%d)\n"
			"Total number of Rx queues = %d\n",
			qInx, pdata->rx_avb_q_cnt);
		return EINVAL;
	}

	desc_data = GET_RX_WRAPPER_DESC(qInx);

	DBGPR("skb_realloc_idx = %d, dirty_rx = %d\n",
		desc_data->skb_realloc_idx, desc_data->dirty_rx);

	for (i = 0; i < desc_data->dirty_rx; i++) {
		desc = GET_RX_DESC_PTR(qInx, desc_data->skb_realloc_idx);
		buffer = GET_RX_BUF_PTR(qInx, desc_data->skb_realloc_idx);

		buffer->packet.next = NULL;

		memset(desc, 0, sizeof(desc));
		/* update buffer 1 address */
		NRB_RXD_RDES0(desc) = buffer->packet.map.paddr +
					buffer->packet.offset;
		/* set OWN, INTE and BUF1V bit */
		NRB_RXD_RDES3_OWN(desc) = 1;
		NRB_RXD_RDES3_INTE(desc) = 1;
		NRB_RXD_RDES3_BUF1V(desc) = 1;

		tail_idx = desc_data->skb_realloc_idx;
		INCR_RX_DESC_INDEX(desc_data->skb_realloc_idx, 1);
	}
	desc_data->dirty_rx = 0;

	/* update the TAIL pointer */
	DMA_RDTP_RPDR_RgWr((qInx + pdata->rx_avb_q_idx),
		GET_RX_DESC_DMA_ADDR(qInx, tail_idx));

	DBGPR("skb_realloc_idx = %d, dirty_rx = %d, tail_idx = %d\n",
		desc_data->skb_realloc_idx, desc_data->dirty_rx, tail_idx);

	DBGPR("<--%s\n", __func__);

	return 0;
}

/* api to configure CBS BW alogorithm
   Returns 0 on success and +ve error number of failure.
 */
int DWC_ETH_QOS_program_CBS_alogorithm(struct device *pdev,
          u_int8_t qInx, unsigned int bw)
{
        struct DWC_ETH_QOS_prv_data *pdata = NULL;
        struct DWC_ETH_QOS_link_cmd link;
        unsigned int idle_slope = 0;
        unsigned int send_slope = 0;
        unsigned int hi_credit = 0;
        unsigned int low_credit = 0;
        unsigned int multiplier = 1;
        int ret = 0;

	if (pdev == NULL)
		return EINVAL;

	pdata = (struct DWC_ETH_QOS_prv_data *)pdev->private_data;
	if (pdata == NULL) {
		printf("failed to get valid pdata\n");
		return ENXIO;
	}

	DBGPR("-->%s\n", __func__);
        
        /* get link parameters */
	ret = ioctl(pdata->dev, DWC_ETH_QOS_LINK_PARAM, &link);
        if (ret)
                return ENXIO;

        if (link.up == 0)
                return EINVAL;

        if (link.duplex != FULL_DUPLEX)
                return EINVAL;
        
        if (link.speed < SPEED_100)
                return EINVAL;

        if (link.speed == SPEED_1000)
                multiplier = 2;

        if (bw == 0) {
                set_tx_queue_operating_mode((qInx + pdata->tx_avb_q_idx),
                                DWC_ETH_QOS_Q_GENERIC);
                set_avb_algorithm((qInx + pdata->tx_avb_q_idx), 0);
        } else {
                /* idleslope = ((bandwidth/100) * (4 [for MII] or 8 [for GMII])) * 1024
                 * 1024 is multiplied for normalizing the calculated value
                 */
                idle_slope = (((4 * multiplier * bw) * 1024)/100);

                /* sendslope = (((100 - bandwidth)/100) * (4 [for MII] or 8 [for GMII])) * 1024
                 * 1024 is multiplied for normalizing the calculated value
                 * OR
                 * sendslope = (1024 * (4 [for MII] or 8 [for GMII]) - idle_slope)
                 */
                send_slope = (((4 * multiplier * (100 - bw)) * 1024)/100);

                hi_credit = (((DWC_ETH_QOS_MAX_INT_FRAME_SIZE * bw)/100) * 1024);
                low_credit = (((DWC_ETH_QOS_MAX_INT_FRAME_SIZE * (100 - bw))/100) * 1024);

                set_tx_queue_operating_mode((qInx + pdata->tx_avb_q_idx),
                                DWC_ETH_QOS_Q_AVB);
                set_avb_algorithm((qInx + pdata->tx_avb_q_idx), 1);
        }

	config_send_slope((qInx + pdata->tx_avb_q_idx), send_slope);
	config_idle_slope((qInx + pdata->tx_avb_q_idx), idle_slope);
	config_high_credit((qInx + pdata->tx_avb_q_idx), hi_credit);
	config_low_credit((qInx + pdata->tx_avb_q_idx), low_credit);

	DBGPR("<--%s\n", __func__);
        
        return 0;
}
