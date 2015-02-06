UMOD=../../umod/
LIB_DIR=../../../lib/
AVTP_HEADER=$(LIB_DIR)/avtp
SESSION_MNGR_HEADER=$(LIB_DIR)/session_manager

CFLAGS=-I $(AVTP_HEADER) -I $(SESSION_MNGR_HEADER) -I.
CFLAGS+=-L $(LIB_DIR)/build -lavtp -Wall `pkg-config --libs opencv`
EXTRA_FLAGS=-I $(UMOD)
EXTRA_FLAGS+=-L $(UMOD) -lDWC_ETH_QOS_drv -lpci -lrt -lpthread -lm -lpcap -ggdb `pkg-config --cflags opencv`

all:  microphone speaker dwc_audio_listener dwc_audio_talker dwc_video_talker dwc_video_listener sync_to_master_time non_avb_bandwidth_generator

microphone: microphone.c
	@echo ""
	@echo "------------------------- $< ----------------------------------------------"
	gcc -o microphone microphone.c -lasound

speaker: speaker.c
	@echo ""
	@echo "------------------------- $< ----------------------------------------------"
	gcc -o speaker speaker.c -lasound

dwc_audio_listener:dwc_audio_listener.c
	@echo ""
	@echo "------------------------- $< ----------------------------------------------"
	gcc -o dwc_audio_listener dwc_audio_listener.c $(CFLAGS) $(EXTRA_FLAGS)

dwc_audio_talker:dwc_audio_talker.c
	@echo ""
	@echo "------------------------- $< ----------------------------------------------"
	gcc -o dwc_audio_talker dwc_audio_talker.c $(CFLAGS) $(EXTRA_FLAGS)

dwc_video_talker:dwc_video_talker.cpp
	@echo ""
	@echo "------------------------- $< ----------------------------------------------"
	g++ -o dwc_video_talker dwc_video_talker.cpp $(CFLAGS) $(EXTRA_FLAGS)

dwc_video_listener:dwc_video_listener.cpp
	@echo ""
	@echo "------------------------- $< ----------------------------------------------"
	g++ -o dwc_video_listener dwc_video_listener.cpp $(CFLAGS) $(EXTRA_FLAGS)

sync_to_master_time:sync_to_master_time.c
	@echo ""
	@echo "------------------------- $< ----------------------------------------------"
	gcc -o sync_to_master_time sync_to_master_time.c $(CFLAGS) $(EXTRA_FLAGS)

non_avb_bandwidth_generator:non_avb_bandwidth_generator.c
	@echo ""
	@echo "------------------------- $< ----------------------------------------------"
	gcc -o non_avb_bandwidth_generator non_avb_bandwidth_generator.c -lpthread -lncurses

install:
	mv microphone speaker dwc_audio_listener dwc_audio_talker dwc_video_talker dwc_video_listener sync_to_master_time non_avb_bandwidth_generator ../build 
	cp audio_avblistener.sh audio_avbtalker.sh video_avblistener.sh video_avbtalker.sh ../build

clean:
	rm -f ../build/*

