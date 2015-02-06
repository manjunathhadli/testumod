if [ $# -eq 1 ] ; then
        clear 
        ./dwc_video_listener $1 2> /dev/null
 
else
	echo ""
	echo "$0 <interface_name> "
	echo ""
	exit 1
fi
