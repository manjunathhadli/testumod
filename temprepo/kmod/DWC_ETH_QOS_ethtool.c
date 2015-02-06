/**********************************************************************
 *
 * Copyright (c) Vayavya Labs Pvt. Ltd. 2008
 *
 * THIS IS A DEVICE DRIVER GENERATED USING VAYAVYA LABS' DDGen TOOL.
 * THIS DRIVER WAS GENERATED FOR THE FOLLOWING SPECS:
 *
 * Device: DWC_ETH_QOS
 * Device Manufacturer: SYNOPSYS
 * Operating System: Linux
 *
 * DPS Reference ID: 5811729:880:146:648
 * RTS Reference ID: 63217:75:131078:0
 *
 * THE SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND. TO
 * THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, VAYAVYA LABS Pvt Ltd.
 * DISCLAIMS ALL WARRANTIES, EXPRESSED OR IMPLIED, INCLUDING, BUT NOT
 * LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE AND ANY WARRANTY AGAINST INFRINGEMENT, WITH REGARD
 * TO THE SOFTWARE.
 *
 * VAYAVYA LABS Pvt.Ltd. SHALL BE RELIEVED OF ANY AND ALL OBLIGATIONS
 * WITH RESPECT TO THIS SECTION FOR ANY PORTIONS OF THE SOFTWARE THAT
 * ARE REVISED, CHANGED, MODIFIED, OR MAINTAINED BY ANYONE OTHER THAN
 * VAYAVYA LABS Pvt.Ltd.
 *
 ***********************************************************************/

/*!@file: DWC_ETH_QOS_ethtool.c
 * @brief: Driver functions.
 */
#include "DWC_ETH_QOS_yheader.h"
#include "DWC_ETH_QOS_ethtool.h"

struct DWC_ETH_QOS_stats {
	char stat_string[ETH_GSTRING_LEN];
	int sizeof_stat;
	int stat_offset;
};

/* HW MAC Management counters (if supported) */
#define DWC_ETH_QOS_MMC_STAT(m)	\
	{ #m, FIELD_SIZEOF(struct DWC_ETH_QOS_mmc_counters, m),	\
	offsetof(struct DWC_ETH_QOS_prv_data, mmc.m)}

static const struct DWC_ETH_QOS_stats DWC_ETH_QOS_mmc[] = {
	/* MMC TX counters */
	DWC_ETH_QOS_MMC_STAT(mmc_tx_octetcount_gb),
	DWC_ETH_QOS_MMC_STAT(mmc_tx_framecount_gb),
	DWC_ETH_QOS_MMC_STAT(mmc_tx_broadcastframe_g),
	DWC_ETH_QOS_MMC_STAT(mmc_tx_multicastframe_g),
	DWC_ETH_QOS_MMC_STAT(mmc_tx_64_octets_gb),
	DWC_ETH_QOS_MMC_STAT(mmc_tx_65_to_127_octets_gb),
	DWC_ETH_QOS_MMC_STAT(mmc_tx_128_to_255_octets_gb),
	DWC_ETH_QOS_MMC_STAT(mmc_tx_256_to_511_octets_gb),
	DWC_ETH_QOS_MMC_STAT(mmc_tx_512_to_1023_octets_gb),
	DWC_ETH_QOS_MMC_STAT(mmc_tx_1024_to_max_octets_gb),
	DWC_ETH_QOS_MMC_STAT(mmc_tx_unicast_gb),
	DWC_ETH_QOS_MMC_STAT(mmc_tx_multicast_gb),
	DWC_ETH_QOS_MMC_STAT(mmc_tx_broadcast_gb),
	DWC_ETH_QOS_MMC_STAT(mmc_tx_underflow_error),
	DWC_ETH_QOS_MMC_STAT(mmc_tx_singlecol_g),
	DWC_ETH_QOS_MMC_STAT(mmc_tx_multicol_g),
	DWC_ETH_QOS_MMC_STAT(mmc_tx_deferred),
	DWC_ETH_QOS_MMC_STAT(mmc_tx_latecol),
	DWC_ETH_QOS_MMC_STAT(mmc_tx_exesscol),
	DWC_ETH_QOS_MMC_STAT(mmc_tx_carrier_error),
	DWC_ETH_QOS_MMC_STAT(mmc_tx_octetcount_g),
	DWC_ETH_QOS_MMC_STAT(mmc_tx_framecount_g),
	DWC_ETH_QOS_MMC_STAT(mmc_tx_excessdef),
	DWC_ETH_QOS_MMC_STAT(mmc_tx_pause_frame),
	DWC_ETH_QOS_MMC_STAT(mmc_tx_vlan_frame_g),

	/* MMC RX counters */
	DWC_ETH_QOS_MMC_STAT(mmc_rx_framecount_gb),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_octetcount_gb),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_octetcount_g),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_broadcastframe_g),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_multicastframe_g),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_crc_errror),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_align_error),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_run_error),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_jabber_error),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_undersize_g),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_oversize_g),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_64_octets_gb),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_65_to_127_octets_gb),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_128_to_255_octets_gb),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_256_to_511_octets_gb),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_512_to_1023_octets_gb),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_1024_to_max_octets_gb),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_unicast_g),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_length_error),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_outofrangetype),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_pause_frames),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_fifo_overflow),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_vlan_frames_gb),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_watchdog_error),

	/* IPC */
	DWC_ETH_QOS_MMC_STAT(mmc_rx_ipc_intr_mask),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_ipc_intr),

	/* IPv4 */
	DWC_ETH_QOS_MMC_STAT(mmc_rx_ipv4_gd),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_ipv4_hderr),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_ipv4_nopay),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_ipv4_frag),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_ipv4_udsbl),

	/* IPV6 */
	DWC_ETH_QOS_MMC_STAT(mmc_rx_ipv6_gd_octets),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_ipv6_hderr_octets),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_ipv6_nopay_octets),

	/* Protocols */
	DWC_ETH_QOS_MMC_STAT(mmc_rx_udp_gd),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_udp_err),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_tcp_gd),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_tcp_err),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_icmp_gd),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_icmp_err),

	/* IPv4 */
	DWC_ETH_QOS_MMC_STAT(mmc_rx_ipv4_gd_octets),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_ipv4_hderr_octets),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_ipv4_nopay_octets),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_ipv4_frag_octets),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_ipv4_udsbl_octets),

	/* IPV6 */
	DWC_ETH_QOS_MMC_STAT(mmc_rx_ipv6_gd),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_ipv6_hderr),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_ipv6_nopay),

	/* Protocols */
	DWC_ETH_QOS_MMC_STAT(mmc_rx_udp_gd_octets),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_udp_err_octets),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_tcp_gd_octets),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_tcp_err_octets),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_icmp_gd_octets),
	DWC_ETH_QOS_MMC_STAT(mmc_rx_icmp_err_octets),
};
#define DWC_ETH_QOS_MMC_STATS_LEN ARRAY_SIZE(DWC_ETH_QOS_mmc)

static const struct ethtool_ops DWC_ETH_QOS_ethtool_ops = {
	//.get_tso = DWC_ETH_QOS_get_tso,
	//.set_tso = DWC_ETH_QOS_set_tso,
	//.get_sg = ethtool_op_get_sg,
	//.set_sg = ethtool_op_set_sg,
	//.get_flags = ethtool_op_get_flags,
	.get_link = ethtool_op_get_link,
	.get_pauseparam = DWC_ETH_QOS_get_pauseparam,
	.set_pauseparam = DWC_ETH_QOS_set_pauseparam,
	.get_settings = DWC_ETH_QOS_getsettings,
	.set_settings = DWC_ETH_QOS_setsettings,
	//.get_wol = DWC_ETH_QOS_get_wol,
	//.set_wol = DWC_ETH_QOS_set_wol,
	.get_coalesce = DWC_ETH_QOS_get_coalesce,
	.set_coalesce = DWC_ETH_QOS_set_coalesce,
	.get_ethtool_stats = DWC_ETH_QOS_get_ethtool_stats,
	.get_strings = DWC_ETH_QOS_get_strings,
	.get_sset_count = DWC_ETH_QOS_get_sset_count,
	.get_ts_info = DWC_ETH_QOS_get_ts_info,
};

struct ethtool_ops *DWC_ETH_QOS_get_ethtool_ops(void)
{
	return (struct ethtool_ops *)&DWC_ETH_QOS_ethtool_ops;
}

/*!
 * \details This function is invoked by kernel when user request to get the
 * pause parameters through standard ethtool command.
 *
 * \param[in] dev – pointer to net device structure.
 * \param[in] Pause – pointer to ethtool_pauseparam structure.
 *
 * \return void
 */

static void DWC_ETH_QOS_get_pauseparam(struct net_device *dev,
				       struct ethtool_pauseparam *pause)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	struct phy_device *phydev = pdata->phydev;
	unsigned int data;

	DBGPR("-->DWC_ETH_QOS_get_pauseparam\n");

	pause->rx_pause = 0;
	pause->tx_pause = 0;

	if (pdata->hw_feat.pcs_sel) {
		pause->autoneg = 1;
		data = hw_if->get_an_adv_pause_param();
		if (!(data == 1) || !(data == 2))
			return;
	} else {
		pause->autoneg = pdata->phydev->autoneg;

		/* return if PHY doesn't support FLOW ctrl */
		if (!(phydev->supported & SUPPORTED_Pause) ||
		    !(phydev->supported & SUPPORTED_Asym_Pause))
			return;
	}

	if ((pdata->flow_ctrl & DWC_ETH_QOS_FLOW_CTRL_RX) ==
	    DWC_ETH_QOS_FLOW_CTRL_RX)
		pause->rx_pause = 1;

	if ((pdata->flow_ctrl & DWC_ETH_QOS_FLOW_CTRL_TX) ==
	    DWC_ETH_QOS_FLOW_CTRL_TX)
		pause->tx_pause = 1;

	DBGPR("<--DWC_ETH_QOS_get_pauseparam\n");
}

/*!
 * \details This function is invoked by kernel when user request to set the
 * pause parameters through standard ethtool command.
 *
 * \param[in] dev – pointer to net device structure.
 * \param[in] pause – pointer to ethtool_pauseparam structure.
 *
 * \return int
 *
 * \retval zero on success and -ve number on failure.
 */

static int DWC_ETH_QOS_set_pauseparam(struct net_device *dev,
				      struct ethtool_pauseparam *pause)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	struct phy_device *phydev = pdata->phydev;
	int new_pause = DWC_ETH_QOS_FLOW_CTRL_OFF;
	unsigned int data;
	int ret = 0;

	DBGPR("-->DWC_ETH_QOS_set_pauseparam: "\
	      "autoneg = %d tx_pause = %d rx_pause = %d\n",
	      pause->autoneg, pause->tx_pause, pause->rx_pause);

	/* return if PHY doesn't support FLOW ctrl */
	if (pdata->hw_feat.pcs_sel) {
		data = hw_if->get_an_adv_pause_param();
		if (!(data == 1) || !(data == 2))
			return -EINVAL;
	} else {
		if (!(phydev->supported & SUPPORTED_Pause) ||
			!(phydev->supported & SUPPORTED_Asym_Pause))
			return -EINVAL;
	}

	if (pause->rx_pause)
		new_pause |= DWC_ETH_QOS_FLOW_CTRL_RX;
	if (pause->tx_pause)
		new_pause |= DWC_ETH_QOS_FLOW_CTRL_TX;

	if (new_pause == pdata->flow_ctrl && !pause->autoneg)
		return -EINVAL;

	pdata->flow_ctrl = new_pause;

	if (pdata->hw_feat.pcs_sel) {
		DWC_ETH_QOS_configure_flow_ctrl(pdata);
	} else {
		phydev->autoneg = pause->autoneg;
		if (phydev->autoneg) {
			if (netif_running(dev))
				ret = phy_start_aneg(phydev);
		} else {
			DWC_ETH_QOS_configure_flow_ctrl(pdata);
		}
	}

	DBGPR("<--DWC_ETH_QOS_set_pauseparam\n");

	return ret;
}

void DWC_ETH_QOS_configure_flow_ctrl(struct DWC_ETH_QOS_prv_data *pdata)
{
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	UINT qInx;

	DBGPR("-->DWC_ETH_QOS_configure_flow_ctrl\n");

	if ((pdata->flow_ctrl & DWC_ETH_QOS_FLOW_CTRL_RX) ==
	    DWC_ETH_QOS_FLOW_CTRL_RX) {
		hw_if->enable_rx_flow_ctrl();
	} else {
		hw_if->disable_rx_flow_ctrl();
	}

	/* As ethtool does not provide queue level configuration
	   Tx flow control is disabled/enabled for all transmit queues */
	if ((pdata->flow_ctrl & DWC_ETH_QOS_FLOW_CTRL_TX) ==
	    DWC_ETH_QOS_FLOW_CTRL_TX) {
		for (qInx = 0; qInx < DWC_ETH_QOS_TX_QUEUE_CNT; qInx++)
			hw_if->enable_tx_flow_ctrl(qInx);
	} else {
		for (qInx = 0; qInx < DWC_ETH_QOS_TX_QUEUE_CNT; qInx++)
			hw_if->disable_tx_flow_ctrl(qInx);
	}

	pdata->oldflow_ctrl = pdata->flow_ctrl;

	DBGPR("<--DWC_ETH_QOS_configure_flow_ctrl\n");
}

/*!
 * \details This function is invoked by kernel when user request to get the
 * various device settings through standard ethtool command. This function
 * support to get the PHY related settings like link status, interface type,
 * auto-negotiation parameters and pause parameters etc.
 *
 * \param[in] dev – pointer to net device structure.
 * \param[in] cmd – pointer to ethtool_cmd structure.
 *
 * \return int
 *
 * \retval zero on success and -ve number on failure.
 */
#define SPEED_UNKNOWN -1
#define DUPLEX_UNKNOWN 0xff
static int DWC_ETH_QOS_getsettings(struct net_device *dev,
				   struct ethtool_cmd *cmd)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	unsigned int pause, duplex;
	unsigned int lp_pause, lp_duplex;
	int ret = 0;

	DBGPR("-->DWC_ETH_QOS_getsettings\n");

	if (pdata->hw_feat.pcs_sel) {
		if (!pdata->pcs_link) {
			ethtool_cmd_speed_set(cmd, SPEED_UNKNOWN);
			cmd->duplex = DUPLEX_UNKNOWN;
			return 0;
		}
		ethtool_cmd_speed_set(cmd, pdata->pcs_speed);
		cmd->duplex = pdata->pcs_duplex;

		pause = hw_if->get_an_adv_pause_param();
		duplex = hw_if->get_an_adv_duplex_param();
		lp_pause = hw_if->get_lp_an_adv_pause_param();
		lp_duplex = hw_if->get_lp_an_adv_duplex_param();

		if (pause == 1)
			cmd->advertising |= ADVERTISED_Pause;
		if (pause == 2)
			cmd->advertising |= ADVERTISED_Asym_Pause;
		/* MAC always supports Auto-negotiation */
		cmd->autoneg = ADVERTISED_Autoneg;
		cmd->supported |= SUPPORTED_Autoneg;
		cmd->advertising |= ADVERTISED_Autoneg;

		if (duplex) {
			cmd->supported |= (SUPPORTED_1000baseT_Full |
				SUPPORTED_100baseT_Full |
				SUPPORTED_10baseT_Full);
			cmd->advertising |= (ADVERTISED_1000baseT_Full |
				ADVERTISED_100baseT_Full |
				ADVERTISED_10baseT_Full);
		} else {
			cmd->supported |= (SUPPORTED_1000baseT_Half |
				SUPPORTED_100baseT_Half |
				SUPPORTED_10baseT_Half);
			cmd->advertising |= (ADVERTISED_1000baseT_Half |
				ADVERTISED_100baseT_Half |
				ADVERTISED_10baseT_Half);
		}

		/* link partner features */
		cmd->lp_advertising |= ADVERTISED_Autoneg;
		if (lp_pause == 1)
			cmd->lp_advertising |= ADVERTISED_Pause;
		if (lp_pause == 2)
			cmd->lp_advertising |= ADVERTISED_Asym_Pause;

		if (lp_duplex)
			cmd->lp_advertising |= (ADVERTISED_1000baseT_Full |
				ADVERTISED_100baseT_Full |
				ADVERTISED_10baseT_Full);
		else
			cmd->lp_advertising |= (ADVERTISED_1000baseT_Half |
				ADVERTISED_100baseT_Half |
				ADVERTISED_10baseT_Half);

		cmd->port = PORT_OTHER;
	} else {
		if (pdata->phydev == NULL) {
			printk(KERN_ALERT "%s: PHY is not registered\n", dev->name);
			return -ENODEV;
		}

		if (!netif_running(dev)) {
			printk(KERN_ALERT "%s: interface is disabled: we cannot track "\
			       "link speed / duplex settings\n", dev->name);
			return -EBUSY;
		}

		cmd->transceiver = XCVR_EXTERNAL;

		spin_lock_irq(&pdata->lock);
		ret = phy_ethtool_gset(pdata->phydev, cmd);
		spin_unlock_irq(&pdata->lock);
	}

	DBGPR("<--DWC_ETH_QOS_getsettings\n");

	return ret;
}

/*!
 * \details This function is invoked by kernel when user request to set the
 * various device settings through standard ethtool command. This function
 * support to set the PHY related settings like link status, interface type,
 * auto-negotiation parameters and pause parameters etc.
 *
 * \param[in] dev – pointer to net device structure.
 * \param[in] cmd – pointer to ethtool_cmd structure.
 *
 * \return int
 *
 * \retval zero on success and -ve number on failure.
 */

static int DWC_ETH_QOS_setsettings(struct net_device *dev,
				   struct ethtool_cmd *cmd)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	unsigned int speed;
	//unsigned int pause, duplex, speed;
	//unsigned int lp_pause, lp_duplex;
	int ret = 0;

	printk(KERN_ALERT "-->DWC_ETH_QOS_setsettings\n");

	if (pdata->hw_feat.pcs_sel) {
		speed = ethtool_cmd_speed(cmd);

		/* verify the settings we care about */
		if ((cmd->autoneg != AUTONEG_ENABLE) &&
			(cmd->autoneg != AUTONEG_DISABLE))
			return -EINVAL;
/*
		if ((cmd->autoneg == AUTONEG_ENABLE) &&
			(cmd->advertising == 0))
			return -EINVAL;
		if ((cmd->autoneg == AUTONEG_DISABLE) &&
			(speed != SPEED_1000 &&
			 speed != SPEED_100 &&
			 speed != SPEED_10) ||
			(cmd->duplex != DUPLEX_FULL &&
			 cmd->duplex != DUPLEX_HALF))
			 return -EINVAL;
*/
		spin_lock_irq(&pdata->lock);
		if (cmd->autoneg == AUTONEG_ENABLE)
			hw_if->control_an(1, 1);
		else
			hw_if->control_an(0, 0);
		spin_unlock_irq(&pdata->lock);
	} else {
		spin_lock_irq(&pdata->lock);
		ret = phy_ethtool_sset(pdata->phydev, cmd);
		spin_unlock_irq(&pdata->lock);
	}

	printk(KERN_ALERT "<--DWC_ETH_QOS_setsettings\n");

	return ret;
}

#if 0
/*!
 * \details This function is invoked by kernel when user request to get report
 * whether wake-on-lan is enable or not.
 *
 * \param[in] dev – pointer to net device structure.
 * \param[in] wol – pointer to ethtool_wolinfo structure.
 *
 * \return void
 */

static void DWC_ETH_QOS_get_wol(struct net_device *dev,
				struct ethtool_wolinfo *wol)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);

	DBGPR("-->DWC_ETH_QOS_get_wol\n");

	wol->supported = 0;
	spin_lock_irq(&pdata->lock);
	if (device_can_wakeup(&pdata->pdev->dev)) {
		if (pdata->hw_feat.mgk_sel)
			wol->supported |= WAKE_MAGIC;
		if (pdata->hw_feat.rwk_sel)
			wol->supported |= WAKE_UCAST;
		wol->wolopts = pdata->wolopts;
	}
	spin_unlock_irq(&pdata->lock);

	DBGPR("<--DWC_ETH_QOS_get_wol\n");

	return;
}

/*!
 * \details This function is invoked by kernel when user request to set
 * pmt parameters for remote wakeup or magic wakeup
 *
 * \param[in] dev – pointer to net device structure.
 * \param[in] wol – pointer to ethtool_wolinfo structure.
 *
 * \return int
 *
 * \retval zero on success and -ve number on failure.
 */

static int DWC_ETH_QOS_set_wol(struct net_device *dev,
			       struct ethtool_wolinfo *wol)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	u32 support = WAKE_MAGIC | WAKE_UCAST;
	int ret = 0;

	DBGPR("-->DWC_ETH_QOS_set_wol\n");

	/* By default almost all GMAC devices support the WoL via
	 * magic frame but we can disable it if the HW capability
	 * register shows no support for pmt_magic_frame. */
	if (!pdata->hw_feat.mgk_sel)
		wol->wolopts &= ~WAKE_MAGIC;
	if (!pdata->hw_feat.rwk_sel)
		wol->wolopts &= ~WAKE_UCAST;

	if (!device_can_wakeup(&pdata->pdev->dev))
		return -EINVAL;

	if (wol->wolopts & ~support)
		return -EINVAL;

	if (wol->wolopts) {
		printk(KERN_ALERT "Wakeup enable\n");
		device_set_wakeup_enable(&pdata->pdev->dev, 1);
		enable_irq_wake(pdata->irq_number);
	} else {
		printk(KERN_ALERT "Wakeup disable\n");
		device_set_wakeup_enable(&pdata->pdev->dev, 0);
		disable_irq_wake(pdata->irq_number);
	}

	spin_lock_irq(&pdata->lock);
	pdata->wolopts = wol->wolopts;
	spin_unlock_irq(&pdata->lock);

	DBGPR("<--DWC_ETH_QOS_set_wol\n");

	return ret;
}
#endif

u32 DWC_ETH_QOS_usec2riwt(u32 usec, struct DWC_ETH_QOS_prv_data *pdata)
{
	u32 ret = 0;

	DBGPR("-->DWC_ETH_QOS_usec2riwt\n");

	/* Eg:
	 * System clock is 62.5MHz, each clock cycle would then be 16ns
	 * For value 0x1 in watchdog timer, device would wait for 256
	 * clock cycles,
	 * ie, (16ns x 256) => 4.096us (rounding off to 4us)
	 * So formula with above values is,
	 * ret = usec/4 */

	ret = (usec * (DWC_ETH_QOS_SYSCLOCK/1000000))/256;

	DBGPR("<--DWC_ETH_QOS_usec2riwt\n");

	return ret;
}

static u32 DWC_ETH_QOS_riwt2usec(u32 riwt, struct DWC_ETH_QOS_prv_data *pdata)
{
	u32 ret = 0;

	DBGPR("-->DWC_ETH_QOS_riwt2usec\n");

	/* using formula from 'DWC_ETH_QOS_usec2riwt' */
	ret = (riwt * 256)/(DWC_ETH_QOS_SYSCLOCK/1000000);

	DBGPR("<--DWC_ETH_QOS_riwt2usec\n");

	return ret;
}

/*!
 * \details This function is invoked by kernel when user request to get
 * interrupt coalescing parameters. As coalescing parameters are same
 * for all the channels, so this function will get coalescing
 * details from channel zero and return.
 *
 * \param[in] dev – pointer to net device structure.
 * \param[in] wol – pointer to ethtool_coalesce structure.
 *
 * \return int
 *
 * \retval 0
 */

static int DWC_ETH_QOS_get_coalesce(struct net_device *dev,
				    struct ethtool_coalesce *ec)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct DWC_ETH_QOS_rx_wrapper_descriptor *rx_desc_data =
	    GET_RX_WRAPPER_DESC(0);
	DBGPR("-->DWC_ETH_QOS_get_coalesce\n");

	memset(ec, 0, sizeof(struct ethtool_coalesce));

#ifdef DWC_ETH_QOS_TIWDT
	ec->tx_coalesce_usecs = pdata->tx_coal_timer;
	ec->tx_max_coalesced_frames = pdata->tx_coal_frames;
#endif
	ec->rx_coalesce_usecs =
	    DWC_ETH_QOS_riwt2usec(rx_desc_data->rx_riwt, pdata);
	ec->rx_max_coalesced_frames = rx_desc_data->rx_coal_frames;

	DBGPR("<--DWC_ETH_QOS_get_coalesce\n");

	return 0;
}

/*!
 * \details This function is invoked by kernel when user request to set
 * interrupt coalescing parameters. This driver maintains same coalescing
 * parameters for all the channels, hence same changes will be applied to
 * all the channels.
 *
 * \param[in] dev – pointer to net device structure.
 * \param[in] wol – pointer to ethtool_coalesce structure.
 *
 * \return int
 *
 * \retval zero on success and -ve number on failure.
 */

static int DWC_ETH_QOS_set_coalesce(struct net_device *dev,
				    struct ethtool_coalesce *ec)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct DWC_ETH_QOS_rx_wrapper_descriptor *rx_desc_data =
	    GET_RX_WRAPPER_DESC(0);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	unsigned int rx_riwt, rx_usec, local_use_riwt, qInx;

	DBGPR("-->DWC_ETH_QOS_set_coalesce\n");

	/* Check for not supported parameters  */
	if ((ec->rx_coalesce_usecs_irq) ||
	    (ec->rx_max_coalesced_frames_irq) || (ec->tx_coalesce_usecs_irq) ||
	    (ec->use_adaptive_rx_coalesce) || (ec->use_adaptive_tx_coalesce) ||
	    (ec->pkt_rate_low) || (ec->rx_coalesce_usecs_low) ||
	    (ec->rx_max_coalesced_frames_low) || (ec->tx_coalesce_usecs_high) ||
	    (ec->tx_max_coalesced_frames_low) || (ec->pkt_rate_high) ||
	    (ec->tx_coalesce_usecs_low) || (ec->rx_coalesce_usecs_high) ||
	    (ec->rx_max_coalesced_frames_high) ||
	    (ec->tx_max_coalesced_frames_irq) ||
	    (ec->stats_block_coalesce_usecs) ||
	    (ec->tx_max_coalesced_frames_high) || (ec->rate_sample_interval) ||
	    (ec->tx_coalesce_usecs) || (ec->tx_max_coalesced_frames))
		return -EOPNOTSUPP;

	/* both rx_coalesce_usecs and rx_max_coalesced_frames should
	 * be > 0 in order for coalescing to be active.
	 * */
	if ((ec->rx_coalesce_usecs <= 3) && (ec->rx_max_coalesced_frames <= 1))
		local_use_riwt = 0;
	else
		local_use_riwt = 1;

	printk(KERN_ALERT "RX COALESCING is %s\n",
	       (local_use_riwt ? "ENABLED" : "DISABLED"));

#ifdef DWC_ETH_QOS_TIWDT
	/* No tx interrupts will be generated if both are zero
	 * TODO: device generates interrupt for all pkt if both of
	 * these values are zero, check it out.
	 * */
	if ((ec->tx_coalesce_usecs == 0) &&
	    (ec->tx_max_coalesced_frames == 0))
		return -EINVAL;

	if ((ec->tx_coalesce_usecs > DWC_ETH_QOS_COAL_TX_TIMER) ||
	    (ec->tx_max_coalesced_frames > DWC_ETH_QOS_TX_MAX_FRAMES))
		return -EINVAL;
#endif
	rx_riwt = DWC_ETH_QOS_usec2riwt(ec->rx_coalesce_usecs, pdata);

	/* Check the bounds of values for RX */
	if (rx_riwt > DWC_ETH_QOS_MAX_DMA_RIWT) {
		rx_usec = DWC_ETH_QOS_riwt2usec(DWC_ETH_QOS_MAX_DMA_RIWT,
		    pdata);
		printk(KERN_ALERT "RX Coalesing is limited to %d usecs\n",
		       rx_usec);
		return -EINVAL;
	}
	if (ec->rx_max_coalesced_frames > RX_DESC_CNT) {
		printk(KERN_ALERT "RX Coalesing is limited to %d frames\n",
		       DWC_ETH_QOS_RX_MAX_FRAMES);
		return -EINVAL;
	}
	if (rx_desc_data->rx_coal_frames != ec->rx_max_coalesced_frames
	    && netif_running(dev)) {
		printk(KERN_ALERT
		 "Coalesce frame parameter can be changed only if interface is down\n");
		return -EINVAL;
	}
	/* The selected parameters are applied to all the
	 * receive queues equally, so all the queue configurations
	 * are in sync */
	for (qInx = 0; qInx < DWC_ETH_QOS_RX_QUEUE_CNT; qInx++) {
		rx_desc_data = GET_RX_WRAPPER_DESC(qInx);
		rx_desc_data->use_riwt = 1;
		rx_desc_data->rx_riwt = rx_riwt;
		rx_desc_data->rx_coal_frames = ec->rx_max_coalesced_frames;
		hw_if->config_rx_watchdog(qInx, rx_desc_data->rx_riwt);
	}

#ifdef DWC_ETH_QOS_TIWDT
		pdata->tx_coal_frames = ec->tx_max_coalesced_frames;
		pdata->tx_coal_timer = ec->tx_coalesce_usecs;
#endif

	DBGPR("<--DWC_ETH_QOS_set_coalesce\n");

	return 0;
}


/*!
 * \details This function is invoked by kernel when user
 * requests to get the extended statistics about the device.
 *
 * \param[in] dev – pointer to net device structure.
 * \param[in] data – pointer in which extended statistics
 *                   should be put.
 *
 * \return void
 */

static void DWC_ETH_QOS_get_ethtool_stats(struct net_device *dev,
	struct ethtool_stats *dummy, u64 *data)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	int i, j = 0;

	DBGPR("-->DWC_ETH_QOS_get_ethtool_stats\n");

	if (pdata->hw_feat.mmc_sel) {
		DWC_ETH_QOS_mmc_read(&pdata->mmc);

		for ( i = 0; i < DWC_ETH_QOS_MMC_STATS_LEN; i++) {
			char *p;
			p = (char *)pdata + DWC_ETH_QOS_mmc[i].stat_offset;

			data[j++] = (DWC_ETH_QOS_mmc[i].sizeof_stat ==
				sizeof(u64)) ? (*(u64 *)p) : (*(u32 *)p);
		}
	}

	DBGPR("<--DWC_ETH_QOS_get_ethtool_stats\n");
}


/*!
 * \details This function returns a set of strings that describe
 * the requested objects.
 *
 * \param[in] dev – pointer to net device structure.
 * \param[in] data – pointer in which requested string should be put.
 *
 * \return void
 */

static void DWC_ETH_QOS_get_strings(struct net_device *dev, u32 stringset, u8 *data)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	int i;
	u8 *p = data;

	DBGPR("-->DWC_ETH_QOS_get_strings\n");

	switch(stringset) {
	case ETH_SS_STATS:
		if (pdata->hw_feat.mmc_sel) {
			for (i = 0; i < DWC_ETH_QOS_MMC_STATS_LEN; i++) {
				memcpy(p, DWC_ETH_QOS_mmc[i].stat_string,
					ETH_GSTRING_LEN);
				p += ETH_GSTRING_LEN;
			}
		}
		break;
	default:
		WARN_ON(1);
	}

	DBGPR("<--DWC_ETH_QOS_get_strings\n");
}


/*!
 * \details This function gets number of strings that @get_strings
 * will write.
 *
 * \param[in] dev – pointer to net device structure.
 *
 * \return int
 *
 * \retval +ve(>0) on success, 0 if that string is not
 * defined and -ve on failure.
 */

static int DWC_ETH_QOS_get_sset_count(struct net_device *dev, int sset)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	int len = 0;

	DBGPR("-->DWC_ETH_QOS_get_sset_count\n");

	switch(sset) {
	case ETH_SS_STATS:
		if (pdata->hw_feat.mmc_sel)
			len = DWC_ETH_QOS_MMC_STATS_LEN;
		break;
	default:
		len = -EOPNOTSUPP;
	}

	DBGPR("<--DWC_ETH_QOS_get_sset_count\n");

	return len;
}


/*!
 * \details This function gets timestamping and PTP HW clock capabilities.
 *
 * \param[in] dev – pointer to net device structure.
 * \param[in] info – pointer to PTP capabilities structure.
 *
 * \return int
 *
 * \retval 0 on success and non-zero on failure. 
 */
static int DWC_ETH_QOS_get_ts_info(struct net_device *dev,
		struct ethtool_ts_info *info)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);

	printk(KERN_ALERT "-->DWC_ETH_QOS_get_ts_info\n");


	info->so_timestamping = SOF_TIMESTAMPING_TX_SOFTWARE |
		SOF_TIMESTAMPING_RX_SOFTWARE |
		SOF_TIMESTAMPING_SOFTWARE;

	if (pdata->hw_feat.tsstssel) {
		info->so_timestamping |= SOF_TIMESTAMPING_TX_HARDWARE |
			SOF_TIMESTAMPING_RX_HARDWARE |
			SOF_TIMESTAMPING_RAW_HARDWARE;
	}


	if (pdata->ptp_clock)
		info->phc_index = ptp_clock_index(pdata->ptp_clock);
	else
		info->phc_index = -1;

	info->tx_types = (1 << HWTSTAMP_TX_OFF) | (1 << HWTSTAMP_TX_ON);

	info->rx_filters = ((1 << HWTSTAMP_FILTER_NONE) |
			(1 << HWTSTAMP_FILTER_PTP_V1_L4_EVENT) |
			(1 << HWTSTAMP_FILTER_PTP_V1_L4_SYNC) |
			(1 << HWTSTAMP_FILTER_PTP_V1_L4_DELAY_REQ) |
			(1 << HWTSTAMP_FILTER_PTP_V2_L4_EVENT) |
			(1 << HWTSTAMP_FILTER_PTP_V2_L4_SYNC) |
			(1 << HWTSTAMP_FILTER_PTP_V2_L4_DELAY_REQ) |
			(1 << HWTSTAMP_FILTER_PTP_V2_EVENT) |
			(1 << HWTSTAMP_FILTER_PTP_V2_SYNC) |
			(1 << HWTSTAMP_FILTER_PTP_V2_DELAY_REQ) |
			(1 << HWTSTAMP_FILTER_ALL));

	printk(KERN_ALERT "<--DWC_ETH_QOS_get_ts_info\n");

	return 0;
}

#if 0
/*!
 * \details This function is invoked by kernel when user
 * request to enable/disable tso feature.
 *
 * \param[in] dev – pointer to net device structure.
 * \param[in] data – 1/0 for enabling/disabling tso.
 *
 * \return int
 *
 * \retval 0 on success and -ve on failure.
 */

static int DWC_ETH_QOS_set_tso(struct net_device *dev, u32 data)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);

	DBGPR("-->DWC_ETH_QOS_set_tso\n");

	if (pdata->hw_feat.tso_en == 0)
			return -EOPNOTSUPP;

	if (data)
		dev->features |= NETIF_F_TSO;
	else
		dev->features &= ~NETIF_F_TSO;

	DBGPR("<--DWC_ETH_QOS_set_tso\n");

	return 0;
}


/*!
 * \details This function is invoked by kernel when user
 * request to get report whether tso feature is enabled/disabled.
 *
 * \param[in] dev – pointer to net device structure.
 *
 * \return unsigned int
 *
 * \retval  +ve no. on success and -ve no. on failure.
 */

static u32 DWC_ETH_QOS_get_tso(struct net_device *dev)
{
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);

	DBGPR("-->DWC_ETH_QOS_get_tso\n");

	if (pdata->hw_feat.tso_en == 0)
			return 0;

	DBGPR("<--DWC_ETH_QOS_get_tso\n");

	return ((dev->features & NETIF_F_TSO) != 0);
}
#endif
