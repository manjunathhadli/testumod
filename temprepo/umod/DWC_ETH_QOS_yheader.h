#ifndef __DWC_ETH_QOS__YHEADER__H__

#define __DWC_ETH_QOS__YHEADER__H__

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <sys/user.h>
#include <time.h>

#include "DWC_ETH_QOS_ycommon.h"

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
#define DWC_ETH_QOS_RX_SETLOCK		_IOW('E', 210, int)

#define DWC_ETH_QOS_MAX_INT_FRAME_SIZE (1024* 64)

#define SPEED_10        10
#define SPEED_100       100
#define SPEED_1000      1000
#define HALF_DUPLEX     0
#define FULL_DUPLEX     1

/* Uncomment to dump statictics of pkt received or transmitted */
//#define DEBUG_DUMP

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
	u_int64_t phys_addr;
	u_int32_t qInx;
	u_int32_t mmap_size;
	u_int32_t alloc_size;
};

/*
 * @pdev_name - name of pci device
 * @mmap_size - device CSR address space
 * */
struct DWC_ETH_QOS_bind_cmd {
	char pdev_name[DWC_ETH_QOS_BIND_NAME_SIZE];
	u_int32_t mmap_size;
};

/*
 * @up - link is up/down -
 * @speed - device speed- SPEED_10/100/1000/2500/10000/UNKNOWN
 * @duplex - DUPLEX_HALF/FULL/UNKNOWN
 */
struct DWC_ETH_QOS_link_cmd {
	u_int32_t up;
	u_int32_t speed;
	u_int32_t duplex;
};


/* max and min packet size are defined assuming
 * std Eth sized frames with VALN tag
 * */
#define DWC_ETH_QOS_MAX_FRAME_SIZE 1518
#define DWC_ETH_QOS_MIN_FRAME_SIZE 64

#define RX_BUF_SIZE 2048

#define SNPS_VENDOR_ID 0x700
#define SNPS_DWC_ETH_QOS_DEVICE_ID 0x1018

#define DWC_ETH_QOS_ETH_HDR_LEN 6

#define DWC_ETH_QOS_MAX_AVB_Q_CNT 2

/* MTL queue operation mode values */
#define DWC_ETH_QOS_Q_DISABLED	0x0
#define DWC_ETH_QOS_Q_AVB 			0x1
#define DWC_ETH_QOS_Q_DCB 			0x2
#define DWC_ETH_QOS_Q_GENERIC 	0x3

/* MAC Rx queue operation mode values */
#define DWC_ETH_QOS_Q_DISABLED	0x0
#define DWC_ETH_QOS_Q_AVB 			0x1
#define DWC_ETH_QOS_Q_DCB 			0x2
#define DWC_ETH_QOS_Q_GENERIC 	0x3


/* Helper macros for TX descriptor handling */

#define GET_TX_QUEUE_PTR(qInx) (&(pdata->tx_queue[(qInx)]))

#define GET_TX_DESC_PTR(qInx, dInx) (pdata->tx_queue[(qInx)].tx_desc_data.tx_desc_ptrs[(dInx)])

#define GET_TX_DESC_DMA_ADDR(qInx, dInx) (pdata->tx_queue[(qInx)].tx_desc_data.tx_desc_dma_addrs[(dInx)])

#define GET_TX_WRAPPER_DESC(qInx) (&(pdata->tx_queue[(qInx)].tx_desc_data))

#define GET_TX_BUF_PTR(qInx, dInx) (pdata->tx_queue[(qInx)].tx_desc_data.tx_buf_ptrs[(dInx)])

#define GET_TX_DESC_SIZE(qInx) (pdata->tx_queue[(qInx)].tx_desc_data.mmap_size)

#define INCR_TX_DESC_INDEX(inx, offset) do {\
	(inx) += (offset);\
	if ((inx) >= TX_DESC_CNT)\
		(inx) = ((inx) - TX_DESC_CNT);\
} while (0)

#define DECR_TX_DESC_INDEX(inx) do {\
  (inx)--;\
  if ((inx) < 0)\
    (inx) = (TX_DESC_CNT + (inx));\
} while (0)

#define INCR_TX_LOCAL_INDEX(inx, offset)\
	(((inx) + (offset)) >= TX_DESC_CNT ?\
	((inx) + (offset) - TX_DESC_CNT) : ((inx) + (offset)))

#define GET_CURRENT_XFER_DESC_CNT(qInx) (pdata->tx_queue[(qInx)].tx_desc_data.packet_count)

#define GET_CURRENT_XFER_LAST_DESC_INDEX(qInx, start_index, offset)\
	(GET_CURRENT_XFER_DESC_CNT((qInx)) == 0) ? (TX_DESC_CNT - 1) :\
	((GET_CURRENT_XFER_DESC_CNT((qInx)) == 1) ? (INCR_TX_LOCAL_INDEX((start_index), (offset))) :\
	INCR_TX_LOCAL_INDEX((start_index), (GET_CURRENT_XFER_DESC_CNT((qInx)) + (offset) - 1)))

#define GET_TX_DESC_IDX(qInx, desc) (((desc) - GET_TX_DESC_DMA_ADDR((qInx), 0))/(sizeof(struct DWC_ETH_QOS_tx_desc)))

/* Helper macros for RX descriptor handling */

#define GET_RX_QUEUE_PTR(qInx) (&(pdata->rx_queue[(qInx)]))

#define GET_RX_DESC_PTR(qInx, dInx) (pdata->rx_queue[(qInx)].rx_desc_data.rx_desc_ptrs[(dInx)])

#define GET_RX_DESC_DMA_ADDR(qInx, dInx) (pdata->rx_queue[(qInx)].rx_desc_data.rx_desc_dma_addrs[(dInx)])

#define GET_RX_WRAPPER_DESC(qInx) (&(pdata->rx_queue[(qInx)].rx_desc_data))

#define GET_RX_BUF_PTR(qInx, dInx) (pdata->rx_queue[(qInx)].rx_desc_data.rx_buf_ptrs[(dInx)])

#define GET_RX_DESC_SIZE(qInx) (pdata->rx_queue[(qInx)].rx_desc_data.mmap_size)

#define INCR_RX_DESC_INDEX(inx, offset) do {\
  (inx) += (offset);\
  if ((inx) >= RX_DESC_CNT)\
    (inx) = ((inx) - RX_DESC_CNT);\
} while (0)

#define DECR_RX_DESC_INDEX(inx) do {\
	(inx)--;\
	if ((inx) < 0)\
		(inx) = (RX_DESC_CNT + (inx));\
} while (0)

#define INCR_RX_LOCAL_INDEX(inx, offset)\
	(((inx) + (offset)) >= RX_DESC_CNT ?\
	((inx) + (offset) - RX_DESC_CNT) : ((inx) + (offset)))

#define GET_CURRENT_RCVD_LAST_DESC_INDEX(start_index, offset) (RX_DESC_CNT - 1)

#define GET_RX_DESC_IDX(qInx, desc) (((desc) - GET_RX_DESC_DMA_ADDR((qInx), 0))/(sizeof(struct DWC_ETH_QOS_rx_desc)))

#define GET_RX_DATA_BUF_PTR(qInx, dInx) (&(pdata->rx_queue[(qInx)].rx_desc_data.rx_data_buf))


#define FIFO_SIZE_B(x) (x)
#define FIFO_SIZE_KB(x) (x*1024)

typedef enum {
	eDWC_ETH_QOS_256 = 0x0,
	eDWC_ETH_QOS_512 = 0x1,
	eDWC_ETH_QOS_1k = 0x3,
	eDWC_ETH_QOS_2k = 0x7,
	eDWC_ETH_QOS_4k = 0xf,
	eDWC_ETH_QOS_8k = 0x1f,
	eDWC_ETH_QOS_16k = 0x3f,
	eDWC_ETH_QOS_32k = 0x7f
} eDWC_ETH_QOS_mtl_fifo_size;


/* helper macros to read/write tx descriptor fields */
#define NRB_TXD_TDES0(ptr)         ((ptr)->normal_desc_rb.tdes0)
#define NRB_TXD_TDES1(ptr)         ((ptr)->normal_desc_rb.tdes1)
#define NRB_TXD_TDES2_B1L(ptr)     ((ptr)->normal_desc_rb.tdes2.b1l)
#define NRB_TXD_TDES2_VTIR(ptr)    ((ptr)->normal_desc_rb.tdes2.vtir)
#define NRB_TXD_TDES2_TTSE(ptr)    ((ptr)->normal_desc_rb.tdes2.ttse)
#define NRB_TXD_TDES2_IC(ptr)      ((ptr)->normal_desc_rb.tdes2.ic)
#define NRB_TXD_TDES3_FL(ptr)      ((ptr)->normal_desc_rb.tdes3.fl)
#define NRB_TXD_TDES3_TIPH(ptr)    ((ptr)->normal_desc_rb.tdes3.tiplh)
#define NRB_TXD_TDES3_CIC(ptr)     ((ptr)->normal_desc_rb.tdes3.CIC)
#define NRB_TXD_TDES3_TSE(ptr)     ((ptr)->normal_desc_rb.tdes3.TSE)
#define NRB_TXD_TDES3_SLOTNUM(ptr) ((ptr)->normal_desc_rb.tdes3.slotnum)
#define NRB_TXD_TDES3_SAIC(ptr)    ((ptr)->normal_desc_rb.tdes3.saic)
#define NRB_TXD_TDES3_CPC(ptr)     ((ptr)->normal_desc_rb.tdes3.cpc)
#define NRB_TXD_TDES3_LD(ptr)      ((ptr)->normal_desc_rb.tdes3.ld)
#define NRB_TXD_TDES3_FD(ptr)      ((ptr)->normal_desc_rb.tdes3.fd)
#define NRB_TXD_TDES3_CTXT(ptr)    ((ptr)->normal_desc_rb.tdes3.ctxt)
#define NRB_TXD_TDES3_OWN(ptr)     ((ptr)->normal_desc_rb.tdes3.own)

#define NWB_TXD_TDES0(ptr)       ((ptr)->normal_desc_wb.tdes0)
#define NWB_TXD_TDES1(ptr)       ((ptr)->normal_desc_wb.tdes1)
#define NWB_TXD_TDES2(ptr)       ((ptr)->normal_desc_wb.tdes2)
#define NWB_TXD_TDES3_IHE(ptr)   ((ptr)->normal_desc_wb.tdes3.ihe)
#define NWB_TXD_TDES3_DB(ptr)    ((ptr)->normal_desc_wb.tdes3.db)
#define NWB_TXD_TDES3_UF(ptr)    ((ptr)->normal_desc_wb.tdes3.ed)
#define NWB_TXD_TDES3_ED(ptr)    ((ptr)->normal_desc_wb.tdes3.cc)
#define NWB_TXD_TDES3_LC(ptr)    ((ptr)->normal_desc_wb.tdes3.ec)
#define NWB_TXD_TDES3_LOC(ptr)   ((ptr)->normal_desc_wb.tdes3.lc)
#define NWB_TXD_TDES3_PCE(ptr)   ((ptr)->normal_desc_wb.tdes3.pce)
#define NWB_TXD_TDES3_FF(ptr)    ((ptr)->normal_desc_wb.tdes3.ff)
#define NWB_TXD_TDES3_JT(ptr)    ((ptr)->normal_desc_wb.tdes3.jt)
#define NWB_TXD_TDES3_ES(ptr)    ((ptr)->normal_desc_wb.tdes3.es)
#define NWB_TXD_TDES3_RSVD1(ptr) ((ptr)->normal_desc_wb.tdes3.rsvd1)
#define NWB_TXD_TDES3_TTSS(ptr)  ((ptr)->normal_desc_wb.tdes3.ttss)
#define NWB_TXD_TDES3_RSVD2(ptr) ((ptr)->normal_desc_wb.tdes3.rsvd2)
#define NWB_TXD_TDES3_LD(ptr)    ((ptr)->normal_desc_wb.tdes3.ld)
#define NWB_TXD_TDES3_FD(ptr)    ((ptr)->normal_desc_wb.tdes3.fd)
#define NWB_TXD_TDES3_CTXT(ptr)  ((ptr)->normal_desc_wb.tdes3.ctxt)
#define NWB_TXD_TDES3_OWN(ptr)   ((ptr)->normal_desc_wb.tdes3.own)

#define CTXT_TXD_TDES0(ptr)        ((ptr)->ctxt_desc.tdes0)
#define CTXT_TXD_TDES1(ptr)        ((ptr)->ctxt_desc.tdes1)
#define CTXT_TXD_TDES2_MSS(ptr)    ((ptr)->ctxt_desc.tdes2.mss)
#define CTXT_TXD_TDES2_RSVD(ptr)   ((ptr)->ctxt_desc.tdes2.rsvd)
#define CTXT_TXD_TDES2_IVT(ptr)    ((ptr)->ctxt_desc.tdes2.ivt)
#define CTXT_TXD_TDES3_VT(ptr)     ((ptr)->ctxt_desc.tdes3.vt)
#define CTXT_TXD_TDES3_VLTV(ptr)   ((ptr)->ctxt_desc.tdes3.vltv)
#define CTXT_TXD_TDES3_IVTIR(ptr)  ((ptr)->ctxt_desc.tdes3.ivtir)
#define CTXT_TXD_TDES3_RSVD1(ptr)  ((ptr)->ctxt_desc.tdes3.rsvd1)
#define CTXT_TXD_TDES3_CDE(ptr)    ((ptr)->ctxt_desc.tdes3.cde)
#define CTXT_TXD_TDES3_RSVD2(ptr)  ((ptr)->ctxt_desc.tdes3.rsvd2)
#define CTXT_TXD_TDES3_TCMSSV(ptr) ((ptr)->ctxt_desc.tdes3.tcmssv)
#define CTXT_TXD_TDES3_OSTC(ptr)   ((ptr)->ctxt_desc.tdes3.ostc)
#define CTXT_TXD_TDES3_RSVD3(ptr)  ((ptr)->ctxt_desc.tdes3.rsvd3)
#define CTXT_TXD_TDES3_CTXT(ptr)   ((ptr)->ctxt_desc.tdes3.ctxt)
#define CTXT_TXD_TDES3_OWN(ptr)    ((ptr)->ctxt_desc.tdes3.own)

/* tx descriptor structure */
union DWC_ETH_QOS_tx_desc {
	struct {
		u_int32_t tdes0; /* Buffer 1 Address Pointer or Header Address Pointer */
		u_int32_t tdes1; /* Buffer 2 Address Pointer */
		struct {
			u_int32_t b1l:14; /* Header Length or Buffer 1 Length */
			u_int32_t vtir:2; /* VLAN Tag Insertion or Replacement */
			u_int32_t b2l:14; /* Buffer 2 Length */
			u_int32_t ttse:1; /* Transmit Timestamp Enable */
			u_int32_t ic:1; /* Interrupt on Completion */
		}tdes2;
		struct {
			u_int32_t fl:15; /* Packet Length or TCP Payload Length */
			u_int32_t tiplh:1; /* Reserved or TCP Payload Length High */
			u_int32_t cic:2; /* Checksum Insertion Control or TCP Payload Length */
			u_int32_t tse:1; /* TCP Segmentation Enable */
			u_int32_t slotnum:4; /* Slot Number Control Bits in AV mode or TCP Header Length */
			u_int32_t saic:3; /* SA Insertion Control */
			u_int32_t cpc:2; /* CRC Pad Control */
			u_int32_t ld:1; /* Last Descriptor */
			u_int32_t fd:1; /* First Descriptor */
			u_int32_t ctxt:1; /* Context Type */
			u_int32_t own:1; /* Own Bit */
		}tdes3;
	}normal_desc_rb;
	struct {
		u_int32_t tdes0; /* Timestamp Low */
		u_int32_t tdes1; /* Timestamp High */
		u_int32_t tdes2; /* Reserved */
		struct {
			u_int32_t ihe:1; /* */
			u_int32_t db:1;
			u_int32_t uf:1;
			u_int32_t ed:1;
			u_int32_t cc:4;
			u_int32_t ec:1;
			u_int32_t lc:1;
			u_int32_t nc:1;
			u_int32_t loc:1;
			u_int32_t pce:1;
			u_int32_t ff:1;
			u_int32_t jt:1;
			u_int32_t es:1;
			u_int32_t rsvd1:1;
			u_int32_t ttss:1;
			u_int32_t rsvd2:10;
			u_int32_t ld:1;
			u_int32_t fd:1;
			u_int32_t ctxt:1;
			u_int32_t own:1;
		}tdes3;
	}normal_desc_wb;
	struct {
		u_int32_t tdes0; /* Timestamp Low */
		u_int32_t tdes1; /* Timestamp High */
		struct {
			u_int32_t mss:15; /* Maximum Segment Size */
			u_int32_t rsvd:1; /* Reserved */
			u_int32_t ivt:16; /* Inner VLAN Tag */
		}tdes2;
		struct {
			u_int32_t vt:16; /* VLAN tag */
			u_int32_t vltv:1; /* VLAN Tag Valid */
			u_int32_t ivltv:1; /* Inner VLAN Tag Valid */
			u_int32_t ivtir:2; /* Inner VLAN Tag Insert or Replace */
			u_int32_t rsvd1:3; /* Reserved */
			u_int32_t cde:1; /* Context Descriptor Error */
			u_int32_t rsvd2:2; /* Reserved */
			u_int32_t tcmssv:1; /* One Step Timestamp Correction or MSS valid */
			u_int32_t ostc:1; /* One Step Timestamp Correction Enable */
			u_int32_t rsvd3:2; /* Reserved */
			u_int32_t ctxt:1; /* Context Type */
			u_int32_t own:1; /* Own Bit */
		}tdes3;
	}ctxt_desc;
};


/* helper macros to read/write rx descriptor fields */
#define NRB_RXD_RDES0(ptr)          ((ptr)->normal_desc_rb.rdes0)
#define NRB_RXD_RDES1(ptr)          ((ptr)->normal_desc_rb.rdes1)
#define NRB_RXD_RDES2(ptr)          ((ptr)->normal_desc_rb.rdes2)
#define NRB_RXD_RDES3_RSVD1(ptr)    ((ptr)->normal_desc_rb.rdes3.rsvd1)
#define NRB_RXD_RDES3_BUF1V(ptr)     ((ptr)->normal_desc_rb.rdes3.buf1v)
#define NRB_RXD_RDES3_BUf2V(ptr)     ((ptr)->normal_desc_rb.rdes3.buf2v)
#define NRB_RXD_RDES3_RSVD2(ptr)    ((ptr)->normal_desc_rb.rdes3.rsvd2)
#define NRB_RXD_RDES3_INTE(ptr)     ((ptr)->normal_desc_rb.rdes3.inte)
#define NRB_RXD_RDES3_OWN(ptr)      ((ptr)->normal_desc_rb.rdes3.own)

#define NWB_RXD_RDES0_VT(ptr)       ((ptr)->normal_desc_wb.rdes0.vt)
#define NWB_RXD_RDES0_RSVD(ptr)     ((ptr)->normal_desc_wb.rdes0.rsvd)
#define NWB_RXD_RDES1_PT(ptr)       ((ptr)->normal_desc_wb.rdes1.pt)
#define NWB_RXD_RDES1_IPHE(ptr)     ((ptr)->normal_desc_wb.rdes1.iphe)
#define NWB_RXD_RDES1_IPV4(ptr)     ((ptr)->normal_desc_wb.rdes1.ipv4)
#define NWB_RXD_RDES1_IPV6(ptr)     ((ptr)->normal_desc_wb.rdes1.ipv6)
#define NWB_RXD_RDES1_IPCB(ptr)     ((ptr)->normal_desc_wb.rdes1.ipcb)
#define NWB_RXD_RDES1_IPCE(ptr)     ((ptr)->normal_desc_wb.rdes1.ipce)
#define NWB_RXD_RDES1_PMT(ptr)      ((ptr)->normal_desc_wb.rdes1.pmt)
#define NWB_RXD_RDES1_PFT(ptr)      ((ptr)->normal_desc_wb.rdes1.pft)
#define NWB_RXD_RDES1_PV(ptr)       ((ptr)->normal_desc_wb.rdes1.pv)
#define NWB_RXD_RDES1_TSA(ptr)      ((ptr)->normal_desc_wb.rdes1.tsa)
#define NWB_RXD_RDES1_TD(ptr)       ((ptr)->normal_desc_wb.rdes1.td)
#define NWB_RXD_RDES1_IPT1C(ptr)    ((ptr)->normal_desc_wb.rdes1.ipt1c)
#define NWB_RXD_RDES2_HL(ptr)       ((ptr)->normal_desc_wb.rdes2.hl)
#define NWB_RXD_RDES2_ARPRCVD(ptr)  ((ptr)->normal_desc_wb.rdes2.arprcvd)
#define NWB_RXD_RDES2_ARPNR(ptr)    ((ptr)->normal_desc_wb.rdes2.arpnr)
#define NWB_RXD_RDES2_RSVD(ptr)     ((ptr)->normal_desc_wb.rdes2.rsvd)
#define NWB_RXD_RDES2_VF(ptr)       ((ptr)->normal_desc_wb.rdes2.vf)
#define NWB_RXD_RDES2_SAF(ptr)      ((ptr)->normal_desc_wb.rdes2.saf)
#define NWB_RXD_RDES2_DAF(ptr)      ((ptr)->normal_desc_wb.rdes2.daf)
#define NWB_RXD_RDES2_HF(ptr)       ((ptr)->normal_desc_wb.rdes2.hf)
#define NWB_RXD_RDES2_MADRM(ptr)    ((ptr)->normal_desc_wb.rdes2.madrm)
#define NWB_RXD_RDES2_L3FM(ptr)     ((ptr)->normal_desc_wb.rdes2.l3fm)
#define NWB_RXD_RDES2_L4FM(ptr)     ((ptr)->normal_desc_wb.rdes2.l4fm)
#define NWB_RXD_RDES2_L3L4FM(ptr)   ((ptr)->normal_desc_wb.rdes2.l3l4fm)
#define NWB_RXD_RDES3_PL(ptr)       ((ptr)->normal_desc_wb.rdes3.pl)
#define NWB_RXD_RDES3_ES(ptr)       ((ptr)->normal_desc_wb.rdes3.es)
#define NWB_RXD_RDES3_LT(ptr)       ((ptr)->normal_desc_wb.rdes3.lt)
#define NWB_RXD_RDES3_DE(ptr)       ((ptr)->normal_desc_wb.rdes3.de)
#define NWB_RXD_RDES3_RE(ptr)       ((ptr)->normal_desc_wb.rdes3.re)
#define NWB_RXD_RDES3_OE(ptr)       ((ptr)->normal_desc_wb.rdes3.oe)
#define NWB_RXD_RDES3_RWT(ptr)      ((ptr)->normal_desc_wb.rdes3.rwt)
#define NWB_RXD_RDES3_GP(ptr)       ((ptr)->normal_desc_wb.rdes3.gp)
#define NWB_RXD_RDES3_CE(ptr)       ((ptr)->normal_desc_wb.rdes3.ce)
#define NWB_RXD_RDES3_RS0V(ptr)     ((ptr)->normal_desc_wb.rdes3.rs0v)
#define NWB_RXD_RDES3_RS1V(ptr)     ((ptr)->normal_desc_wb.rdes3.rs1v)
#define NWB_RXD_RDES3_RS2V(ptr)     ((ptr)->normal_desc_wb.rdes3.rs2v)
#define NWB_RXD_RDES3_LD(ptr)       ((ptr)->normal_desc_wb.rdes3.ld)
#define NWB_RXD_RDES3_FD(ptr)       ((ptr)->normal_desc_wb.rdes3.fd)
#define NWB_RXD_RDES3_CTXT(ptr)     ((ptr)->normal_desc_wb.rdes3.ctxt)
#define NWB_RXD_RDES3_OWN(ptr)      ((ptr)->normal_desc_wb.rdes3.own)

#define CTXT_RXD_RDES0(ptr)         ((ptr)->ctxt_desc.rdes0)
#define CTXT_RXD_RDES1(ptr)         ((ptr)->ctxt_desc.rdes1)
#define CTXT_RXD_RDES2(ptr)         ((ptr)->ctxt_desc.rdes2)
#define CTXT_RXD_RDES3_RSVD(ptr)    ((ptr)->ctxt_desc.rdes3.rsvd)
#define CTXT_RXD_RDES3_CTXT(ptr)    ((ptr)->ctxt_desc.rdes3.ctxt)
#define CTXT_RXD_RDES3_OWN(ptr)     ((ptr)->ctxt_desc.rdes3.own)

/* rx descriptor structure */
union DWC_ETH_QOS_rx_desc {
	struct {
		u_int32_t rdes0; /* Buffer 1 Address Pointer or Header Address Pointer */
		u_int32_t rdes1; /* Reserved */
		u_int32_t rdes2; /* Buffer 2 Address Pointer */
		struct {
			u_int32_t rsvd1:24; /* Reserved */
			u_int32_t buf1v:1; /* Buffer 1 valid */
			u_int32_t buf2v:1; /* Buffer 2 valid */
			u_int32_t rsvd2:4; /* Reserved */
			u_int32_t inte:1; /* Interrupt Enable on Completion */
			u_int32_t own:1; /* Own Bit */
		}rdes3;
	}normal_desc_rb;
	struct {
		struct {
			u_int32_t vt:16; /* VLAN Tag */
			u_int32_t rsvd:16; /* Reserved */
		}rdes0;
		struct {
			u_int32_t pt:3; /* Packet Type */
			u_int32_t iphe:1; /* IP Header Error */
			u_int32_t ipv4:1; /* IPv4 Header Present */
			u_int32_t ipv6:1; /* IPv6 Header Present */
			u_int32_t ipcb:1; /* IP Checksum Bypassed */
			u_int32_t ipce:1; /* IPC Checksum Error */
			u_int32_t pmt:4; /* PTP Message Type */
			u_int32_t pft:1; /* PTP Packet Type */
			u_int32_t pv:1; /* PTP Version */
			u_int32_t tsa:1; /* Timestamp Available */
			u_int32_t td:1; /* Timestamp Dropped */
			u_int32_t ipt1c:16; /* IP Type 1 Checksum, OAM Sub-Type Code
			                       or MAC Control Packet opcode */
		}rdes1;
		struct {
			u_int32_t hl:10; /* L3/L4 Header Length */
			u_int32_t arprcvd:1; /* ARP Packet Received */
			u_int32_t arpnr:1; /* ARP Reply Not Generated */
			u_int32_t rsvd:3; /* Reserved */
			u_int32_t vf:1; /* VLAN Filter Status */
			u_int32_t saf:1; /* SA Address Filter Fail */
			u_int32_t daf:1; /* Destination Address Filter Fail */
			u_int32_t hf:1; /* Hash Filter Status */
			u_int32_t madrm:8; /* MAC Address Match or Hash Value */
			u_int32_t l3fm:1; /* Layer 3 Filter Match */
			u_int32_t l4fm:1; /* Layer 4 Filter Match */
			u_int32_t l3l4fm:3; /* Layer 3 and Layer 4 Filter Number Matched */
		}rdes2;
		struct {
			u_int32_t pl:15; /* Packet Length */
			u_int32_t es:1; /* Error Summary */
			u_int32_t lt:3; /* Length/Type Field */
			u_int32_t de:1; /* Dribble bit Error */
			u_int32_t re:1; /* Receive Error */
			u_int32_t oe:1; /* Overflow Error */
			u_int32_t rwt:1; /* Receive Watchdog Timeout */
			u_int32_t gp:1; /* Gaint Packet */
			u_int32_t ce:1; /* CRC Error */
			u_int32_t rs0v:1; /* Receive Status RDES0 Valid */
			u_int32_t rs1v:1; /* Receive Status RDES1 Valid */
			u_int32_t rs2v:1; /* Receive Status RDES2 Valid */
			u_int32_t ld:1; /* Last Descriptor */
			u_int32_t fd:1; /* First Descriptor */
			u_int32_t ctxt:1; /* Receive Context Descriptor */
			u_int32_t own:1; /* Own Bit */
		}rdes3;
	}normal_desc_wb;
	struct {
		u_int32_t rdes0; /* Timestamp Low */
		u_int32_t rdes1; /* Timestamp High */
		u_int32_t rdes2; /* Reserved */
		struct {
			u_int32_t rsvd:30; /* Reserved */
			u_int32_t ctxt:1; /* Context Type */
			u_int32_t own:1; /* Own Bit */
		}rdes3;
	}ctxt_desc;
};


/*
 * @vendor_id - pci vendor id
 * @device_id - pci device id
 */
struct DWC_ETH_QOS_vendor_info {
	unsigned int vendor_id;
	unsigned int device_id;
};


/*
 * @next_eop - index of last descriptor with EOP set in a set of descriptors
 *             for one packet transfer. It will be +ve index number for contex
 *             descriptor or descriptor with SOP set in case contex
 *             descriptor is not required else -1 for all other descriptor.
 * @packet - pointer to data packet.
 */
struct DWC_ETH_QOS_tx_buffer {
	int next_eop;
	struct DWC_ETH_QOS_packet *packet;
};


/*
 * @tx_desc_ptrs - list of all the allocated descriptors virtual address.
 * @tx_desc_dma_addrs - list of all the allocated descriptors dma address.
 * @mmap_size - total size of descriptor allocated per queue
 * @tx_buf_ptrs - list of all the allocated wrapper buffer virtual address.
 * @cur_tx - descriptor which has to be used for current transfer.
 * @dirty_tx - index of descriptor which has to be checked for
 *             transfer complete.
 * @free_desc_cnt - total number of available free descriptor count
 *                  for driver.
 * @tx_pkt_queued - total number of packet queued for transmission
 * @queue_stopped - queue/channel is stopped due to non available
 *                  free descriptor. (not used)
 * @bytes - total bytes of data transmitted;
 * @packet - total number of packet transmitted
 */
struct DWC_ETH_QOS_tx_wrapper_descriptor {
	void *tx_desc_ptrs[TX_DESC_CNT];
	u_int64_t tx_desc_dma_addrs[TX_DESC_CNT];
	u_int32_t mmap_size;

	struct DWC_ETH_QOS_tx_buffer *tx_buf_ptrs[TX_DESC_CNT];

	int cur_tx;
	int dirty_tx;
	unsigned int free_desc_cnt;
	unsigned int tx_pkt_queued;
	unsigned int queue_stopped;
	unsigned long bytes;
	unsigned long packets;
};


/*
 * @tx_desc_data - pointer to tx wrapper descriptor structure
 * @q_op_mode - operating mode of queue(GENERIC, AVB or DCB)
 * */
struct DWC_ETH_QOS_tx_queue {
	struct DWC_ETH_QOS_tx_wrapper_descriptor tx_desc_data;
	int q_op_mode;
};


/*
 * @packet - pointer to data packet.
 */
struct DWC_ETH_QOS_rx_buffer {
	struct DWC_ETH_QOS_packet packet;
};


/*
 * @rx_desc_ptrs - list of all the allocated descriptors virtual address.
 * @rx_desc_dma_addrs - list of all the allocated descriptors dma address.
 * @mmap_size - total size of descriptor allocated per queue
 * @rx_buf_ptrs - list of all the allocated wrapper buffer virtual address.
 * @rx_data_buf - starting address of receive data buffer.
 * @cur_rx - descriptor which has to be used for current receive.
 * @dirty_rx - total number of descriptor count which have to be reallocated
 *             and given ownership to device.
 * @skb_realloc_idx -
 * @bytes - total bytes of data transmitted
 * @packet - total number of packet transmitted
 * @error_pkts - total number of error packets received
 * @pkt_present -
 */
struct DWC_ETH_QOS_rx_wrapper_descriptor {
	void *rx_desc_ptrs[RX_DESC_CNT];
	u_int64_t rx_desc_dma_addrs[RX_DESC_CNT];
	u_int32_t mmap_size;

	struct DWC_ETH_QOS_rx_buffer *rx_buf_ptrs[RX_DESC_CNT];
	struct DWC_ETH_QOS_user_buff rx_data_buf;

	int cur_rx;
	int dirty_rx;
	unsigned int skb_realloc_idx;

	unsigned long bytes;
	unsigned long packets;
	unsigned long error_pkts;

	unsigned int pkt_present;
};


/*
 * @rx_desc_data - pointer to rx wrapper descriptor structure
 * @q_op_mode - operating mode of queue(GENERIC, AVB or DCB)
 * */
struct DWC_ETH_QOS_rx_queue {
	struct DWC_ETH_QOS_rx_wrapper_descriptor rx_desc_data;
	int q_op_mode;
};


/*
 * @dev - file descriptor
 * @csr_space - device CSR address space
 * @max_frame_size - maximum frame size in bytes (not used)
 * @max_frame_size - minimum frame size in bytes (not used)
 * @pdev - pointer to device details
 * @base_addr - device CSR base address
 * @mac_addr - device hw/mac address (not used)
 * @tx_queue - pointer to tx wrapper queue structure
 * @tx_q_cnt - total number of transmit queues supported by device
 * @tx_avb_q_cnt - total number of tx AVB queue count
 * @tx_avb_q_idx - starting index of tx AVB queue
 * @rx_queue - pointer to rx wrapper queue structure
 * @rx_q_cnt - total number of receive queues supported by device
 * @rx_avb_q_cnt - total number of rx AVB queue count
 * @rx_avb_q_idx - starting index of rx AVB queue
 * */
struct DWC_ETH_QOS_prv_data {
	int dev;
	struct resources csr_space;
	int max_frame_size;
	int min_frame_size;

	struct device *pdev;

	/* HW details */
	u_int8_t *base_addr;
	u_int8_t mac_addr[DWC_ETH_QOS_ETH_HDR_LEN];

	/* TX Queue */
	struct DWC_ETH_QOS_tx_queue *tx_queue;
	u_int8_t tx_q_cnt;
	u_int8_t tx_avb_q_cnt;
	u_int8_t tx_avb_q_idx;

	/* RX Queue */
	struct DWC_ETH_QOS_rx_queue *rx_queue;
	u_int8_t rx_q_cnt;
	u_int8_t rx_avb_q_cnt;
	u_int8_t rx_avb_q_idx;
};

//#define YDEBUG
#ifdef YDEBUG
#define DBGPR(x...) printf(x)
#else
#define DBGPR(x...) do { } while (0)
#endif

#endif
