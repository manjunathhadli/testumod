# Shell Script to read form indiviual or all registers.

if [ $# -lt 1 ]; then 
  echo -e "\nPlease specify arguments correctly"
  echo     "-----------------------------------------------------------------------"
  echo     " USAGE : ./DWC_ETH_QOS_reg_read.sh <register_name> "
  echo     "     OR: ./DWC_ETH_QOS_reg_read.sh -type <category_name>"
  echo     " Before doing read or write operation mount the debugfs on your system." 
  echo -e  "------------------------------------------------------------------------\n"
else 
  case $1 in
          all) 
            cat /sys/kernel/debug/ddgen_DWC_ETH_QOS/registers
            echo -e "\nRegister Reads Completed for all Registers\n"
            ;;
          -type)
            if [ $# -lt 2 ]; then 
              echo -e "\nPlease specify arguments correctly"
              echo    "------------------------------------------------------------"
              echo    " USAGE : ./DWC_ETH_QOS_reg_read.sh -type <category_name>"
              echo -e "------------------------------------------------------------\n"
            else
              ./DWC_ETH_QOS_reg_category_read.sh $2
            fi
            ;;
          *) 
            cat /sys/kernel/debug/ddgen_DWC_ETH_QOS/$1 2> /dev/null
            if [ "$?" -ne 0 ]; then 
               echo -e "\nInvalid Register Specified For a Read Operation"
               echo -e "Or Check if the Register is Write Only!"
               echo -e "Or Check if the debugfs is mounted or not!"
            else 
              echo -e "Register Read Completed for $1 \n"
            fi
            ;;
  esac
fi

