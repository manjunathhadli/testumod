# Shell Script to read form all descriptors.

#function to read individual channel descriptors
read_single_ch_descriptors()
{
	#update the qInx
	echo "qInx $1" > /sys/kernel/debug/ddgen_DWC_ETH_QOS/qInx

	#read the descriptors
	cat /sys/kernel/debug/ddgen_DWC_ETH_QOS/RX_NORMAL_DESC_descriptor0			> ch$1_desc_dump
	cat /sys/kernel/debug/ddgen_DWC_ETH_QOS/RX_NORMAL_DESC_descriptor1			>> ch$1_desc_dump
	cat /sys/kernel/debug/ddgen_DWC_ETH_QOS/RX_NORMAL_DESC_descriptor2			>> ch$1_desc_dump
	cat /sys/kernel/debug/ddgen_DWC_ETH_QOS/RX_NORMAL_DESC_descriptor3			>> ch$1_desc_dump
  cat /sys/kernel/debug/ddgen_DWC_ETH_QOS/RX_NORMAL_DESC_descriptor4			>> ch$1_desc_dump
  cat /sys/kernel/debug/ddgen_DWC_ETH_QOS/RX_NORMAL_DESC_descriptor5			>> ch$1_desc_dump
  cat /sys/kernel/debug/ddgen_DWC_ETH_QOS/RX_NORMAL_DESC_descriptor6			>> ch$1_desc_dump
  cat /sys/kernel/debug/ddgen_DWC_ETH_QOS/RX_NORMAL_DESC_descriptor7			>> ch$1_desc_dump
  cat /sys/kernel/debug/ddgen_DWC_ETH_QOS/RX_NORMAL_DESC_descriptor8			>> ch$1_desc_dump
  cat /sys/kernel/debug/ddgen_DWC_ETH_QOS/RX_NORMAL_DESC_descriptor9			>> ch$1_desc_dump
  cat /sys/kernel/debug/ddgen_DWC_ETH_QOS/RX_NORMAL_DESC_descriptor10 		>> ch$1_desc_dump
  cat /sys/kernel/debug/ddgen_DWC_ETH_QOS/RX_NORMAL_DESC_descriptor11 		>> ch$1_desc_dump
  cat /sys/kernel/debug/ddgen_DWC_ETH_QOS/RX_NORMAL_DESC_descriptor12 		>> ch$1_desc_dump
  cat /sys/kernel/debug/ddgen_DWC_ETH_QOS/TX_NORMAL_DESC_descriptor0			>> ch$1_desc_dump
  cat /sys/kernel/debug/ddgen_DWC_ETH_QOS/TX_NORMAL_DESC_descriptor1			>> ch$1_desc_dump
  cat /sys/kernel/debug/ddgen_DWC_ETH_QOS/TX_NORMAL_DESC_descriptor2			>> ch$1_desc_dump
  cat /sys/kernel/debug/ddgen_DWC_ETH_QOS/TX_NORMAL_DESC_descriptor3			>> ch$1_desc_dump
  cat /sys/kernel/debug/ddgen_DWC_ETH_QOS/TX_NORMAL_DESC_descriptor4			>> ch$1_desc_dump
  cat /sys/kernel/debug/ddgen_DWC_ETH_QOS/TX_NORMAL_DESC_descriptor5			>> ch$1_desc_dump
  cat /sys/kernel/debug/ddgen_DWC_ETH_QOS/TX_NORMAL_DESC_descriptor6			>> ch$1_desc_dump
  cat /sys/kernel/debug/ddgen_DWC_ETH_QOS/TX_NORMAL_DESC_descriptor7			>> ch$1_desc_dump
  cat /sys/kernel/debug/ddgen_DWC_ETH_QOS/TX_NORMAL_DESC_descriptor8			>> ch$1_desc_dump
  cat /sys/kernel/debug/ddgen_DWC_ETH_QOS/TX_NORMAL_DESC_descriptor9			>> ch$1_desc_dump
  cat /sys/kernel/debug/ddgen_DWC_ETH_QOS/TX_NORMAL_DESC_descriptor10 		>> ch$1_desc_dump
  cat /sys/kernel/debug/ddgen_DWC_ETH_QOS/TX_NORMAL_DESC_descriptor11 		>> ch$1_desc_dump
  cat /sys/kernel/debug/ddgen_DWC_ETH_QOS/TX_NORMAL_DESC_descriptor12 		>> ch$1_desc_dump
}


if [ $# -ne 1 ]; then
	channel_count=0
	echo "Reading only channel 0"
else
	channel_count=$1
fi

i=0
retval=0
while [ $i -le $channel_count ]
do
		echo "qInx $i" > /sys/kernel/debug/ddgen_DWC_ETH_QOS/qInx
		retval=$?
		if [ $retval -eq 0 ]; then
			read_single_ch_descriptors $i
		else
			echo "qInx : $i is not present"
		fi

		i=$((i+1))
done

echo -e "\nDescriptor Read Completed for all Descriptors"
