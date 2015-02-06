#ifndef __DWC_ETH_QOS_DRV_H__

#define __DWC_ETH_QOS_DRV_H__

static int DWC_ETH_QOS_open(struct net_device *);

static int DWC_ETH_QOS_close(struct net_device *);

static void DWC_ETH_QOS_set_rx_mode(struct net_device *);

static int DWC_ETH_QOS_start_xmit(struct sk_buff *, struct net_device *);

static void DWC_ETH_QOS_tx_interrupt(struct net_device *,
				     struct DWC_ETH_QOS_prv_data *,
				     UINT qInx);

static struct net_device_stats *DWC_ETH_QOS_get_stats(struct net_device *);

#ifdef CONFIG_NET_POLL_CONTROLLER
static void DWC_ETH_QOS_poll_controller(struct net_device *);
#endif				/*end of CONFIG_NET_POLL_CONTROLLER */

static int DWC_ETH_QOS_set_features(struct net_device *dev, u64 features);

INT DWC_ETH_QOS_configure_remotewakeup(struct net_device *,
				       struct ifr_data_struct *);

static void DWC_ETH_QOS_program_dcb_algorithm(struct DWC_ETH_QOS_prv_data *pdata,
		struct ifr_data_struct *req);

static void DWC_ETH_QOS_program_avb_algorithm(struct DWC_ETH_QOS_prv_data *pdata,
		struct ifr_data_struct *req);

static void DWC_ETH_QOS_config_tx_pbl(struct DWC_ETH_QOS_prv_data *pdata,
				      UINT tx_pbl, UINT ch_no);
static void DWC_ETH_QOS_config_rx_pbl(struct DWC_ETH_QOS_prv_data *pdata,
				      UINT rx_pbl, UINT ch_no);

static int DWC_ETH_QOS_handle_prv_ioctl(struct DWC_ETH_QOS_prv_data *pdata,
					struct ifr_data_struct *req);

static int DWC_ETH_QOS_ioctl(struct net_device *, struct ifreq *, int);

irqreturn_t DWC_ETH_QOS_ISR_SW_DWC_ETH_QOS(int, void *);

static INT DWC_ETH_QOS_change_mtu(struct net_device *dev, INT new_mtu);

static int DWC_ETH_QOS_clean_split_hdr_rx_irq(struct DWC_ETH_QOS_prv_data *pdata,
					  int quota, UINT qInx);

static int DWC_ETH_QOS_clean_jumbo_rx_irq(struct DWC_ETH_QOS_prv_data *pdata,
					  int quota, UINT qInx);

static int DWC_ETH_QOS_clean_rx_irq(struct DWC_ETH_QOS_prv_data *pdata,
				    int quota, UINT qInx);

static void DWC_ETH_QOS_consume_page(struct DWC_ETH_QOS_rx_buffer *buffer,
				     struct sk_buff *skb,
				     u16 length, u16 buf2_len);

static void DWC_ETH_QOS_receive_skb(struct DWC_ETH_QOS_prv_data *pdata,
				    struct net_device *dev,
				    struct sk_buff *skb,
				    UINT qInx);

static void DWC_ETH_QOS_configure_rx_fun_ptr(struct DWC_ETH_QOS_prv_data
					     *pdata);


static int DWC_ETH_QOS_alloc_split_hdr_rx_buf(struct DWC_ETH_QOS_prv_data *pdata,
					  struct DWC_ETH_QOS_rx_buffer *buffer,
					  gfp_t gfp);

static int DWC_ETH_QOS_alloc_jumbo_rx_buf(struct DWC_ETH_QOS_prv_data *pdata,
					  struct DWC_ETH_QOS_rx_buffer *buffer,
					  gfp_t gfp);

static int DWC_ETH_QOS_alloc_rx_buf(struct DWC_ETH_QOS_prv_data *pdata,
				    struct DWC_ETH_QOS_rx_buffer *buffer,
				    gfp_t gfp);

static void DWC_ETH_QOS_default_common_confs(struct DWC_ETH_QOS_prv_data
					     *pdata);
static void DWC_ETH_QOS_default_tx_confs(struct DWC_ETH_QOS_prv_data *pdata);
static void DWC_ETH_QOS_default_tx_confs_single_q(struct DWC_ETH_QOS_prv_data
						  *pdata, UINT qInx);
static void DWC_ETH_QOS_default_rx_confs(struct DWC_ETH_QOS_prv_data *pdata);
static void DWC_ETH_QOS_default_rx_confs_single_q(struct DWC_ETH_QOS_prv_data
						  *pdata, UINT qInx);

int DWC_ETH_QOS_poll(struct DWC_ETH_QOS_prv_data *pdata, int budget, int qInx);

static void DWC_ETH_QOS_mmc_setup(struct DWC_ETH_QOS_prv_data *pdata);
inline unsigned int DWC_ETH_QOS_reg_read(volatile ULONG *ptr);

#ifdef DWC_ETH_QOS_QUEUE_SELECT_ALGO
u16	DWC_ETH_QOS_select_queue(struct net_device *dev, struct sk_buff *skb);
#endif

static int DWC_ETH_QOS_vlan_rx_add_vid(struct net_device *dev, u16 vid);
static int DWC_ETH_QOS_vlan_rx_kill_vid(struct net_device *dev, u16 vid);

#endif
