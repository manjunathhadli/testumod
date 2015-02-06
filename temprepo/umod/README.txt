This is the README.txt for DWC_ETH_QOS (Synopsys Ethernet 4.0) user mode driver.
The driver consists of the following files:

DWC_ETH_QOS_dev.c DWC_ETH_QOS_dev.h
  Contains all the functions that device specific.
  This will contain functions relevant to descriptor
  programming also.

DWC_ETH_QOS_drv.c
  This C file contain all the APIs that are registered with
	the network stack.

DWC_ETH_QOS_yheader.h
  This header file contains all the datatype definitions and
	macros (other than register access macros). This file also
	contains prototype declarations of non static functions used
	across files.

DWC_ETH_QOS_yregacc.h
  This header file contains all the macros for accessing all
	registers, register fields and descriptor fields

DWC_ETH_QOS_ycommon.h
  This header file contains all the structure definitions and
	macros (other than register access macros). This file also
	contains prototype declarations of non static functions used
	across files. These are used in kernel driver and user level

Makefile
  This is the Makefile to compile and link the driver

NOTES:
0: This driver has been compiled agains Linux Kernel 3.8.0-25-lowlatency

