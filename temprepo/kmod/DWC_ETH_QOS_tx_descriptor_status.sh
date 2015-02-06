# Shell Script to read form all descriptors.

read_single_ch_desc_status()
{
	#update the qInx
	echo "qInx $1" > /sys/kernel/debug/ddgen_DWC_ETH_QOS/qInx

	#read desc status
  cat /sys/kernel/debug/ddgen_DWC_ETH_QOS/TX_NORMAL_DESC_STATUS
}

if [ $# -lt 1 ]; then 
  echo -e "\nPlease specify Arguments Correctly"
  echo    "-----------------------------------------------------------------------------------"
  echo -e "USAGE:\n"
	echo -e "./DWC_ETH_QOS_tx_descriptors_status.sh <channel_no> [to read individaul DMA channel]"
	echo -e "\n"
	echo -e "./DWC_ETH_QOS_tx_descriptors_status.sh all [to read all DMA channel]"
  echo -e "-----------------------------------------------------------------------------------\n"
else 
	case $1 in
		all)
			for i in 0 1 2 3  4 5 6 7
			do
				read_single_ch_desc_status $i
			done
			;;
	
			*)
				read_single_ch_desc_status $i
			;;
			esac
fi


