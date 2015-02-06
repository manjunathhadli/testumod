#include "DWC_ETH_QOS_yheader.h"
#include "DWC_ETH_QOS_yregacc.h"

int get_tx_queue_count(void)
{
	int count;
	unsigned int varMAC_HFR2;

	MAC_HFR2_RgRd(varMAC_HFR2);
	count = GET_VALUE(varMAC_HFR2, MAC_HFR2_TXCHCNT_LPOS, MAC_HFR2_TXCHCNT_HPOS);

	return (count + 1);
}

int get_rx_queue_count(void)
{
	int count;
	unsigned int varMAC_HFR2;

	MAC_HFR2_RgRd(varMAC_HFR2);
	count = GET_VALUE(varMAC_HFR2, MAC_HFR2_RXCHCNT_LPOS, MAC_HFR2_RXCHCNT_HPOS);

	return (count + 1);
}

u_int32_t get_hw_rx_fifo_size(void)
{
	u_int32_t varMAC_HFR1;

	MAC_HFR1_RgRd(varMAC_HFR1);

	return (varMAC_HFR1 >> 0) & MAC_HFR1_RXFIFOSIZE_Mask;
}

u_int32_t get_hw_tx_fifo_size(void)
{
	u_int32_t varMAC_HFR1;

	MAC_HFR1_RgRd(varMAC_HFR1);

	return (varMAC_HFR1 >> 6) & MAC_HFR1_TXFIFOSIZE_Mask;
}

/*!
* \details This API will calculate per queue FIFO size.
*
* \param[in] fifo_size - total fifo size in h/w register
* \param[in] queue_count - total queue count
*
* \return returns integer
* \retval - fifo size per queue.
*/
static u_int32_t calculate_per_queue_fifo(u_int32_t fifo_size,
					u_int32_t queue_count)
{
	u_int32_t q_fifo_size = 0;	/* calculated fifo size per queue */
	u_int32_t p_fifo = eDWC_ETH_QOS_256; /* per queue fifo size programmable value */

	/* calculate Tx/Rx fifo share per queue */
	switch (fifo_size) {
	case 0:
		q_fifo_size = FIFO_SIZE_B(128);
		break;
	case 1:
		q_fifo_size = FIFO_SIZE_B(256);
		break;
	case 2:
		q_fifo_size = FIFO_SIZE_B(512);
		break;
	case 3:
		q_fifo_size = FIFO_SIZE_KB(1);
		break;
	case 4:
		q_fifo_size = FIFO_SIZE_KB(2);
		break;
	case 5:
		q_fifo_size = FIFO_SIZE_KB(4);
		break;
	case 6:
		q_fifo_size = FIFO_SIZE_KB(8);
		break;
	case 7:
		q_fifo_size = FIFO_SIZE_KB(16);
		break;
	case 8:
		q_fifo_size = FIFO_SIZE_KB(32);
		break;
	case 9:
		q_fifo_size = FIFO_SIZE_KB(64);
		break;
	case 10:
		q_fifo_size = FIFO_SIZE_KB(128);
		break;
	case 11:
		q_fifo_size = FIFO_SIZE_KB(256);
		break;
	}

	q_fifo_size = q_fifo_size/queue_count;

	if (q_fifo_size >= FIFO_SIZE_KB(32)) {
		p_fifo = eDWC_ETH_QOS_32k;
	} else if (q_fifo_size >= FIFO_SIZE_KB(16)) {
		p_fifo = eDWC_ETH_QOS_16k;
	} else if (q_fifo_size >= FIFO_SIZE_KB(8)) {
		p_fifo = eDWC_ETH_QOS_8k;
	} else if (q_fifo_size >= FIFO_SIZE_KB(4)) {
		p_fifo = eDWC_ETH_QOS_4k;
	} else if (q_fifo_size >= FIFO_SIZE_KB(2)) {
		p_fifo = eDWC_ETH_QOS_2k;
	} else if (q_fifo_size >= FIFO_SIZE_KB(1)) {
		p_fifo = eDWC_ETH_QOS_1k;
	} else if (q_fifo_size >= FIFO_SIZE_B(512)) {
		p_fifo = eDWC_ETH_QOS_512;
	} else if (q_fifo_size >= FIFO_SIZE_B(256)) {
		p_fifo = eDWC_ETH_QOS_256;
	}

	return p_fifo;
}

/* return zero on success and +ve error number on failure */
int configure_mtl_queue(struct DWC_ETH_QOS_prv_data *pdata,
				u_int8_t qInx)
{
	u_int32_t retryCount = 1000;
	u_int32_t vy_count;
	volatile u_int32_t varMTL_QTOMR;
	u_int32_t p_rx_fifo = eDWC_ETH_QOS_256, p_tx_fifo = eDWC_ETH_QOS_256;

	DBGPR("-->%s\n", __func__);

	/* Flush Tx Queue */
	MTL_QTOMR_FTQ_UdfWr(qInx, 0x1);
	vy_count = 0;
	while (1) {
		if (vy_count > retryCount) {
			return EBUSY;
		} else {
			vy_count++;
		}
		MTL_QTOMR_RgRd(qInx, varMTL_QTOMR);
		if (GET_VALUE(varMTL_QTOMR, MTL_QTOMR_FTQ_LPOS, MTL_QTOMR_FTQ_HPOS)
				== 0) {
			break;
		}
	}

	/* Enable Store and Forward mode for TX */
	MTL_QTOMR_TSF_UdfWr(qInx, 0x1);
	MTL_QTOMR_TXQEN_UdfWr(qInx, DWC_ETH_QOS_Q_GENERIC);
	/* Transmit Queue weight (required only if Queue is enable for DCB) */
	MTL_QW_ISCQW_UdfWr(qInx, (0x10 + qInx));

	//MTL_QROMR_FEP_UdfWr(qInx, 0x1);

	p_rx_fifo = calculate_per_queue_fifo(get_hw_rx_fifo_size(),
			pdata->rx_q_cnt);
	p_tx_fifo = calculate_per_queue_fifo(get_hw_tx_fifo_size(),
			pdata->tx_q_cnt);

	/* Transmit/Receive queue fifo size programmed */
	MTL_QROMR_RQS_UdfWr(qInx, p_rx_fifo);
	MTL_QTOMR_TQS_UdfWr(qInx, p_tx_fifo);
	DBGPR("Queue%d Tx fifo size %d, Rx fifo size %d\n",
		qInx, ((p_tx_fifo + 1) * 256), ((p_rx_fifo + 1) * 256));

	/* flow control will be used only if
	 * * each channel gets 8KB or more fifo */
	if (p_rx_fifo >= eDWC_ETH_QOS_4k) {
		/* Enable Rx FLOW CTRL in MTL and MAC
		 * Programming is valid only if Rx fifo size is greater than
		 * or equal to 8k
		 * */
		MTL_QROMR_EHFC_UdfWr(qInx, 0x1);

		/* Set Threshold for Activating Flow Contol space for min 2 frames
		 * ie, (1500 * 2) + (64 * 2) = 3128 bytes, rounding off to 4k
		 *
		 * Set Threshold for Deactivating Flow Contol for space of
		 * min 1 frame (frame size 1500bytes) in receive fifo */
		if (p_rx_fifo == eDWC_ETH_QOS_4k) {
			/* This violates the above formula because of FIFO size limit
			 * therefore overflow may occur inspite of this
			 * */
			MTL_QROMR_RFD_UdfWr(qInx, 0x2);
			MTL_QROMR_RFA_UdfWr(qInx, 0x1);
		}
		else if (p_rx_fifo == eDWC_ETH_QOS_8k) {
			MTL_QROMR_RFD_UdfWr(qInx, 0x4);
			MTL_QROMR_RFA_UdfWr(qInx, 0x2);
		}
		else if (p_rx_fifo == eDWC_ETH_QOS_16k) {
			MTL_QROMR_RFD_UdfWr(qInx, 0x5);
			MTL_QROMR_RFA_UdfWr(qInx, 0x2);
		}
		else if (p_rx_fifo == eDWC_ETH_QOS_32k) {
			MTL_QROMR_RFD_UdfWr(qInx, 0x7);
			MTL_QROMR_RFA_UdfWr(qInx, 0x2);
		}
	}

	DBGPR("<--%s\n", __func__);
}


/* return zero on success and +ve error number on failure */
int reset_mtl_queue(u_int8_t qInx)
{
	DBGPR("-->%s\n", __func__);

	MTL_QTOMR_RgWr(qInx, 0x0);
	MTL_QTOMR_TSF_UdfWr(qInx, 0x0);
	MTL_QTOMR_TXQEN_UdfWr(qInx, 0x0);
	MTL_QW_ISCQW_UdfWr(qInx, 0x0);
	MTL_QROMR_FEP_UdfWr(qInx, 0x0);
	MTL_QROMR_RQS_UdfWr(qInx, 0x0);
	MTL_QTOMR_TQS_UdfWr(qInx, 0x0);

	DBGPR("<--%s\n", __func__);
}


void configure_dma_channel(struct DWC_ETH_QOS_prv_data *pdata,
				u_int8_t qInx)
{
	unsigned int tmp;

	DBGPR("-->%s\n", __func__);

	/* Enable OSF mode */
	DMA_TCR_OSP_UdfWr(qInx, 0x1);

	/* select Rx buffer size = 2048bytes */
	DMA_RCR_RBSZ_UdfWr(qInx, 2048);

	/* disable RX watchdog timer */
	DMA_RIWTR_RWT_UdfWr(qInx, 0);

	/* clear all the interrupts which are set */
	DMA_SR_RgRd(qInx, tmp);
	DMA_SR_RgWr(qInx, tmp);

	/* Enable only normal interupt summary interrupt */
	DMA_IER_RgWr(qInx, 0);
	DMA_IER_NIE_UdfWr(qInx, 0x1);

	/* set PBLx8 */
	DMA_CR_PBLx8_UdfWr(qInx, 0x1);
	/* set TX PBL = 256 */
	DMA_TCR_PBL_UdfWr(qInx, 32);
	/* set RX PBL = 256 */
	DMA_RCR_PBL_UdfWr(qInx, 2);

	/* start TX DMA */
	DMA_TCR_ST_UdfWr(qInx, 0x1);
	/* start RX DMA */
	DMA_RCR_ST_UdfWr(qInx, 0x1);

	DBGPR("<--%s\n", __func__);
}


void reset_dma_channel(u_int8_t qInx)
{
	unsigned int tmp;

	DBGPR("-->%s\n", __func__);

	DMA_TCR_OSP_UdfWr(qInx, 0x0);
	DMA_RCR_RBSZ_UdfWr(qInx, 0x0);
	DMA_RIWTR_RWT_UdfWr(qInx, 0);
	
	/* clear all the interrupts which are set */
	DMA_SR_RgRd(qInx, tmp);
	DMA_SR_RgWr(qInx, tmp);

	DMA_IER_RgWr(qInx, 0);

	DMA_CR_PBLx8_UdfWr(qInx, 0x0);
	DMA_TCR_PBL_UdfWr(qInx, 0);
	DMA_RCR_PBL_UdfWr(qInx, 0);

	DMA_TDRLR_RgWr(qInx, 0x0);
	DMA_TDLAR_RgWr(qInx, 0x0);
	DMA_RDRLR_RgWr(qInx, 0x0);
	DMA_RDTP_RPDR_RgWr(qInx, 0x0);
	DMA_RDLAR_RgWr(qInx, 0x0);

	DBGPR("<--%s\n", __func__);
}


void configure_mac(struct DWC_ETH_QOS_prv_data *pdata, u_int8_t qInx)
{
	/* enable rx queue */
	MAC_RQC0R_RXQEN_UdfWr(qInx, DWC_ETH_QOS_Q_DCB); //TODO: check this

	/* set Pause Time */
	MAC_QTFCR_PT_UdfWr(qInx, 0xffff);
	
	/* Assign priority for RX flow control */
  /* Assign priority for TX flow control */
	switch(qInx) {
	case 0:
		MAC_TQPM0R_PSTQ0_UdfWr(0);
		MAC_RQC2R_PSRQ0_UdfWr(0x1 << qInx);
		break;
	case 1:
		MAC_TQPM0R_PSTQ1_UdfWr(1);
		MAC_RQC2R_PSRQ1_UdfWr(0x1 << qInx);
		break;
	case 2:
		MAC_TQPM0R_PSTQ2_UdfWr(2);
		MAC_RQC2R_PSRQ2_UdfWr(0x1 << qInx);
		break;
	case 3:
		MAC_TQPM0R_PSTQ3_UdfWr(3);
		MAC_RQC2R_PSRQ3_UdfWr(0x1 << qInx);
		break;
	case 4:
		MAC_TQPM1R_PSTQ4_UdfWr(4);
		MAC_RQC3R_PSRQ4_UdfWr(0x1 << qInx);
		break;
	case 5:
		MAC_TQPM1R_PSTQ5_UdfWr(5);
		MAC_RQC3R_PSRQ5_UdfWr(0x1 << qInx);
		break;
	case 6:
		MAC_TQPM1R_PSTQ6_UdfWr(6);
		MAC_RQC3R_PSRQ6_UdfWr(0x1 << qInx);
		break;
	case 7:
		MAC_TQPM1R_PSTQ7_UdfWr(7);
		MAC_RQC3R_PSRQ7_UdfWr(0x1 << qInx);
		break;
	}
	/* enable tx flow control */
	MAC_QTFCR_TFE_UdfWr(qInx, 1);

	/* Other MAC control register initialization happened as
	 * part of kmod driver
	 * */
}


void reset_mac(u_int8_t qInx)
{
	MAC_RQC0R_RXQEN_UdfWr(qInx, 0x0);
	MAC_QTFCR_PT_UdfWr(qInx, 0x0);
	MAC_QTFCR_TFE_UdfWr(qInx, 0);
}


/* return zero on success and +ve error number on failure */
int stop_dma_tx(u_int8_t qInx)
{
        unsigned int retryCount = 1000;
        unsigned int vy_count;
        volatile unsigned int varDMA_DSR0;
        volatile unsigned int varDMA_DSR1;
        volatile unsigned int varDMA_DSR2;

        /* issue Tx dma stop command */
        DMA_TCR_ST_UdfWr(qInx, 0);

        /* wait for Tx DMA to stop, ie wait till Tx DMA
         * goes in Suspend state or stopped state.
         */
        if (qInx == 0) {
                vy_count = 0;
                while(1){
                        if(vy_count > retryCount) {
                                printf("ERROR: Channel 0 stop failed, DSR0 = %#x\n",
                                                varDMA_DSR0);
                                return EBUSY;
                        }

                        DMA_DSR0_RgRd(varDMA_DSR0);
                        if ((GET_VALUE(varDMA_DSR0, DMA_DSR0_TPS0_LPOS, DMA_DSR0_TPS0_HPOS) == 0x6) ||
                                        (GET_VALUE(varDMA_DSR0, DMA_DSR0_TPS0_LPOS, DMA_DSR0_TPS0_HPOS) == 0x0)) {
                                break;
                        }
                        vy_count++;
                }
        } else if (qInx == 1) {
                vy_count = 0;
                while(1){
                        if(vy_count > retryCount) {
                                printf("ERROR: Channel 1 stop failed, DSR0 = %#x\n",
                                                varDMA_DSR0);
                                return EBUSY;
                        }

                        DMA_DSR0_RgRd(varDMA_DSR0);
                        if ((GET_VALUE(varDMA_DSR0, DMA_DSR0_TPS1_LPOS, DMA_DSR0_TPS1_HPOS) == 0x6) ||
                                        (GET_VALUE(varDMA_DSR0, DMA_DSR0_TPS1_LPOS, DMA_DSR0_TPS1_HPOS) == 0x0)) {
                                break;
                        }
                        vy_count++;
                }
        } else if (qInx == 2) {
                vy_count = 0;
                while(1){
                        if(vy_count > retryCount) {
                                printf("ERROR: Channel 2 stop failed, DSR0 = %#x\n",
                                                varDMA_DSR0);
                                return EBUSY;
                        }

                        DMA_DSR0_RgRd(varDMA_DSR0);
                        if ((GET_VALUE(varDMA_DSR0, DMA_DSR0_TPS2_LPOS, DMA_DSR0_TPS2_HPOS) == 0x6) ||
                                        (GET_VALUE(varDMA_DSR0, DMA_DSR0_TPS2_LPOS, DMA_DSR0_TPS2_HPOS) == 0x0)) {
                                break;
                        }
                        vy_count++;
                }
        } else if (qInx == 3) {
                vy_count = 0;
                while(1){
                        if(vy_count > retryCount) {
                                printf("ERROR: Channel 3 stop failed, DSR0 = %#x\n",
                                                varDMA_DSR1);
                                return EBUSY;
                        }

                        DMA_DSR1_RgRd(varDMA_DSR1);
                        if ((GET_VALUE(varDMA_DSR1, DMA_DSR1_TPS3_LPOS, DMA_DSR1_TPS3_HPOS) == 0x6) ||
                                        (GET_VALUE(varDMA_DSR1, DMA_DSR1_TPS3_LPOS, DMA_DSR1_TPS3_HPOS) == 0x0)) {
                                break;
                        }
                        vy_count++;
                }
        } else if (qInx == 4) {
                vy_count = 0;
                while(1){
                        if(vy_count > retryCount) {
                                printf("ERROR: Channel 4 stop failed, DSR0 = %#x\n",
                                                varDMA_DSR1);
                                return EBUSY;
                        }

                        DMA_DSR1_RgRd(varDMA_DSR1);
                        if ((GET_VALUE(varDMA_DSR1, DMA_DSR1_TPS4_LPOS, DMA_DSR1_TPS4_HPOS) == 0x6) ||
                                        (GET_VALUE(varDMA_DSR1, DMA_DSR1_TPS4_LPOS, DMA_DSR1_TPS4_HPOS) == 0x0)) {
                                break;
                        }
                        vy_count++;
                }
        } else if (qInx == 5) {
                vy_count = 0;
                while(1){
                        if(vy_count > retryCount) {
                                printf("ERROR: Channel 5 stop failed, DSR0 = %#x\n",
                                                varDMA_DSR1);
                                return EBUSY;
                        }

                        DMA_DSR1_RgRd(varDMA_DSR1);
                        if ((GET_VALUE(varDMA_DSR1, DMA_DSR1_TPS5_LPOS, DMA_DSR1_TPS5_HPOS) == 0x6) ||
                                        (GET_VALUE(varDMA_DSR1, DMA_DSR1_TPS5_LPOS, DMA_DSR1_TPS5_HPOS) == 0x0)) {
                                break;
                        }
                        vy_count++;
                }
        } else if (qInx == 6) {
                vy_count = 0;
                while(1){
                        if(vy_count > retryCount) {
                                printf("ERROR: Channel 6 stop failed, DSR0 = %#x\n",
                                                varDMA_DSR1);
                                return EBUSY;
                        }

                        DMA_DSR1_RgRd(varDMA_DSR1);
                        if ((GET_VALUE(varDMA_DSR1, DMA_DSR1_TPS6_LPOS, DMA_DSR1_TPS6_HPOS) == 0x6) ||
                                        (GET_VALUE(varDMA_DSR1, DMA_DSR1_TPS6_LPOS, DMA_DSR1_TPS6_HPOS) == 0x0)) {
                                break;
                        }
                        vy_count++;
                }
        } else if (qInx == 7) {
                vy_count = 0;
                while(1){
                        if(vy_count > retryCount) {
                                printf("ERROR: Channel 7 stop failed, DSR0 = %#x\n",
                                                varDMA_DSR2);
                                return EBUSY;
                        }

                        DMA_DSR2_RgRd(varDMA_DSR2);
                        if ((GET_VALUE(varDMA_DSR2, DMA_DSR2_TPS7_LPOS, DMA_DSR2_TPS7_HPOS) == 0x6) ||
                                        (GET_VALUE(varDMA_DSR2, DMA_DSR2_TPS7_LPOS, DMA_DSR2_TPS7_HPOS) == 0x0)) {
                                break;
                        }
                        vy_count++;
                }
        }

        return 0;
}


/* return zero on success and +ve error number on failure */
int stop_dma_rx(u_int8_t qInx)
{
        unsigned int retryCount = 1000;
        unsigned int vy_count;
        volatile unsigned int varDMA_DSR0;
        volatile unsigned int varDMA_DSR1;
        volatile unsigned int varDMA_DSR2;

        /* issue Rx dma stop command */
        DMA_RCR_ST_UdfWr(qInx, 0);

        /* wait for Rx DMA to stop, ie wait till Rx DMA
         * goes in either Running or Suspend state.
         * */
        if (qInx == 0) {
                vy_count = 0;
                while(1){
                        if(vy_count > retryCount) {
                                printf("ERROR: Rx Channel 0 stop failed, DSR0 = %#x\n",
                                                varDMA_DSR0);
                                return EBUSY;
                        }

                        DMA_DSR0_RgRd(varDMA_DSR0);
                        if ((GET_VALUE(varDMA_DSR0, DMA_DSR0_RPS0_LPOS, DMA_DSR0_RPS0_HPOS) == 0x3)
                                        || (GET_VALUE(varDMA_DSR0, DMA_DSR0_RPS0_LPOS, DMA_DSR0_RPS0_HPOS) == 0x4)
                                        || (GET_VALUE(varDMA_DSR0, DMA_DSR0_RPS0_LPOS, DMA_DSR0_RPS0_HPOS) == 0x0)) {
                                break;
                        }
                        vy_count++;
                }
        } else if (qInx == 1) {
                vy_count = 0;
                while(1){
                        if(vy_count > retryCount) {
                                printf("ERROR: Rx Channel 1 stop failed, DSR0 = %#x\n",
                                                varDMA_DSR0);
                                return EBUSY;
                        }

                        DMA_DSR0_RgRd(varDMA_DSR0);
                        if ((GET_VALUE(varDMA_DSR0, DMA_DSR0_RPS1_LPOS, DMA_DSR0_RPS1_HPOS) == 0x3)
                                        || (GET_VALUE(varDMA_DSR0, DMA_DSR0_RPS1_LPOS, DMA_DSR0_RPS1_HPOS) == 0x4)
                                        || (GET_VALUE(varDMA_DSR0, DMA_DSR0_RPS1_LPOS, DMA_DSR0_RPS1_HPOS) == 0x0)) {
                                break;
                        }
                        vy_count++;
                }
        } else if (qInx == 2) {
                vy_count = 0;
                while(1){
                        if(vy_count > retryCount) {
                                printf("ERROR: Rx Channel 2 stop failed, DSR0 = %#x\n",
                                                varDMA_DSR0);
                                return EBUSY;
                        }

                        DMA_DSR0_RgRd(varDMA_DSR0);
                        if ((GET_VALUE(varDMA_DSR0, DMA_DSR0_RPS2_LPOS, DMA_DSR0_RPS2_HPOS) == 0x3)
                                        || (GET_VALUE(varDMA_DSR0, DMA_DSR0_RPS2_LPOS, DMA_DSR0_RPS2_HPOS) == 0x4)
                                        || (GET_VALUE(varDMA_DSR0, DMA_DSR0_RPS2_LPOS, DMA_DSR0_RPS2_HPOS) == 0x0)) {
                                break;
                        }
                        vy_count++;
                }
        } else if (qInx == 3) {
                vy_count = 0;
                while(1){
                        if(vy_count > retryCount) {
                                printf("ERROR: Rx Channel 3 stop failed, DSR0 = %#x\n",
                                                varDMA_DSR1);
                                return EBUSY;
                        }

                        DMA_DSR1_RgRd(varDMA_DSR1);
                        if ((GET_VALUE(varDMA_DSR1, DMA_DSR1_RPS3_LPOS, DMA_DSR1_RPS3_HPOS) == 0x3)
                                        || (GET_VALUE(varDMA_DSR1, DMA_DSR1_RPS3_LPOS, DMA_DSR1_RPS3_HPOS) == 0x4)
                                        || (GET_VALUE(varDMA_DSR1, DMA_DSR1_RPS3_LPOS, DMA_DSR1_RPS3_HPOS) == 0x0)) {
                                break;
                        }
                        vy_count++;
                }
        } else if (qInx == 4) {
                vy_count = 0;
                while(1){
                        if(vy_count > retryCount) {
                                printf("ERROR: Rx Channel 4 stop failed, DSR0 = %#x\n",
                                                varDMA_DSR1);
                                return EBUSY;
                        }

                        DMA_DSR1_RgRd(varDMA_DSR1);
                        if ((GET_VALUE(varDMA_DSR1, DMA_DSR1_RPS4_LPOS, DMA_DSR1_RPS4_HPOS) == 0x3)
                                        || (GET_VALUE(varDMA_DSR1, DMA_DSR1_RPS4_LPOS, DMA_DSR1_RPS4_HPOS) == 0x4)
                                        || (GET_VALUE(varDMA_DSR1, DMA_DSR1_RPS4_LPOS, DMA_DSR1_RPS4_HPOS) == 0x0)) {
                                break;
                        }
                        vy_count++;
                }
        } else if (qInx == 5) {
                vy_count = 0;
                while(1){
                        if(vy_count > retryCount) {
                                printf("ERROR: Rx Channel 5 stop failed, DSR0 = %#x\n",
                                                varDMA_DSR1);
                                return EBUSY;
                        }

                        DMA_DSR1_RgRd(varDMA_DSR1);
                        if ((GET_VALUE(varDMA_DSR1, DMA_DSR1_RPS5_LPOS, DMA_DSR1_RPS5_HPOS) == 0x3)
                                        || (GET_VALUE(varDMA_DSR1, DMA_DSR1_RPS5_LPOS, DMA_DSR1_RPS5_HPOS) == 0x4)
                                        || (GET_VALUE(varDMA_DSR1, DMA_DSR1_RPS5_LPOS, DMA_DSR1_RPS5_HPOS) == 0x0)) {
                                break;
                        }
                        vy_count++;
                }
        } else if (qInx == 6) {
                vy_count = 0;
                while(1){
                        if(vy_count > retryCount) {
                                printf("ERROR: Rx Channel 6 stop failed, DSR0 = %#x\n",
                                                varDMA_DSR1);
                                return EBUSY;
                        }

                        DMA_DSR1_RgRd(varDMA_DSR1);
                        if ((GET_VALUE(varDMA_DSR1, DMA_DSR1_RPS6_LPOS, DMA_DSR1_RPS6_HPOS) == 0x3)
                                        || (GET_VALUE(varDMA_DSR1, DMA_DSR1_RPS6_LPOS, DMA_DSR1_RPS6_HPOS) == 0x4)
                                        || (GET_VALUE(varDMA_DSR1, DMA_DSR1_RPS6_LPOS, DMA_DSR1_RPS6_HPOS) == 0x0)) {
                                break;
                        }
                        vy_count++;
                }
        } else if (qInx == 7) {
                vy_count = 0;
                while(1){
                        if(vy_count > retryCount) {
                                printf("ERROR: Rx Channel 7 stop failed, DSR0 = %#x\n",
                                                varDMA_DSR2);
                                return EBUSY;
                        }

                        DMA_DSR2_RgRd(varDMA_DSR2);
                        if ((GET_VALUE(varDMA_DSR2, DMA_DSR2_RPS7_LPOS, DMA_DSR2_RPS7_HPOS) == 0x3)
                                        || (GET_VALUE(varDMA_DSR2, DMA_DSR2_RPS7_LPOS, DMA_DSR2_RPS7_HPOS) == 0x4)
                                        || (GET_VALUE(varDMA_DSR2, DMA_DSR2_RPS7_LPOS, DMA_DSR2_RPS7_HPOS) == 0x0)) {
                                break;
                        }
                        vy_count++;
                }
        }

        return 0;
}

unsigned long long get_wallclock()
{
  unsigned int timehigh, timelow;
  unsigned long long ns;

  MAC_STNSR_TSSS_UdfRd(timelow);
  MAC_STSR_RgRd(timehigh);

  ns = timelow + (timehigh * 1000000000ull);

  return ns;
}

void set_tx_queue_operating_mode(u_int8_t qInx, unsigned int q_mode)
{
  /* set tx in AVB mode */
  MTL_QTOMR_TXQEN_UdfWr(qInx, 0x1);
}

void set_avb_algorithm(u_int8_t qInx, unsigned char avb_algo)
{
  MTL_QECR_AVALG_UdfWr(qInx, avb_algo);
}

void config_credit_control(u_int8_t qInx, unsigned int cc)
{
  MTL_QECR_CC_UdfWr(qInx, cc);
}

void config_send_slope(u_int8_t qInx, unsigned int sendSlope)
{
  MTL_QSSCR_SSC_UdfWr(qInx, sendSlope);
}

void config_idle_slope(u_int8_t qInx, unsigned int idleSlope)
{
  MTL_QW_ISCQW_UdfWr(qInx, idleSlope);
}

void config_high_credit(u_int8_t qInx, unsigned int hiCredit)
{
  MTL_QHCR_HC_UdfWr(qInx, hiCredit);
}

void config_low_credit(u_int8_t qInx, unsigned int lowCredit)
{
  MTL_QLCR_LC_UdfWr(qInx, lowCredit);
}
