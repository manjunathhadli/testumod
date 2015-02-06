
echo  ""
lsmod | grep ptp > /dev/null
if [ $? -eq 1 ]; then
  echo  "modprobe ptp\n";
  modprobe ptp
  if [ $? -ne 0 ]
  then
    echo  "insmod ptp.ko Failed.\n"
  fi
fi

if [ -f DWC_ETH_QOS.ko ]; then
  echo  "insmod DWC_ETH_QOS.ko\n";
  insmod DWC_ETH_QOS.ko
  if [ $? -ne 0 ]
  then
    echo  "insmod DWC_ETH_QOS.ko Failed.\n"
  fi
else
  echo "No DWC_ETH_QOS.ko file is present"
fi

