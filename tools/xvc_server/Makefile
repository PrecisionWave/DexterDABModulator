
CROSS_CC := ../../bsp_pcw/bin/arm-none-linux-gnueabi-static/bin/arm-none-linux-gnueabi-gcc

.PHONY : all
all : xvcServer_mmap

xvcServer_mmap: xvcServer.c
	$(CROSS_CC) -static -o $@ $<

