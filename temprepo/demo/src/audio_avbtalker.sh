
if [ $# -eq 1 ] ; then
        clear
	./microphone -p 64 -b 1024 -r 44100 | ./dwc_audio_talker $1 128

else
	echo ""
	echo "$0 <interface_name> "
	echo ""
	exit 1
fi
