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

/*!@file: DWC_ETH_QOS_pci.c
 * @brief: Driver functions.
 */
#include "DWC_ETH_QOS_yheader.h"
#include "DWC_ETH_QOS_pci.h"

#ifdef CONFIG_TALKER
  #define MAC_ID {0, 0x55, 0x7b, 0xb5, 0x7d, 0xf9}
#elif CONFIG_LISTENER
  #define MAC_ID {0, 0x55, 0x7b, 0xb5, 0x7d, 0xf7}
#else
  #define MAC_ID {0, 0x55, 0x7b, 0xb5, 0x7d, 0xf3}
#endif

static UCHAR dev_addr[6] = MAC_ID;

static ULONG pci_base_addr;

void DWC_ETH_QOS_init_all_fptrs(struct DWC_ETH_QOS_prv_data *pdata)
{
	DWC_ETH_QOS_init_function_ptrs_dev(&pdata->hw_if);
	DWC_ETH_QOS_init_function_ptrs_desc(&pdata->desc_if);
}

static void DWC_ETH_QOS_init_all_base_addrs(void)
{
	DWC_ETH_QOS_dev_init_pci_base_addr(pci_base_addr);
	DWC_ETH_QOS_drv_init_pci_base_addr(pci_base_addr);

#ifdef DWC_ETH_QOS_CONFIG_DEBUGFS
	DWC_ETH_QOS_debug_init_pci_base_addr(pci_base_addr);
#endif

#ifdef DWC_ETH_QOS_CONFIG_PGTEST
	DWC_ETH_QOS_pg_init_pci_base_addr(pci_base_addr);
#endif
}


/*!
* \brief API to initialize the device.
*
* \details This probing function gets called (during execution of
* pci_register_driver() for already existing devices or later if a
* new device gets inserted) for all PCI devices which match the ID table
* and are not "owned" by the other drivers yet. This function gets passed
* a "struct pci_dev *" for each device whose entry in the ID table matches
* the device. The probe function returns zero when the driver chooses to take
* "ownership" of the device or an error code (negative number) otherwise.
* The probe function always gets called from process context, so it can sleep.
*
* \param[in] pdev - pointer to pci_dev structure.
* \param[in] id   - pointer to table of device ID/ID's the driver is inerested.
*
* \return integer
*
* \retval 0 on success & -ve number on failure.
*/

int DWC_ETH_QOS_probe(struct pci_dev *pdev,
				const struct pci_device_id *id)
{

	struct DWC_ETH_QOS_prv_data *pdata = NULL;
	struct net_device *dev = NULL;
	int i, ret = 0, bar_no;
	struct hw_if_struct *hw_if = NULL;
	struct desc_if_struct *desc_if = NULL;
	CHAR tx_q_count = 0, rx_q_count = 0;

	DBGPR("--> DWC_ETH_QOS_probe\n");

	ret = pci_enable_device(pdev);
	if (ret) {
		printk(KERN_ALERT "%s:Unable to enable device\n", DEV_NAME);
		goto err_out_enb_failed;
	}
	if (pci_request_regions(pdev, DEV_NAME)) {
		printk(KERN_ALERT "%s:Failed to get PCI regions\n", DEV_NAME);
		ret = -ENODEV;
		goto err_out_req_reg_failed;
	}
	pci_set_master(pdev);

	for (bar_no = 0; bar_no <= 5; bar_no++) {
		if (pci_resource_len(pdev, bar_no) == 0)
			continue;
		pci_base_addr = (ULONG) pci_iomap(pdev, bar_no, COMPLETE_BAR);
		if ((void __iomem *)pci_base_addr == NULL) {
			printk(KERN_ALERT
			       "%s: cannot map register memory, aborting",
			       pci_name(pdev));
			ret = -EIO;
			goto err_out_map_failed;
		}
		break;
	}

	DWC_ETH_QOS_init_all_base_addrs();

	DBGPR("pci_base_addr = %#lx\n", pci_base_addr);

	/* queue count */
	tx_q_count = get_tx_queue_count();
	rx_q_count = get_rx_queue_count();

#ifdef DWC_ETH_QOS_CONFIG_UMODEDRV
	tx_q_count = ((tx_q_count - DWC_ETH_QOS_MAX_AVB_Q_CNT) <= 0) ?
		(tx_q_count) : (tx_q_count - DWC_ETH_QOS_MAX_AVB_Q_CNT);

	printk(KERN_ALERT "total tx queue count = %d and\n"
	                  "tx queue reserved for AVB trafiic = %d\n"
			  "tx queue reserved for non-AVB trafiic = %d\n",
			  get_tx_queue_count(),
			  DWC_ETH_QOS_MAX_AVB_Q_CNT, tx_q_count);

	rx_q_count = ((rx_q_count - DWC_ETH_QOS_MAX_AVB_Q_CNT) <= 0) ?
		(rx_q_count) : (rx_q_count - DWC_ETH_QOS_MAX_AVB_Q_CNT);

	printk(KERN_ALERT "total rx queue count = %d and\n"
	                  "rx queue reserved for AVB trafiic = %d\n"
			  "rx queue reserved for non-AVB trafiic = %d\n",
			  get_rx_queue_count(),
			  DWC_ETH_QOS_MAX_AVB_Q_CNT, rx_q_count);
#endif /* end of DWC_ETH_QOS_CONFIG_UMODEDRV */

	dev = alloc_etherdev_mqs(sizeof(struct DWC_ETH_QOS_prv_data),
				tx_q_count, rx_q_count);
	if (dev == NULL) {
		printk(KERN_ALERT "%s:Unable to alloc new net device\n",
		    DEV_NAME);
		ret = -ENOMEM;
		goto err_out_dev_failed;
	}
	dev->dev_addr[0] = dev_addr[0];
	dev->dev_addr[1] = dev_addr[1];
	dev->dev_addr[2] = dev_addr[2];
	dev->dev_addr[3] = dev_addr[3];
	dev->dev_addr[4] = dev_addr[4];
	dev->dev_addr[5] = dev_addr[5];

	dev->base_addr = pci_base_addr;
	SET_NETDEV_DEV(dev, &pdev->dev);
	pdata = netdev_priv(dev);
	DWC_ETH_QOS_init_all_fptrs(pdata);
	hw_if = &(pdata->hw_if);
	desc_if = &(pdata->desc_if);

	pci_set_drvdata(pdev, dev);
	pdata->pdev = pdev;

	pdata->dev = dev;
	pdata->tx_queue_cnt = tx_q_count;
	pdata->max_tx_queue_cnt = get_tx_queue_count();
	pdata->rx_queue_cnt = rx_q_count;
	pdata->max_rx_queue_cnt = get_rx_queue_count();
#ifdef DWC_ETH_QOS_CONFIG_UMODEDRV
	pdata->tx_avb_q_idx = (pdata->max_tx_queue_cnt -
		DWC_ETH_QOS_MAX_AVB_Q_CNT);
	pdata->rx_avb_q_idx = (pdata->max_rx_queue_cnt -
		DWC_ETH_QOS_MAX_AVB_Q_CNT);
	pdata->bar_no = bar_no;
#endif /* end of DWC_ETH_QOS_CONFIG_UMODEDRV */

#ifdef DWC_ETH_QOS_CONFIG_DEBUGFS
	/* to give prv data to debugfs */
	DWC_ETH_QOS_get_pdata(pdata);
#endif

	/* issue software reset to device */
	hw_if->exit();
	dev->irq = pdev->irq;

	DWC_ETH_QOS_get_all_hw_features(pdata);
	DWC_ETH_QOS_print_all_hw_features(pdata);

	ret = desc_if->alloc_queue_struct(pdata);
	if (ret < 0) {
		printk(KERN_ALERT "ERROR: Unable to alloc Tx/Rx queue\n");
		goto err_out_q_alloc_failed;
	}

	dev->netdev_ops = DWC_ETH_QOS_get_netdev_ops();

	pdata->interface = DWC_ETH_QOS_get_phy_interface(pdata);
	if (1 == pdata->hw_feat.sma_sel) {
		ret = DWC_ETH_QOS_mdio_register(dev);
		if (ret < 0) {
			printk(KERN_ALERT "MDIO bus (id %d) registration failed\n",
			       pdata->bus_id);
			goto err_out_mdio_reg;
		}
	} else {
		printk(KERN_ALERT "%s: MDIO is not present\n\n", DEV_NAME);
	}

#ifndef DWC_ETH_QOS_CONFIG_PGTEST
	/* enabling and registration of irq with magic wakeup */
	if (1 == pdata->hw_feat.mgk_sel) {
		device_set_wakeup_capable(&pdev->dev, 1);
		pdata->wolopts = WAKE_MAGIC;
		enable_irq_wake(dev->irq);
	}

	for (i = 0; i < DWC_ETH_QOS_RX_QUEUE_CNT; i++) {
		struct DWC_ETH_QOS_rx_queue *rx_queue = GET_RX_QUEUE_PTR(i);

		netif_napi_add(dev, &rx_queue->napi, DWC_ETH_QOS_poll_mq, 64);
	}

	SET_ETHTOOL_OPS(dev, DWC_ETH_QOS_get_ethtool_ops());
	if (pdata->hw_feat.tso_en) {
		dev->hw_features = NETIF_F_TSO;
		dev->hw_features |= NETIF_F_SG;
		dev->hw_features |= NETIF_F_IP_CSUM;
		dev->hw_features |= NETIF_F_IPV6_CSUM;
		printk(KERN_ALERT "Supports TSO, SG and TX COE\n");
	}
	else if (pdata->hw_feat.tx_coe_sel) {
		dev->hw_features = NETIF_F_IP_CSUM ;
		dev->hw_features |= NETIF_F_IPV6_CSUM;
		printk(KERN_ALERT "Supports TX COE\n");
	}

	if (pdata->hw_feat.rx_coe_sel) {
		dev->hw_features |= NETIF_F_RXCSUM;
		printk(KERN_ALERT "Supports RX COE\n");
	}
#ifdef DWC_ETH_QOS_ENABLE_VLAN_TAG
	dev->vlan_features |= dev->hw_features;
	dev->hw_features |= NETIF_F_HW_VLAN_RX;
	if (pdata->hw_feat.sa_vlan_ins) {
		dev->hw_features |= NETIF_F_HW_VLAN_TX;
		printk(KERN_ALERT "VLAN Feature enabled\n");
	}
	if (pdata->hw_feat.vlan_hash_en) {
		dev->hw_features |= NETIF_F_HW_VLAN_FILTER;
		printk(KERN_ALERT "VLAN HASH Filtering enabled\n");
	}
#endif /* end of DWC_ETH_QOS_ENABLE_VLAN_TAG */
	dev->features |= dev->hw_features;
	pdata->dev_state |= dev->features;

#ifdef DWC_ETH_QOS_TIWDT
	DWC_ETH_QOS_init_tx_coalesce(pdata);
#endif /* end of DWC_ETH_QOS_TIWDT */

	DWC_ETH_QOS_init_rx_coalesce(pdata);
#endif /* end of DWC_ETH_QOS_CONFIG_PGTEST */

	spin_lock_init(&pdata->lock);
	spin_lock_init(&pdata->tx_lock);

#ifdef DWC_ETH_QOS_CONFIG_PGTEST
	ret = DWC_ETH_QOS_alloc_pg(pdata);
	if (ret < 0) {
		printk(KERN_ALERT "ERROR:Unable to allocate PG memory\n");
		goto err_out_pg_failed;
	}
	printk(KERN_ALERT "\n");
	printk(KERN_ALERT "/*******************************************\n");
	printk(KERN_ALERT "*\n");
	printk(KERN_ALERT "* PACKET GENERATOR MODULE ENABLED IN DRIVER\n");
	printk(KERN_ALERT "*\n");
	printk(KERN_ALERT "*******************************************/\n");
	printk(KERN_ALERT "\n");
#endif /* end of DWC_ETH_QOS_CONFIG_PGTEST */

#ifdef DWC_ETH_QOS_CONFIG_PTP
	DWC_ETH_QOS_ptp_init(pdata);
#endif	/* end of DWC_ETH_QOS_CONFIG_PTP */

	ret = register_netdev(dev);
	if (ret) {
		printk(KERN_ALERT "%s: Net device registration failed\n",
		    DEV_NAME);
		goto err_out_netdev_failed;
	}

#ifdef DWC_ETH_QOS_CONFIG_UMODEDRV
	pdata->ubuff = NULL;
	pdata->tx_avb_queue = 0;
	pdata->rx_avb_queue = 0;

	/* init semaphores and acquire/down it */
	for (i = 0; i < DWC_ETH_QOS_MAX_AVB_Q_CNT; i++)
		sema_init(&pdata->rx_sem[i], 0);
#endif

	DBGPR("<-- DWC_ETH_QOS_probe\n");

	if (pdata->hw_feat.pcs_sel) {
		netif_carrier_off(dev);
		printk(KERN_ALERT "carrier off till LINK is up\n");
	}

	return 0;

 err_out_netdev_failed:
#ifdef DWC_ETH_QOS_CONFIG_PTP
	DWC_ETH_QOS_ptp_remove(pdata);
#endif	/* end of DWC_ETH_QOS_CONFIG_PTP */

#ifdef DWC_ETH_QOS_CONFIG_PGTEST
	DWC_ETH_QOS_free_pg(pdata);
 err_out_pg_failed:
#endif
	if (1 == pdata->hw_feat.sma_sel)
		DWC_ETH_QOS_mdio_unregister(dev);

 err_out_mdio_reg:
	desc_if->free_queue_struct(pdata);

 err_out_q_alloc_failed:
	free_netdev(dev);
	pci_set_drvdata(pdev, NULL);

 err_out_dev_failed:
	pci_iounmap(pdev, (void __iomem *)pci_base_addr);

 err_out_map_failed:
	pci_release_regions(pdev);

 err_out_req_reg_failed:
	pci_disable_device(pdev);

 err_out_enb_failed:
	return ret;
}

/*!
* \brief API to release all the resources from the driver.
*
* \details The remove function gets called whenever a device being handled
* by this driver is removed (either during deregistration of the driver or
* when it is manually pulled out of a hot-pluggable slot). This function
* should reverse operations performed at probe time. The remove function
* always gets called from process context, so it can sleep.
*
* \param[in] pdev - pointer to pci_dev structure.
*
* \return void
*/

void DWC_ETH_QOS_remove(struct pci_dev *pdev)
{
	struct net_device *dev = pci_get_drvdata(pdev);
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct desc_if_struct *desc_if = &(pdata->desc_if);

	DBGPR("--> DWC_ETH_QOS_remove\n");

	if (pdata->irq_number != 0) {
		free_irq(pdata->irq_number, pdata);
		pdata->irq_number = 0;
	}

	if (1 == pdata->hw_feat.sma_sel)
		DWC_ETH_QOS_mdio_unregister(dev);

#ifdef DWC_ETH_QOS_TIWDT
	del_timer_sync(&pdata->tx_wd_timer);
#endif /* end of DWC_ETH_QOS_TIWDT */

#ifdef DWC_ETH_QOS_CONFIG_PTP
	DWC_ETH_QOS_ptp_remove(pdata);
#endif /* end of DWC_ETH_QOS_CONFIG_PTP */

	unregister_netdev(dev);

#ifdef DWC_ETH_QOS_CONFIG_PGTEST
	DWC_ETH_QOS_free_pg(pdata);
#endif /* end of DWC_ETH_QOS_CONFIG_PGTEST */

	desc_if->free_queue_struct(pdata);

	free_netdev(dev);

	pci_set_drvdata(pdev, NULL);
	pci_iounmap(pdev, (void __iomem *)pci_base_addr);

	pci_release_regions(pdev);
	pci_disable_device(pdev);

	DBGPR("<-- DWC_ETH_QOS_remove\n");

	return;
}

static struct pci_device_id DWC_ETH_QOS_id = {
	PCI_DEVICE(VENDOR_ID, DEVICE_ID)
};

struct pci_dev *DWC_ETH_QOS_pcidev;

static struct pci_driver DWC_ETH_QOS_pci_driver = {
	.name = "DWC_ETH_QOS",
	.id_table = &DWC_ETH_QOS_id,
	.probe = DWC_ETH_QOS_probe,
	.remove = DWC_ETH_QOS_remove,
	.shutdown = DWC_ETH_QOS_shutdown,
	.suspend_late = DWC_ETH_QOS_suspend_late,
	.resume_early = DWC_ETH_QOS_resume_early,
#ifdef CONFIG_PM
	.suspend = DWC_ETH_QOS_suspend,
	.resume = DWC_ETH_QOS_resume,
#endif
	.driver = {
		   .name = DEV_NAME,
		   .owner = THIS_MODULE,
	},
};

static void DWC_ETH_QOS_shutdown(struct pci_dev *pdev)
{
	printk(KERN_ALERT "-->DWC_ETH_QOS_shutdown\n");
	printk(KERN_ALERT "Handle the shutdown\n");
	printk(KERN_ALERT ">--DWC_ETH_QOS_shutdown\n");

	return;
}

static INT DWC_ETH_QOS_suspend_late(struct pci_dev *pdev, pm_message_t state)
{
	printk(KERN_ALERT "-->DWC_ETH_QOS_suspend_late\n");
	printk(KERN_ALERT "Handle the suspend_late\n");
	printk(KERN_ALERT "<--DWC_ETH_QOS_suspend_late\n");

	return 0;
}

static INT DWC_ETH_QOS_resume_early(struct pci_dev *pdev)
{
	printk(KERN_ALERT "-->DWC_ETH_QOS_resume_early\n");
	printk(KERN_ALERT "Handle the resume_early\n");
	printk(KERN_ALERT "<--DWC_ETH_QOS_resume_early\n");

	return 0;
}

#ifdef CONFIG_PM

/*!
 * \brief Routine to put the device in suspend mode
 *
 * \details This function gets called by PCI core when the device is being
 * suspended. The suspended state is passed as input argument to it.
 * Following operations are performed in this function,
 * - stop the phy.
 * - detach the device from stack.
 * - stop the queue.
 * - Disable napi.
 * - Stop DMA TX and RX process.
 * - Enable power down mode using PMT module or disable MAC TX and RX process.
 * - Save the pci state.
 *
 * \param[in] pdev – pointer to pci device structure.
 * \param[in] state – suspend state of device.
 *
 * \return int
 *
 * \retval 0
 */

static INT DWC_ETH_QOS_suspend(struct pci_dev *pdev, pm_message_t state)
{
	struct net_device *dev = pci_get_drvdata(pdev);
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	INT ret, pmt_flags = 0;
	unsigned int rwk_filter_values[] = {
		/* for filter 0 CRC is computed on 0 - 7 bytes from offset */
		0x000000ff,

		/* for filter 1 CRC is computed on 0 - 7 bytes from offset */
		0x000000ff,

		/* for filter 2 CRC is computed on 0 - 7 bytes from offset */
		0x000000ff,

		/* for filter 3 CRC is computed on 0 - 31 bytes from offset */
		0x000000ff,

		/* filter 0, 1 independently enabled and would apply for
		 * unicast packet only filter 3, 2 combined as,
		 * "Filter-3 pattern AND NOT Filter-2 pattern" */
		0x03050101,

		/* filter 3, 2, 1 and 0 offset is 50, 58, 66, 74 bytes
		 * from start */
		0x4a423a32,

		/* pattern for filter 1 and 0, "0x55", "11", repeated 8 times */
		0xe7b77eed,

		/* pattern for filter 3 and 4, "0x44", "33", repeated 8 times */
		0x9b8a5506,
	};

	DBGPR("-->DWC_ETH_QOS_suspend\n");

	if (!dev || !netif_running(dev) || (!pdata->hw_feat.mgk_sel &&
			!pdata->hw_feat.rwk_sel)) {
		DBGPR("<--DWC_ETH_QOS_dev_suspend\n");
		return -EINVAL;
	}

	if (pdata->hw_feat.rwk_sel && (pdata->wolopts & WAKE_UCAST)) {
		pmt_flags |= DWC_ETH_QOS_REMOTE_WAKEUP;
		hw_if->configure_rwk_filter(rwk_filter_values, 8);
	}

	if (pdata->hw_feat.mgk_sel && (pdata->wolopts & WAKE_MAGIC))
		pmt_flags |= DWC_ETH_QOS_MAGIC_WAKEUP;

	ret = DWC_ETH_QOS_powerdown(dev, pmt_flags, DWC_ETH_QOS_DRIVER_CONTEXT);
	pci_save_state(pdev);
	pci_set_power_state(pdev, pci_choose_state(pdev, state));

	DBGPR("<--DWC_ETH_QOS_suspend\n");

	return ret;
}

/*!
 * \brief Routine to resume device operation
 *
 * \details This function gets called by PCI core when the device is being
 * resumed. It is always called after suspend has been called. These function
 * reverse operations performed at suspend time. Following operations are
 * performed in this function,
 * - restores the saved pci power state.
 * - Wakeup the device using PMT module if supported.
 * - Starts the phy.
 * - Enable MAC and DMA TX and RX process.
 * - Attach the device to stack.
 * - Enable napi.
 * - Starts the queue.
 *
 * \param[in] pdev – pointer to pci device structure.
 *
 * \return int
 *
 * \retval 0
 */

static INT DWC_ETH_QOS_resume(struct pci_dev *pdev)
{
	struct net_device *dev = pci_get_drvdata(pdev);
	INT ret;

	DBGPR("-->DWC_ETH_QOS_resume\n");

	if (!dev || !netif_running(dev)) {
		DBGPR("<--DWC_ETH_QOS_dev_resume\n");
		return -EINVAL;
	}

	pci_set_power_state(pdev, PCI_D0);
	pci_restore_state(pdev);

	ret = DWC_ETH_QOS_powerup(dev, DWC_ETH_QOS_DRIVER_CONTEXT);

	DBGPR("<--DWC_ETH_QOS_resume\n");

	return ret;
}

#endif	/* CONFIG_PM */

#ifdef DWC_ETH_QOS_CONFIG_UMODEDRV
/* return 0 on success and -ve no. on failure */
static int DWC_ETH_QOS_notify_pdata(struct device *pdev, void *data)
{
	struct net_device *dev = dev_get_drvdata(pdev);
	struct DWC_ETH_QOS_prv_data *pdata = netdev_priv(dev);
	struct DWC_ETH_QOS_pdata_lookup *pdata_lookup =
		(struct DWC_ETH_QOS_pdata_lookup *)data;
	int ret;

	DBGPR_CHDRV("-->DWC_ETH_QOS_notify_pdata\n");

	/* look at pci string, if it is ours, update the adapter pointer */
	DBGPR_CHDRV("checking against adapter name %s\n",
		pci_name(pdata->pdev));
	if ((strncmp(pci_name(pdata->pdev), pdata_lookup->pdev_name,
		DWC_ETH_QOS_BIND_NAME_SIZE)) == 0) {
		pdata_lookup->pdata = pdata;
		ret = 0;
	} else {
		pdata_lookup->pdata = NULL;
		ret = -1;
	}

	DBGPR_CHDRV("<--DWC_ETH_QOS_notify_pdata\n");

	return ret;
}

/* return valid pdata on success and NULL on failure */
struct DWC_ETH_QOS_prv_data *DWC_ETH_QOS_search_pdata(char *name)
{
	struct DWC_ETH_QOS_pdata_lookup pdata_lookup;
	int ret;

	DBGPR_CHDRV("-->DWC_ETH_QOS_search_pdata\n");

	pdata_lookup.pdata = NULL;
	pdata_lookup.pdev_name = name;

	/* Iterate over the loaded interfaces and match
	 * the pci device ID identifier - eq "0000:7:0.0"
	 * */
	ret = driver_for_each_device(&DWC_ETH_QOS_pci_driver.driver,
				     NULL, &pdata_lookup,
				     DWC_ETH_QOS_notify_pdata);

	DBGPR_CHDRV("<--DWC_ETH_QOS_search_pdata\n");

	return (pdata_lookup.pdata);
}
#endif /* end of DWC_ETH_QOS_CONFIG_UMODEDRV */


/*!
* \brief API to register the driver.
*
* \details This is the first function called when the driver is loaded.
* It register the driver with PCI sub-system
*
* \return void.
*/

static int DWC_ETH_QOS_init_module(void)
{
	INT ret = 0;

	DBGPR("-->DWC_ETH_QOS_init_module\n");

	ret = pci_register_driver(&DWC_ETH_QOS_pci_driver);
	if (ret < 0) {
		printk(KERN_ALERT "DWC_ETH_QOS:driver registration failed");
		return ret;
	}

#ifdef DWC_ETH_QOS_CONFIG_DEBUGFS
	create_debug_files();
#endif

#ifdef DWC_ETH_QOS_CONFIG_UMODEDRV
	DWC_ETH_QOS_misc_register();
#endif

	DBGPR("<--DWC_ETH_QOS_init_module\n");

	return ret;
}

/*!
* \brief API to unregister the driver.
*
* \details This is the first function called when the driver is removed.
* It unregister the driver from PCI sub-system
*
* \return void.
*/

static void __exit DWC_ETH_QOS_exit_module(void)
{
	DBGPR("-->DWC_ETH_QOS_exit_module\n");

#ifdef DWC_ETH_QOS_CONFIG_DEBUGFS
	remove_debug_files();
#endif

#ifdef DWC_ETH_QOS_CONFIG_UMODEDRV
	DWC_ETH_QOS_misc_unregister();
#endif

	pci_unregister_driver(&DWC_ETH_QOS_pci_driver);

	DBGPR("<--DWC_ETH_QOS_exit_module\n");
}

/*!
* \brief Macro to register the driver registration function.
*
* \details A module always begin with either the init_module or the function
* you specify with module_init call. This is the entry function for modules;
* it tells the kernel what functionality the module provides and sets up the
* kernel to run the module's functions when they're needed. Once it does this,
* entry function returns and the module does nothing until the kernel wants
* to do something with the code that the module provides.
*/
module_init(DWC_ETH_QOS_init_module);

/*!
* \brief Macro to register the driver un-registration function.
*
* \details All modules end by calling either cleanup_module or the function
* you specify with the module_exit call. This is the exit function for modules;
* it undoes whatever entry function did. It unregisters the functionality
* that the entry function registered.
*/
module_exit(DWC_ETH_QOS_exit_module);

/*!
* \brief Macro to declare the module author.
*
* \details This macro is used to declare the module's authore.
*/
MODULE_AUTHOR("DDGen Vayavya Labs Pvt Ltd");

/*!
* \brief Macro to describe what the module does.
*
* \details This macro is used to describe what the module does.
*/
MODULE_DESCRIPTION("DWC_ETH_QOS Driver");

/*!
* \brief Macro to describe the module license.
*
* \details This macro is used to describe the module license.
*/
MODULE_LICENSE("GPL");
