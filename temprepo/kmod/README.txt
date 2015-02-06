This is the README.txt for DWC_ETH_QOS (Synopsys Ethernet 4.0) driver.
The driver consists of the following files:

DWC_ETH_QOS_dev.c
        Contains all the functions that device specific.
        This will contain functions relevant to descriptor
        programming also.

DWC_ETH_QOS_avb.c DWC_ETH_QOS_avb.h
        This C file contains the avb supported APIs and structures
        which is common across user mode driver.

DWC_ETH_QOS_desc.c DWC_ETH_QOS_desc.h
        This C file contains the memory management functions
        for descriptors and the wrapper functions for descriptor
	initializing/resetting. This file also contains functions
	necessary for handling other private data structure member
	elements related to descriptor management.

DWC_ETH_QOS_drv.c DWC_ETH_QOS_drv.h
  This C file contain all the APIs that are registered with
	the network stack.

DWC_ETH_QOS_mdio.c
  This C file contains all the APIs that are used for Read/Write
  related to Phy.
 
DWC_ETH_QOS_pci.c DWC_ETH_QOS_pci.h
  This C file contains all the APIs that are registered with
	the PCI subsystem.

DWC_ETH_QOS_ethtool.c DWC_ETH_QOS_ethtool.h
  This C file contains the ethtool functions that have been
	implemented

DWC_ETH_QOS_yheader.h
  This header file contains all the datatype definitions and
	macros (other than register access macros). This file also
	contains prototype declarations of non static functions used
	across files.

DWC_ETH_QOS_yregacc.h
  This header file contains all the macros for accessing all
	registers, register fields and descriptor fields

DWC_ETH_QOS_debug_operation.c
  This C file contains the necessary code for setting up
	debug fs.

DWC_ETH_QOS_reg_config
	This is a text file for configuring various register groups
	and is used by the four shell scripts below.


DWC_ETH_QOS_descriptors_read.sh
DWC_ETH_QOS_reg_category_read.sh
DWC_ETH_QOS_reg_read.sh
DWC_ETH_QOS_reg_write.sh
  These shell scripts can be sued to read/write to all registers
	and read all descriptors

Makefile
  This is the Makefile to compile and link the driver

load_module.sh
  Script to insert the driver module along with dependent modules.

build.sh
  Script to compile the driver along with ptp , user mode driver,
  debug infrastructure and talker/listener=y.

remove.sh
  script to remove the kernel driver module

map_rxqueues.sh
  Script to map the rx queues based on the vlan id tag.
