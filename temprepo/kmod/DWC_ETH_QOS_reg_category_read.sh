# Shell Script to read form registers of a category.

#!/bin/bash
 
processLine() {
  read -r line
  until [ "$line" == "}" ]
  do
    cat /sys/kernel/debug/ddgen_DWC_ETH_QOS/$line 2> /dev/null
    if [ "$?" -ne 0 ]; then 
       echo    "$line : Invalid Register Name Specified For a Read Operation"
       echo -e "Or Check if the Register is Write Only\n"
    fi
  
    read -r line
  done
}

FILE="DWC_ETH_QOS_reg_config"
RES=1

# make sure file exist and readable
if [ ! -f $FILE ]; then
 echo "$FILE : does not exists"
 exit 1
elif [ ! -r $FILE ]; then
 echo "$FILE: can not read"
 exit 2
fi
# read $FILE using the file descriptors
 
# Set loop separator to end of line
BAKIFS=$IFS
IFS=$(echo -en "\n\b")
exec 3<&0
exec 0<"$FILE"
while read -r line
do
  if [ "$line" = "$1 {" ] ; then
    processLine
    RES=0
  fi
done
exec 0<&3
 
# restore $IFS which was used to determine what the field separators are
IFS=$BAKIFS
if [ "$RES" -ne 0 ]; then 
  echo "Invalid Register Category Name Specified For a Read Operation"
fi
exit $RES

