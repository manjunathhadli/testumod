#ifndef __DWC_ETH_QOS_YPG_H__

#define __DWC_ETH_QOS_YPG_H__

static void DWC_ETH_QOS_tx_interrupt_pg(struct DWC_ETH_QOS_prv_data *pdata,
				     UINT qInx);

static void DWC_ETH_QOS_prepare_desc(struct DWC_ETH_QOS_prv_data *pdata,
				struct s_TX_NORMAL_DESC *txptr,
				struct DWC_ETH_QOS_tx_buffer *buffer,
				int i,
				unsigned int qInx);

static int DWC_ETH_QOS_poll_pg_sq(struct DWC_ETH_QOS_prv_data *pdata,
					unsigned int qInx);

static void DWC_ETH_QOS_poll_pg(struct DWC_ETH_QOS_prv_data *pdata);

static void DWC_ETH_QOS_prepare_hw_for_pg_test(struct DWC_ETH_QOS_prv_data *pdata);

static void DWC_ETH_QOS_pg_timer_fun(unsigned long data);

static void DWC_ETH_QOS_pg_run(struct DWC_ETH_QOS_prv_data *pdata);

static void DWC_ETH_QOS_setup_timer(struct DWC_ETH_QOS_prv_data *pdata);

static void DWC_ETH_QOS_pg_run_test(struct DWC_ETH_QOS_prv_data *pdata);

static void DWC_ETH_QOS_print_pg_struct(struct DWC_ETH_QOS_prv_data *pdata);

static void DWC_ETH_QOS_pg_set_config(struct DWC_ETH_QOS_prv_data *pdata,
					struct ifr_data_struct *req);

static void DWC_ETH_QOS_pg_get_result(struct DWC_ETH_QOS_prv_data *pdata,
					struct ifr_data_struct *req);
#endif
