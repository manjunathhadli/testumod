OBJS=DWC_ETH_QOS_drv.o
OBJS1=DWC_ETH_QOS_dev.o
INCL=DWC_ETH_QOS_yheader.h DWC_ETH_QOS_yregacc.h DWC_ETH_QOS_ycommon.h DWC_ETH_QOS_dev.h
AVBLIB=libDWC_ETH_QOS_drv.a

all: $(AVBLIB)

$(AVBLIB):$(OBJS) $(OBJS1)
	rm -f $@
	ar -cvq $@ $(OBJS) $(OBJS1)

$(OBJS): DWC_ETH_QOS_drv.c $(INCL)
	gcc -c DWC_ETH_QOS_drv.c

$(OBJS1): DWC_ETH_QOS_dev.c $(INCL)
	gcc -c DWC_ETH_QOS_dev.c

clean:
	rm -f `find . -name "*~" -o -name "*.[oa]" -o -name "\#*\#" -o -name TAGS -o -name core -o -name "*.orig"`
