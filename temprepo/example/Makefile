OPT=-O2
CFLAGS=$(OPT) -Wall -W -Wno-parentheses

CC=gcc
INCFLAGS=-I../umod/
LDLIBS=-lDWC_ETH_QOS_drv -lpci -lrt
LDFLAGS=-L../umod/

all: listener_talker listener talker

#compile both receiver and talket app
listener_talker: simple_listener_talker
simple_listener_talker: simple_listener_talker.o
simple_listener_talker.o: simple_listener_talker.c
	gcc -c -g $(INCFLAGS) $(CFLAGS) simple_listener_talker.c

#compile only receiver app
listener: simple_listener
simple_listener: simple_listener.o
simple_listener.o: simple_listener.c
	gcc -c -g $(INCFLAGS) $(CFLAGS) simple_listener.c

#compile only talker app
talker: simple_talker
simple_talker: simple_talker.o
simple_talker.o: simple_talker.c
	gcc -c -g $(INCFLAGS) $(CFLAGS) simple_talker.c

%: %.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

clean:
	rm -f `find . -name "*~" -o -name "*.[oa]" -o -name "\#*\#" -o -name TAGS -o -name core -o -name "*.orig"` simple_listener_talker simple_listener simple_talker
