#ifndef __DWC_ETH_QOS__PCI_H__

#define __DWC_ETH_QOS__PCI_H__

int DWC_ETH_QOS_probe(struct pci_dev *, const struct pci_device_id *);

void DWC_ETH_QOS_remove(struct pci_dev *);

static void DWC_ETH_QOS_shutdown(struct pci_dev *);

static INT DWC_ETH_QOS_suspend_late(struct pci_dev *, pm_message_t);

static INT DWC_ETH_QOS_resume_early(struct pci_dev *);

#ifdef CONFIG_PM
static INT DWC_ETH_QOS_suspend(struct pci_dev *, pm_message_t);

static INT DWC_ETH_QOS_resume(struct pci_dev *);
#endif				/* end of CONFIG_PM */

#endif
