
CROSS_CC := ../../bsp_pcw/bin/arm-none-linux-gnueabi-static/bin/arm-none-linux-gnueabi-gcc

.PHONY : all
all : mbox

mbox: mbox.c
	$(CROSS_CC) -static -Wall -o $@ $<

