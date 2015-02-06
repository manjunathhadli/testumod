#ifndef __DWC_ETH_QOS_DESC_H__

#define __DWC_ETH_QOS_DESC_H__

static INT allocate_buffer_and_desc(struct DWC_ETH_QOS_prv_data *);

static void DWC_ETH_QOS_wrapper_tx_descriptor_init(struct DWC_ETH_QOS_prv_data
						   *pdata);

static void DWC_ETH_QOS_wrapper_tx_descriptor_init_single_q(struct
							    DWC_ETH_QOS_prv_data
							    *pdata, UINT qInx);

static void DWC_ETH_QOS_wrapper_rx_descriptor_init(struct DWC_ETH_QOS_prv_data
						   *pdata);

static void DWC_ETH_QOS_wrapper_rx_descriptor_init_single_q(struct
							    DWC_ETH_QOS_prv_data
							    *pdata, UINT qInx);

static int DWC_ETH_QOS_get_skb_hdr(struct sk_buff *skb, void **iphdr,
				   void **tcph, u64 *hdr_flags, void *priv);

static void DWC_ETH_QOS_tx_free_mem(struct DWC_ETH_QOS_prv_data *);

static void DWC_ETH_QOS_rx_free_mem(struct DWC_ETH_QOS_prv_data *);

static unsigned int DWC_ETH_QOS_map_skb(struct net_device *, struct sk_buff *);

static void DWC_ETH_QOS_unmap_tx_skb(struct DWC_ETH_QOS_prv_data *,
				     struct DWC_ETH_QOS_tx_buffer *);

static void DWC_ETH_QOS_unmap_rx_skb(struct DWC_ETH_QOS_prv_data *,
				     struct DWC_ETH_QOS_rx_buffer *);

static void DWC_ETH_QOS_re_alloc_skb(struct DWC_ETH_QOS_prv_data *pdata,
					UINT qInx);

static void DWC_ETH_QOS_tx_desc_free_mem(struct DWC_ETH_QOS_prv_data *pdata,
					 UINT tx_q_cnt);

static void DWC_ETH_QOS_tx_buf_free_mem(struct DWC_ETH_QOS_prv_data *pdata,
					UINT tx_q_cnt);

static void DWC_ETH_QOS_rx_desc_free_mem(struct DWC_ETH_QOS_prv_data *pdata,
					 UINT rx_q_cnt);

static void DWC_ETH_QOS_rx_buf_free_mem(struct DWC_ETH_QOS_prv_data *pdata,
					UINT rx_q_cnt);

static void DWC_ETH_QOS_rx_skb_free_mem(struct DWC_ETH_QOS_prv_data *pdata,
					UINT rx_qCnt);

static void DWC_ETH_QOS_tx_skb_free_mem(struct DWC_ETH_QOS_prv_data *pdata,
					UINT tx_qCnt);
#endif
