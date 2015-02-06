#ifndef __DWC_ETH_QOS__DEV__H__

#define __DWC_ETH_QOS__DEV__H__

int get_tx_queue_count(void);
int get_rx_queue_count(void);
u_int32_t get_hw_rx_fifo_size(void);
u_int32_t get_hw_tx_fifo_size(void);
static u_int32_t calculate_per_queue_fifo(u_int32_t fifo_size,
					u_int32_t queue_count);
int configure_mtl_queue(struct DWC_ETH_QOS_prv_data *pdata,
				u_int8_t qInx);
int reset_mtl_queue(u_int8_t qInx);
void configure_dma_channel(struct DWC_ETH_QOS_prv_data *pdata,
				u_int8_t qInx);
void reset_dma_channel(u_int8_t qInx);
void configure_mac(struct DWC_ETH_QOS_prv_data *pdata, u_int8_t qInx);
void reset_mac(u_int8_t qInx);
int stop_dma_tx(u_int8_t qInx);
int stop_dma_rx(u_int8_t qInx);

#endif
