
CROSS_CC := ../../bsp_pcw/bin/arm-none-linux-gnueabi-static/bin/arm-none-linux-gnueabi-gcc

.PHONY : all
all : devmem2 devmem2hd

devmem2: devmem2.c
	$(CROSS_CC) -static -o $@ $<

devmem2hd: devmem2hd.c
	$(CROSS_CC) -static -o $@ $<

