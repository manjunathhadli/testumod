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

/*!@file: DWC_ETH_QOS_eee.c
 * @brief: Driver functions.
 */
#include "DWC_ETH_QOS_yheader.h"

/* Clause 22 registers to access clause 45 register set */
#define MMD_CTRL_REG		0x0D	/* MMD Access Control Register */
#define MMD_ADDR_DATA_REG	0x0E	/* MMD Access Address Data Register */

/* MMD Access Control register fields */
#define MMD_CTRL_FUNC_ADDR		0x0000	/* address */
#define MMD_CTRL_FUNC_DATA_NOINCR	0x4000	/* data, no post increment */
#define MMD_CTRL_FUNC_DATA_INCR_ON_RDWT	0x8000	/* data, post increment on
						   reads & writes */
#define MMD_CTRL_FUNC_DATA_INCR_ON_WT	0xC000	/* data, post increment on
						   writes only */
/* Clause 45 expansion register */
#define CL45_PCS_EEE_ABLE 0x14	/* EEE Capability register */
#define CL45_ADV_EEE_REG 0x3C   /* EEE advertisement */
#define CL45_AN_EEE_LPABLE_REG	0x3D	/* EEE Link Partner ability reg */
#define CL45_CLK_STOP_EN_REG 0x0 /* Clock Stop enable reg */

/* Clause 45 expansion registers fields */
#define CL45_LP_ADV_EEE_STATS_1000BASE_T 0x0004	/* LP EEE capabilities
						   status */
#define CL45_CLK_STOP_EN	0x400 /* Enable xMII Clock Stop */


void DWC_ETH_QOS_enable_eee_mode(struct DWC_ETH_QOS_prv_data *pdata)
{
	struct DWC_ETH_QOS_tx_wrapper_descriptor *tx_desc_data = NULL;
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	int tx_idle = 0, qInx;

	printk(KERN_ALERT "-->DWC_ETH_QOS_enable_eee_mode\n");

	for (qInx = 0; qInx < pdata->tx_queue_cnt; qInx++) {
		tx_desc_data = GET_TX_WRAPPER_DESC(qInx);

		if ((tx_desc_data->dirty_tx == tx_desc_data->cur_tx) &&
			(pdata->tx_path_in_lpi_mode == false)) {
			tx_idle = 1;
		} else {
			tx_idle = 0;
			break;
		}
	}

	if (tx_idle)
		hw_if->set_eee_mode();

	printk(KERN_ALERT "<--DWC_ETH_QOS_enable_eee_mode\n");
}

void DWC_ETH_QOS_disable_eee_mode(struct DWC_ETH_QOS_prv_data *pdata)
{
	struct hw_if_struct *hw_if = &(pdata->hw_if);

	printk(KERN_ALERT "-->DWC_ETH_QOS_disable_eee_mode\n");

	hw_if->reset_eee_mode();
	del_timer_sync(&pdata->eee_ctrl_timer);
	pdata->tx_path_in_lpi_mode = false;

	printk(KERN_ALERT "-->DWC_ETH_QOS_disable_eee_mode\n");
}


/*!
* \brief API to control EEE mode.
*
* \details This function will move the MAC transmitter in LPI mode
* if there is no data transfer and MAC is not already in LPI state.
*
* \param[in] data - data hook
*
* \return void
*/

static void DWC_ETH_QOS_eee_ctrl_timer(unsigned long data)
{
	struct DWC_ETH_QOS_prv_data *pdata =
		(struct DWC_ETH_QOS_prv_data *)data;

	printk(KERN_ALERT "-->DWC_ETH_QOS_eee_ctrl_timer\n");

	DWC_ETH_QOS_enable_eee_mode(pdata);
	mod_timer(&pdata->eee_ctrl_timer,
		DWC_ETH_QOS_LPI_TIMER(DWC_ETH_QOS_DEFAULT_LPI_TIMER));

	printk(KERN_ALERT "<--DWC_ETH_QOS_eee_ctrl_timer\n");
}


static void DWC_ETH_QOS_mmd_phy_indirect(struct mii_bus *bus,
					 int regAddr,
					 int devAddr,
					 int phyAddr)
{
	/* Write the desired MMD devAddr */
	bus->write(bus, phyAddr, MMD_CTRL_REG, devAddr);

	/* Write the desired MMD regAddr */
	bus->write(bus, phyAddr, MMD_ADDR_DATA_REG, regAddr);

	/* Select the Function : DATA with no post increment */
	bus->write(bus, phyAddr, MMD_CTRL_REG,
		(devAddr | MMD_CTRL_FUNC_DATA_NOINCR));
}


/*!
* \brief API to read data from the MMD registers.
*
* \details This function will read data from the MMD(clause 45) registers
* using clause 22 registers. The procedure to read MMD registers is,
* 1. Write the desired MMD device addr into reg 13
* 2. Write the desired MMD reg addr into reg 14
* 3. Select the desired Function - MMD data command by writing in reg 13
* 4. Read the content of the MMD's selected reg through reg 14
*
* \param[in] bus - the target MII bus
* \param[in] regAddr - desired MMD reg addr to be read
* \param[in] devAddr - desired MMD address
* \param[in] phyAddr - PHY addr/id on the MII bus
*
* \return integer
*/
static int DWC_ETH_QOS_phy_read_mmd_indirect(struct mii_bus *bus,
					     int regAddr,
					     int devAddr,
					     int phyAddr)
{
	u32 ret;

	printk(KERN_ALERT "-->DWC_ETH_QOS_phy_read_mmd_indirect\n");

	DWC_ETH_QOS_mmd_phy_indirect(bus, regAddr, devAddr, phyAddr);
	/* read the content of the MMD's selected register */
	ret = bus->read(bus, phyAddr, MMD_ADDR_DATA_REG);

	printk(KERN_ALERT "<--DWC_ETH_QOS_phy_read_mmd_indirect\n");

	return ret;
}


/*!
* \brief API to write data into the MMD registers.
*
* \details This function will write data into MMD(clause 45) registers
* using clause 22 registers. The procedure to write MMD registers is,
* 1. Write the desired MMD device addr into reg 13
* 2. Write the desired MMD reg addr into reg 14
* 3. Select the desired Function - MMD data command by writing in reg 13
* 4. Write the data into MMD's selected reg through reg 14
*
* \param[in] bus - the target MII bus
* \param[in] regAddr - desired MMD reg addr to be written
* \param[in] devAddr - desired MMD address
* \param[in] phyAddr - PHY addr/id on the MII bus
* \param[in] data - data to write into the MMD register
*
* \return void
*/
static void DWC_ETH_QOS_phy_write_mmd_indirect(struct mii_bus *bus,
					     int regAddr,
					     int devAddr,
					     int phyAddr,
					     u32 data)
{
	printk(KERN_ALERT "-->DWC_ETH_QOS_phy_write_mmd_indirect\n");

	DWC_ETH_QOS_mmd_phy_indirect(bus, regAddr, devAddr, phyAddr);
	/* Write the data into MMD's selected register */
	bus->write(bus, phyAddr, MMD_ADDR_DATA_REG, data);

	printk(KERN_ALERT "<--DWC_ETH_QOS_phy_write_mmd_indirect\n");
}

#if 0

#define MDIO_EEE_100TX		0x0002	/* EEE is supported for 100BASE-TX */
#define MDIO_EEE_1000T		0x0004	/* EEE is supported for 1000BASE-T */
#define MDIO_EEE_10GT			0x0008	/* EEE is supported for 10GBASE-T */
#define MDIO_EEE_1000KX		0x0010	/* EEE is supported for 1000BASE-KX */
#define MDIO_EEE_10GKX4		0x0020	/* EEE is supported for 10GBASE-KX4 */
#define MDIO_EEE_10GKR		0x0040	/* EEE is supported for 10GBASE KR */

 /* A small helper function that translates MMD EEE Capability (3.20) bits
 * to ethtool supported settings.
 * */
static u32 DWC_ETH_QOS_mmd_eee_cap_to_ethtool_sup_t(u16 eee_cap)
{
	u32 supported = 0;

	if (eee_cap & MDIO_EEE_100TX)
		supported |= SUPPORTED_100baseT_Full;
	if (eee_cap & MDIO_EEE_1000T)
		supported |= SUPPORTED_1000baseT_Full;
	if (eee_cap & MDIO_EEE_10GT)
		supported |= SUPPORTED_10000baseT_Full;
	if (eee_cap & MDIO_EEE_1000KX)
		supported |= SUPPORTED_1000baseKX_Full;
	if (eee_cap & MDIO_EEE_10GKX4)
		supported |= SUPPORTED_10000baseKX4_Full;
	if (eee_cap & MDIO_EEE_10GKR)
		supported |= SUPPORTED_10000baseKR_Full;

	return supported;
}

 /* A small helper function that translates the MMD EEE Advertisment (7.60)
  * and MMD EEE Link Partner Ability (7.61) bits to ethtool advertisement
  * settings.
  * */
static inline u32 DWC_ETH_QOS_mmd_eee_adv_to_ethtool_adv_t(u16 eee_adv)
{
	u32 adv = 0;

	if (eee_adv & MDIO_EEE_100TX)
		adv |= ADVERTISED_100baseT_Full;
	if (eee_adv & MDIO_EEE_1000T)
		adv |= ADVERTISED_1000baseT_Full;
	if (eee_adv & MDIO_EEE_10GT)
		adv |= ADVERTISED_10000baseT_Full;
	if (eee_adv & MDIO_EEE_1000KX)
		adv |= ADVERTISED_1000baseKX_Full;
	if (eee_adv & MDIO_EEE_10GKX4)
		adv |= ADVERTISED_10000baseKX4_Full;
	if (eee_adv & MDIO_EEE_10GKR)
		adv |= ADVERTISED_10000baseKR_Full;

	return adv;
}
#endif

/*!
* \brief API to initialize and check EEE mode.
*
* \details This function checks if the EEE is supported by
* looking at the MMD registes and it also programs the MMD
* register 3.0 setting the "Clock stop enable" bit if required.
*
* \param[in] phydev - pointer to target phy_device structure
* \param[in] clk_stop_enable - PHY may stop the clock during LPI
*
* \return integer
*
* \retval zero if EEE is supported else return -ve number.
*/
static int DWC_ETH_QOS_phy_init_eee(struct phy_device *phydev,
		bool clk_stop_enable)
{
	int ret = -EPROTONOSUPPORT;

	DBGPR("-->DWC_ETH_QOS_phy_init_eee\n");

	/* According to 802.3az,the EEE is supported only in full duplex-mode.
	 * Also EEE feature is active when core is operating with MII, GMII
	 * or RGMII.
	 */
	if ((phydev->duplex == DUPLEX_FULL) &&
	    ((phydev->interface == PHY_INTERFACE_MODE_MII) ||
	    (phydev->interface == PHY_INTERFACE_MODE_GMII) ||
	    (phydev->interface == PHY_INTERFACE_MODE_RGMII))) {
		int eee_lp, eee_cap, eee_adv;
		//u32 cap,lp , adv;
		int status;//, idx;

		/* Read phy status to properly get the right settings */
		status = phy_read_status(phydev);
		if (status)
			return status;

		/* First check if the EEE ability is supported */
		eee_cap = DWC_ETH_QOS_phy_read_mmd_indirect(phydev->bus,
				CL45_PCS_EEE_ABLE, MDIO_MMD_PCS, phydev->addr);
		if (eee_cap < 0)
			return eee_cap;
/*
		cap = DWC_ETH_QOS_mmd_eee_cap_to_ethtool_sup_t(eee_cap);
		if (!cap)
			goto eee_exit;
*/
		/* check whether link Partner support EEE or not */
		eee_lp = DWC_ETH_QOS_phy_read_mmd_indirect(phydev->bus,
				CL45_AN_EEE_LPABLE_REG, MDIO_MMD_AN, phydev->addr);
		if (eee_lp < 0)
			return eee_lp;

		eee_adv = DWC_ETH_QOS_phy_read_mmd_indirect(phydev->bus,
				CL45_ADV_EEE_REG, MDIO_MMD_AN, phydev->addr);
		if (eee_adv < 0)
			return eee_adv;
/*
		//TODO:check this
		adv = DWC_ETH_QOS_mmd_eee_adv_to_ethtool_adv_t(eee_adv);
		lp = DWC_ETH_QOS_mmd_eee_adv_to_ethtool_adv_t(eee_lp);
		idx = phy_find_setting(phydev->speed, phydev->duplex);
		if ((lp & adv & settings[idx].setting))
			goto eee_exit;
*/
		if (clk_stop_enable) {
			/* Configure the PHY to stop receiving xMII
			 * clock while it is signaling LPI.
			 */
			int val = DWC_ETH_QOS_phy_read_mmd_indirect(phydev->bus,
					CL45_CLK_STOP_EN_REG, MDIO_MMD_PCS,
					phydev->addr);
			if (val < 0)
				return val;

			val |= CL45_CLK_STOP_EN;
			DWC_ETH_QOS_phy_write_mmd_indirect(phydev->bus,
					CL45_CLK_STOP_EN_REG, MDIO_MMD_PCS,
					phydev->addr, val);
		}

		ret = 0; /* EEE supported */
	}

	DBGPR("<--DWC_ETH_QOS_phy_init_eee\n");

//eee_exit:
	return ret;
}


/*!
* \brief API to initialize EEE mode.
*
* \details This function enables the LPI state and start the timer
* to verify whether the tx path can enter in LPI state if
* a. GMAC supports EEE mode &
* b. phy can also manage EEE.
*
* \param[in] pdata - pointer to private data structure
*
* \return bool
*
* \retval true on success & false on failure.
*/
bool DWC_ETH_QOS_eee_init(struct DWC_ETH_QOS_prv_data *pdata)
{
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	bool ret = false;

	printk(KERN_ALERT "-->DWC_ETH_QOS_eee_init\n");

	/* HW supports the EEE feature */
	if (pdata->hw_feat.eee_sel) {
		/* check if the PHY supports EEE */
		if (DWC_ETH_QOS_phy_init_eee(pdata->phydev, 1))
			goto phy_eee_failed;

		pdata->eee_active = 1;
		init_timer(&pdata->eee_ctrl_timer);
		pdata->eee_ctrl_timer.function = DWC_ETH_QOS_eee_ctrl_timer;
		pdata->eee_ctrl_timer.data = (unsigned long)pdata;
		pdata->eee_ctrl_timer.expires =
			DWC_ETH_QOS_LPI_TIMER(DWC_ETH_QOS_DEFAULT_LPI_TIMER);
		add_timer(&pdata->eee_ctrl_timer);

		hw_if->set_eee_timer(DWC_ETH_QOS_DEFAULT_LPI_LS_TIMER,
				DWC_ETH_QOS_DEFAULT_LPI_TWT_TIMER);
		printk(KERN_ALERT "EEE initialized\n");
		ret = true;
	}

	printk(KERN_ALERT "<--DWC_ETH_QOS_eee_init\n");

phy_eee_failed:
	return ret;
}

void DWC_ETH_QOS_eee_adjust_pls(struct DWC_ETH_QOS_prv_data *pdata)
{
	struct hw_if_struct *hw_if = &(pdata->hw_if);

	printk(KERN_ALERT "-->DWC_ETH_QOS_eee_adjust_pls\n");

	/* When EEE has been already initialized we have to modify
	 * the PLS bit in MAC_LPI_Control_Status reg according to
	 * PHY link status.
	 * */
	if (pdata->eee_enabled)
		hw_if->set_eee_pls(pdata->phydev->link);

	printk(KERN_ALERT "<--DWC_ETH_QOS_eee_adjust_pls\n");
}

#define MAC_LPS_TLPIEN 0x00000001
#define MAC_LPS_TLPIEX 0x00000002
#define MAC_LPS_RLPIEN 0x00000004
#define MAC_LPS_RLPIEX 0x00000008
void DWC_ETH_QOS_handle_eee_interrupt(struct DWC_ETH_QOS_prv_data *pdata)
{
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	u32 lpi_status;

	printk(KERN_ALERT "-->DWC_ETH_QOS_handle_eee_interrupt\n");

	lpi_status = hw_if->get_lpi_status();
	printk(KERN_ALERT "MAC_LPI_Control_Status = %#x\n", lpi_status);

	if (lpi_status & MAC_LPS_TLPIEN) {
		pdata->tx_path_in_lpi_mode = 1;
		printk(KERN_ALERT "MAC Transmitter has entered the LPI state\n");
	}

	if (lpi_status & MAC_LPS_TLPIEX) {
		pdata->tx_path_in_lpi_mode = 0;
		printk(KERN_ALERT "MAC Transmitter has exited the LPI state\n");
	}

	if (lpi_status & MAC_LPS_RLPIEN) {
		printk(KERN_ALERT "MAC Receiver has entered the LPI state\n");
	}

	if (lpi_status & MAC_LPS_RLPIEX) {
		printk(KERN_ALERT "MAC Receiver has exited the LPI state\n");
	}

	printk(KERN_ALERT "<--DWC_ETH_QOS_handle_eee_interrupt\n");
}
