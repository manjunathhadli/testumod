# Shell Script to write into an register.

if [ $# -ne 2 ]; then 
  echo -e "\nPlease specify Arguments Correctly"
  echo    "-----------------------------------------------------------------------"
  echo    " USAGE : ./DWC_ETH_QOS_reg_write.sh <register_name> <value>" 
  echo    " Before doing read or write operation mount the debugfs on your system." 
  echo -e "------------------------------------------------------------------------\n"
else 
  echo "$1 $2" > /sys/kernel/debug/ddgen_DWC_ETH_QOS/$1 
  if [ "$?" -ne 0 ]; then 
    echo -e "\nInvalid Register Specified For a Write Operation"
    echo -e  "Or Check if the Register is Read Only!"
    echo -e  "Or Check if the debugfs is mounted or not!"
  else 
    echo -e "\nRegister Write Completed for $1 \n"
		./DWC_ETH_QOS_reg_read.sh $1
  fi
fi

