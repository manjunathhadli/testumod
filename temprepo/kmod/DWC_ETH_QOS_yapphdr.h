#ifndef __DWC_ETH_QOS_YAPPHDR_H__

#define __DWC_ETH_QOS_YAPPHDR_H__

#define DWC_ETH_QOS_MAX_TX_QUEUE_CNT 8
#define DWC_ETH_QOS_MAX_RX_QUEUE_CNT 8

/* Private IOCTL for handling device specific task */
#define DWC_ETH_QOS_PRV_IOCTL	SIOCDEVPRIVATE

#define DWC_ETH_QOS_POWERUP_MAGIC_CMD	1
#define DWC_ETH_QOS_POWERDOWN_MAGIC_CMD	2
#define DWC_ETH_QOS_POWERUP_REMOTE_WAKEUP_CMD	3
#define DWC_ETH_QOS_POWERDOWN_REMOTE_WAKEUP_CMD	4

/* for TX and RX threshold configures */
#define DWC_ETH_QOS_RX_THRESHOLD_CMD	5
#define DWC_ETH_QOS_TX_THRESHOLD_CMD	6

/* for TX and RX Store and Forward mode configures */
#define DWC_ETH_QOS_RSF_CMD	7
#define DWC_ETH_QOS_TSF_CMD	8

/* for TX DMA Operate on Second Frame mode configures */
#define DWC_ETH_QOS_OSF_CMD	9

/* for TX and RX PBL configures */
#define DWC_ETH_QOS_TX_PBL_CMD	10
#define DWC_ETH_QOS_RX_PBL_CMD	11

/* INCR and INCRX mode */
#define DWC_ETH_QOS_INCR_INCRX_CMD	12

/* RX/TX VLAN */
#define DWC_ETH_QOS_RX_OUTER_VLAN_STRIPPING_CMD	13
#define DWC_ETH_QOS_RX_INNER_VLAN_STRIPPING_CMD	14
#define DWC_ETH_QOS_TX_VLAN_DESC_CMD	15
#define DWC_ETH_QOS_TX_VLAN_REG_CMD	16

/* SA on TX */
#define DWC_ETH_QOS_SA0_DESC_CMD	17
#define DWC_ETH_QOS_SA1_DESC_CMD	18
#define DWC_ETH_QOS_SA0_REG_CMD		19
#define DWC_ETH_QOS_SA1_REG_CMD		20

/* CONTEX desc setup control */
#define DWC_ETH_QOS_SETUP_CONTEXT_DESCRIPTOR 21

/* Packet generation */
#define DWC_ETH_QOS_PG_TEST		22

/* TX/RX channel/queue count */
#define DWC_ETH_QOS_GET_TX_QCNT		23
#define DWC_ETH_QOS_GET_RX_QCNT		24

/* Line speed */
#define DWC_ETH_QOS_GET_CONNECTED_SPEED		25

/* DCB/AVB algorithm */
#define DWC_ETH_QOS_DCB_ALGORITHM		26
#define DWC_ETH_QOS_AVB_ALGORITHM		27

/* RX split header */
#define DWC_ETH_QOS_RX_SPLIT_HDR_CMD		28

/* L3/L4 filter */
#define DWC_ETH_QOS_L3_L4_FILTER_CMD		29
/* IPv4/6 and TCP/UDP filtering */
#define DWC_ETH_QOS_IPV4_FILTERING_CMD	30
#define DWC_ETH_QOS_IPV6_FILTERING_CMD	31
#define DWC_ETH_QOS_UDP_FILTERING_CMD	32
#define DWC_ETH_QOS_TCP_FILTERING_CMD	33
/* VLAN filtering */
#define DWC_ETH_QOS_VLAN_FILTERING_CMD 34
/* L2 DA filtering */
#define DWC_ETH_QOS_L2_DA_FILTERING_CMD 35
/* ARP Offload */
#define DWC_ETH_QOS_ARP_OFFLOAD_CMD 36

#define DWC_ETH_QOS_RWK_FILTER_LENGTH	8

/* List of command errors driver can set */
#define	DWC_ETH_QOS_NO_HW_SUPPORT	-1
#define	DWC_ETH_QOS_CONFIG_FAIL	-3
#define	DWC_ETH_QOS_CONFIG_SUCCESS	0

/* RX THRESHOLD operations */
#define DWC_ETH_QOS_RX_THRESHOLD_32	0x1
#define DWC_ETH_QOS_RX_THRESHOLD_64	0x0
#define DWC_ETH_QOS_RX_THRESHOLD_96	0x2
#define DWC_ETH_QOS_RX_THRESHOLD_128	0x3

/* TX THRESHOLD operations */
#define DWC_ETH_QOS_TX_THRESHOLD_32	0x1
#define DWC_ETH_QOS_TX_THRESHOLD_64	0x0
#define DWC_ETH_QOS_TX_THRESHOLD_96	0x2
#define DWC_ETH_QOS_TX_THRESHOLD_128	0x3
#define DWC_ETH_QOS_TX_THRESHOLD_192	0x4
#define DWC_ETH_QOS_TX_THRESHOLD_256	0x5
#define DWC_ETH_QOS_TX_THRESHOLD_384	0x6
#define DWC_ETH_QOS_TX_THRESHOLD_512	0x7

/* TX and RX Store and Forward Mode operations */
#define DWC_ETH_QOS_RSF_DISABLE	0x0
#define DWC_ETH_QOS_RSF_ENABLE	0x1

#define DWC_ETH_QOS_TSF_DISABLE	0x0
#define DWC_ETH_QOS_TSF_ENABLE	0x1

/* TX DMA Operate on Second Frame operations */
#define DWC_ETH_QOS_OSF_DISABLE	0x0
#define DWC_ETH_QOS_OSF_ENABLE	0x1

/* INCR and INCRX mode */
#define DWC_ETH_QOS_INCR_ENABLE		0x0
#define DWC_ETH_QOS_INCRX_ENABLE	0x1

/* TX and RX PBL operations */
#define DWC_ETH_QOS_PBL_1	1
#define DWC_ETH_QOS_PBL_2	2
#define DWC_ETH_QOS_PBL_4	4
#define DWC_ETH_QOS_PBL_8	8
#define DWC_ETH_QOS_PBL_16	16
#define DWC_ETH_QOS_PBL_32	32
#define DWC_ETH_QOS_PBL_64	64	/* 8 x 8 */
#define DWC_ETH_QOS_PBL_128	128	/* 8 x 16 */
#define DWC_ETH_QOS_PBL_256	256	/* 8 x 32 */

/* RX VLAN operations */
/* Do not strip VLAN tag from received pkt */
#define DWC_ETH_QOS_RX_NO_VLAN_STRIP	0x0
/* Strip VLAN tag if received pkt pass VLAN filter */
#define DWC_ETH_QOS_RX_VLAN_STRIP_IF_FILTER_PASS  0x1
/* Strip VLAN tag if received pkt fial VLAN filter */
#define DWC_ETH_QOS_RX_VLAN_STRIP_IF_FILTER_FAIL  0x2
/* Strip VALN tag always from received pkt */
#define DWC_ETH_QOS_RX_VLAN_STRIP_ALWAYS	0x3

/* TX VLAN operations */
/* Do not add a VLAN tag dring pkt transmission */
#define DWC_ETH_QOS_TX_VLAN_TAG_NONE	0x0
/* Remove the VLAN tag from the pkt before transmission */
#define DWC_ETH_QOS_TX_VLAN_TAG_DELETE	0x1
/* Insert the VLAN tag into pkt to be transmitted */
#define DWC_ETH_QOS_TX_VLAN_TAG_INSERT	0x2
/* Replace the VLAN tag into pkt to be transmitted */
#define DWC_ETH_QOS_TX_VLAN_TAG_REPLACE	0x3

/* RX split header operations */
#define DWC_ETH_QOS_RX_SPLIT_HDR_DISABLE 0x0
#define DWC_ETH_QOS_RX_SPLIT_HDR_ENABLE 0x1

/* L3/L4 filter operations */
#define DWC_ETH_QOS_L3_L4_FILTER_DISABLE 0x0
#define DWC_ETH_QOS_L3_L4_FILTER_ENABLE 0x1

#define DWC_ETH_QOS_MAC0REG 0
#define DWC_ETH_QOS_MAC1REG 1

#define DWC_ETH_QOS_SA0_NONE		((DWC_ETH_QOS_MAC0REG << 2) | 0) /* Do not include the SA */
#define DWC_ETH_QOS_SA0_DESC_INSERT	((DWC_ETH_QOS_MAC0REG << 2) | 1) /* Include/Insert the SA with value given in MAC Addr 0 Reg */
#define DWC_ETH_QOS_SA0_DESC_REPLACE	((DWC_ETH_QOS_MAC0REG << 2) | 2) /* Replace the SA with the value given in MAC Addr 0 Reg */
#define DWC_ETH_QOS_SA0_REG_INSERT	((DWC_ETH_QOS_MAC0REG << 2) | 2) /* Include/Insert the SA with value given in MAC Addr 0 Reg */
#define DWC_ETH_QOS_SA0_REG_REPLACE	((DWC_ETH_QOS_MAC0REG << 2) | 3) /* Replace the SA with the value given in MAC Addr 0 Reg */

#define DWC_ETH_QOS_SA1_NONE		((DWC_ETH_QOS_MAC1REG << 2) | 0) /* Do not include the SA */
#define DWC_ETH_QOS_SA1_DESC_INSERT	((DWC_ETH_QOS_MAC1REG << 2) | 1) /* Include/Insert the SA with value given in MAC Addr 1 Reg */
#define DWC_ETH_QOS_SA1_DESC_REPLACE	((DWC_ETH_QOS_MAC1REG << 2) | 2) /* Replace the SA with the value given in MAC Addr 1 Reg */
#define DWC_ETH_QOS_SA1_REG_INSERT	((DWC_ETH_QOS_MAC1REG << 2) | 2) /* Include/Insert the SA with value given in MAC Addr 1 Reg */
#define DWC_ETH_QOS_SA1_REG_REPLACE	((DWC_ETH_QOS_MAC1REG << 2) | 3) /* Replace the SA with the value given in MAC Addr 1 Reg */

#define DWC_ETH_QOS_MAX_WFQ_WEIGHT	0X7FFF /* value for bandwidth calculation */

#define DWC_ETH_QOS_MAX_INT_FRAME_SIZE (1024* 64)

typedef enum {
	eDWC_ETH_QOS_DMA_TX_FP = 0,
	eDWC_ETH_QOS_DMA_TX_WSP = 1,
	eDWC_ETH_QOS_DMA_TX_WRR = 2,
} e_DWC_ETH_QOS_dma_tx_arb_algo;

typedef enum {
	eDWC_ETH_QOS_DCB_WRR = 0,
	eDWC_ETH_QOS_DCB_WFQ = 1,
	eDWC_ETH_QOS_DCB_DWRR = 2,
	eDWC_ETH_QOS_DCB_SP = 3,
} e_DWC_ETH_QOS_dcb_algorithm;

typedef enum {
	eDWC_ETH_QOS_AVB_SP = 0,
	eDWC_ETH_QOS_AVB_CBS = 1,
} e_DWC_ETH_QOS_avb_algorithm;

typedef enum {
	eDWC_ETH_QOS_QDISABLED = 0x0,
	eDWC_ETH_QOS_QAVB,
	eDWC_ETH_QOS_QDCB,
	eDWC_ETH_QOS_QGENERIC
} eDWC_ETH_QOS_queue_operating_mode;

/* common data structure between driver and application for
 * sharing info through ioctl
 * */
struct ifr_data_struct {
	unsigned int flags;
	unsigned int qInx; /* dma channel no to be configured */
	unsigned int cmd;
	unsigned int context_setup;
	unsigned int connected_speed;
	unsigned int rwk_filter_values[DWC_ETH_QOS_RWK_FILTER_LENGTH];
	unsigned int rwk_filter_length;
	int command_error;
	int test_done;
	void *ptr;
};

struct DWC_ETH_QOS_dcb_algorithm {
	unsigned int qInx;
	unsigned int algorithm;
	unsigned int weight;
	eDWC_ETH_QOS_queue_operating_mode op_mode;
};

struct DWC_ETH_QOS_avb_algorithm {
	unsigned int qInx;
	unsigned int algorithm;
	unsigned int cc;
	unsigned int idle_slope;
	unsigned int send_slope;
	unsigned int hi_credit;
	unsigned int low_credit;
	eDWC_ETH_QOS_queue_operating_mode op_mode;
};

struct DWC_ETH_QOS_l3_l4_filter {
	/* 0, 1,2,3,4,5,6 or 7*/
	int filter_no;
	/* 0 - disable and 1 - enable */
	int filter_enb_dis;
	/* 0 - src addr/port and 1- dst addr/port match */
	int src_dst_addr_match;
	/* 0 - perfect and 1 - inverse match filtering */
	int perfect_inverse_match;
	/* To hold source/destination IPv4 addresses */
	unsigned char ip4_addr[4];
	/* holds single IPv6 addresses */
	unsigned short ip6_addr[8];

	/* TCP/UDP src/dst port number */
	unsigned short port_no;
};

struct DWC_ETH_QOS_vlan_filter {
	/* 0 - disable and 1 - enable */
	int filter_enb_dis;
	/* 0 - perfect and 1 - hash filtering */
	int perfect_hash;
};

struct DWC_ETH_QOS_l2_da_filter {
	/* 0 - perfect and 1 - hash filtering */
	int perfect_hash;
	/* 0 - perfect and 1 - inverse matching */
	int perfect_inverse_match;
};

struct DWC_ETH_QOS_arp_offload {
	unsigned char ip_addr[4];
};


#ifdef DWC_ETH_QOS_CONFIG_PGTEST

/* uncomment below macro to enable application
 * to record all run reports to file */
//#define PGTEST_LOGFILE

/* TX DMA CHANNEL Weights */
#define DWC_ETH_QOS_TX_CH_WEIGHT1	0x0
#define DWC_ETH_QOS_TX_CH_WEIGHT2	0x1
#define DWC_ETH_QOS_TX_CH_WEIGHT3	0x2
#define DWC_ETH_QOS_TX_CH_WEIGHT4	0x3
#define DWC_ETH_QOS_TX_CH_WEIGHT5	0x4
#define DWC_ETH_QOS_TX_CH_WEIGHT6	0x5
#define DWC_ETH_QOS_TX_CH_WEIGHT7	0x6
#define DWC_ETH_QOS_TX_CH_WEIGHT8	0x7

/* PG test sub commands macro's */
#define DWC_ETH_QOS_PG_SET_CONFIG	0x1
#define DWC_ETH_QOS_PG_CONFIG_HW	  0x2
#define DWC_ETH_QOS_PG_RUN_TEST		0x3
#define DWC_ETH_QOS_PG_GET_RESULT	0x4
#define DWC_ETH_QOS_PG_TEST_DONE	0x5


/* DMA channel bandwidth allocation parameters */
struct DWC_ETH_QOS_pg_user_ch_input {
	unsigned char ch_arb_weight;	/* Channel weights(1/2/3/4/5/6/7/8) for arbitration */
	unsigned int ch_fr_size;	/* Channel Frame size */
	unsigned char ch_bw_alloc;	/* The percentage bandwidth allocation for ch */

	unsigned char ch_use_slot_no_check;	/* Should Ch use slot number checking ? */
	unsigned char ch_use_adv_slot_no_check;
	unsigned char ch_slot_count_to_use;	/* How many slot used to report pg bits per slot value */

	unsigned char ch_use_credit_shape;	/* Should Ch use Credid shape algorithm for traffic shaping ? */
	unsigned char ch_CreditControl;	/* Sould Ch use Credit Control algorithm for traffic shaping ? */

	unsigned char ch_tx_desc_slot_no_start;
	unsigned char ch_tx_desc_slot_no_skip;
	unsigned char ch_operating_mode;
	unsigned long ch_AvgBits;
	unsigned long ch_AvgBits_interrupt_count;
	unsigned char ch_avb_algorithm;
	unsigned char ch_debug_mode; /* enable/disable debug mode */
	unsigned int ch_max_tx_frame_cnt; /* maximum pkts to be sent on this channel, can be used for debug purpose */
};

struct DWC_ETH_QOS_pg_user_input {
	unsigned char duration_of_exp;
	/* enable bits for DMA. bit0=>ch0, bit1=>ch1, bit2=>ch2 */
	unsigned char dma_ch_en;

	unsigned char ch_tx_rx_arb_scheme;	/* Should Ch use Weighted RR policy with Rx:Tx/Tx:Rx or Fixed Priority */
	unsigned char ch_use_tx_high_prio;	/* Should Ch Tx have High priority over Rx */
	unsigned char ch_tx_rx_prio_ratio;	/* For RR what is the ratio between Tx:Rx/Rx:Tx */
	unsigned char dma_tx_arb_algo; /* Refer DMA Mode register TAA field */

	unsigned char queue_dcb_algorithm;

	struct DWC_ETH_QOS_pg_user_ch_input ch_input[DWC_ETH_QOS_MAX_TX_QUEUE_CNT];
};

#define copy_pg_ch_input_members(to, from) do { \
	(to)->interrupt_prints = (from)->interrupt_prints; \
	(to)->tx_interrupts = (from)->tx_interrupts; \
	(to)->ch_arb_weight = (from)->ch_arb_weight; \
	(to)->ch_queue_weight = (from)->ch_queue_weight; \
	(to)->ch_bw = (from)->ch_bw; \
	(to)->ch_frame_size = (from)->ch_frame_size; \
	(to)->ch_EnableSlotCheck = (from)->ch_EnableSlotCheck; \
	(to)->ch_EnableAdvSlotCheck = (from)->ch_EnableAdvSlotCheck; \
	(to)->ch_avb_algorithm = (from)->ch_avb_algorithm; \
	(to)->ch_SlotCount = (from)->ch_SlotCount; \
	(to)->ch_AvgBits = (from)->ch_AvgBits; \
	(to)->ch_AvgBits_interrupt_count = (from)->ch_AvgBits_interrupt_count; \
	(to)->ch_CreditControl = (from)->ch_CreditControl; \
	(to)->ch_tx_desc_slot_no_start = (from)->ch_tx_desc_slot_no_start; \
	(to)->ch_tx_desc_slot_no_skip = (from)->ch_tx_desc_slot_no_skip; \
	(to)->ch_SendSlope = (from)->ch_SendSlope; \
	(to)->ch_IdleSlope = (from)->ch_IdleSlope; \
	(to)->ch_HiCredit = (from)->ch_HiCredit; \
	(to)->ch_LoCredit = (from)->ch_LoCredit; \
	(to)->ch_FramecountTx = (from)->ch_FramecountTx; \
	(to)->ch_FramecountRx = (from)->ch_FramecountRx; \
	(to)->ch_operating_mode = (from)->ch_operating_mode; \
	(to)->ch_debug_mode = (from)->ch_debug_mode;\
	(to)->ch_max_tx_frame_cnt = (from)->ch_max_tx_frame_cnt;\
} while (0)

struct DWC_ETH_QOS_pg_ch_input {
	unsigned int interrupt_prints;
	unsigned int tx_interrupts;
	unsigned char ch_arb_weight;
	unsigned int ch_queue_weight;
	unsigned char ch_bw;
	unsigned int ch_frame_size;
	unsigned char ch_EnableSlotCheck;	/* Enable checking of slot numbers programmed in the Tx Desc */
	unsigned char ch_EnableAdvSlotCheck;	/* When Set Data fetched for current slot and for next 2 slots in advance
						When reset data fetched for current slot and in advance for next slot*/

	unsigned char ch_avb_algorithm;
	unsigned char ch_SlotCount;	/* Over which transmiteed bits per slot needs to be computed (Only for Credit based shaping) */
	unsigned long ch_AvgBits;
	unsigned long ch_AvgBits_interrupt_count;

	unsigned char ch_CreditControl;	/* Will be zero (Not used) */

	unsigned char ch_tx_desc_slot_no_start;
	unsigned char ch_tx_desc_slot_no_skip;

	unsigned int ch_SendSlope;
	unsigned int ch_IdleSlope;
	unsigned int ch_HiCredit;
	unsigned int ch_LoCredit;

	unsigned long ch_FramecountTx;	/* No of Frames Transmitted on Channel 1 */
	unsigned long ch_FramecountRx;	/* No of Frames Received on Channel 1 */
	unsigned char ch_operating_mode;

	unsigned char ch_debug_mode; /* enable/disable debug mode */
	unsigned int ch_max_tx_frame_cnt; /* maximum pkts to be sent on this channel, can be used for debug purpose */
	unsigned int ch_desc_prepare; /* max packets which will be reprepared in Tx-interrupt
 																	 do not copy contents to app-copy, only driver should use this variable*/
};

#define copy_PGStruct_members(to, from)	do { \
	(to)->ch_SelMask = (from)->ch_SelMask; \
	(to)->DurationOfExp = (from)->DurationOfExp; \
	(to)->PrioTagForAV = (from)->PrioTagForAV; \
	(to)->queue_dcb_algorithm = (from)->queue_dcb_algorithm; \
	(to)->ch_tx_rx_arb_scheme = (from)->ch_tx_rx_arb_scheme; \
	(to)->ch_use_tx_high_prio = (from)->ch_use_tx_high_prio; \
	(to)->ch_tx_rx_prio_ratio = (from)->ch_tx_rx_prio_ratio; \
	(to)->dma_tx_arb_algo = (from)->dma_tx_arb_algo; \
} while (0)

struct DWC_ETH_QOS_PGStruct {
	/* This gives which DMA channel is enabled and which is disabled
	 * Bit0 for Ch0
	 * Bit1 for Ch1
	 * Bit2 for Ch2 and so on
	 * Bit7 for Ch7
	 * */
	unsigned char ch_SelMask;

	/* Duration for which experiment should be conducted in minutes - Default 2 Minutes */
	unsigned char DurationOfExp;

	/* Used when more than One channel enabled in Rx path (Not Used)
	 * for only CH1 Enabled:
	 * Frames sith Priority > Value programmed, frames sent to CH1
	 * Frames with priority < Value programmed are sent to CH0
	 *
	 * For both CH1 and CH2 Enabled:
	 * Frames sith Priority > Value programmed, frames sent to CH2
	 * Frames with priority < Value programmed are sent to CH
	 * */
	unsigned char PrioTagForAV;

	unsigned char queue_dcb_algorithm;

	unsigned char ch_tx_rx_arb_scheme;	/* Should Ch use Weighted RR policy with Rx:Tx/Tx:Rx or Fixed Priority */
	unsigned char ch_use_tx_high_prio;	/* Should Ch Tx have High priority over Rx */
	unsigned char ch_tx_rx_prio_ratio;	/* For RR what is the ratio between Tx:Rx/Rx:Tx */
	unsigned char dma_tx_arb_algo; /* Refer DMA Mode register TAA field */

	struct DWC_ETH_QOS_pg_ch_input pg_ch_input[DWC_ETH_QOS_MAX_TX_QUEUE_CNT];
	unsigned char channel_running[DWC_ETH_QOS_MAX_TX_QUEUE_CNT];
};
#endif /* end of DWC_ETH_QOS_CONFIG_PGTEST */

#endif
