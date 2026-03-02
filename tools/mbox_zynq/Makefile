
CROSS_CC := ../../bsp_pcw/bin/arm-none-linux-gnueabi-static/bin/arm-none-linux-gnueabi-gcc
#CROSS_CC := /usr/bin/arm-none-linux-gnueabihf-gcc

.PHONY : all
all: mbox kr_hash mmaptest

mbox: mbox.c
	$(CROSS_CC) -static -Wall -o $@ $<

kr_hash: kr_hash.c
	$(CROSS_CC) -static -Wall -o $@ $<

mmaptest: mmaptest.c
	$(CROSS_CC) -static -Wall -o $@ $<
