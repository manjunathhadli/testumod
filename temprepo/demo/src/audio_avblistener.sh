
if [ $# -eq 1 ] ; then
        clear 
	./dwc_audio_listener $1 | ./speaker -p 64 -b 1024 -r 44100
 
else
	echo ""
	echo "$0 <interface_name> "
	echo ""
	exit 1
fi
