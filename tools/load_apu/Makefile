
CROSS_CC := ../../bsp_pcw/bin/arm-none-linux-gnueabi-static/bin/arm-none-linux-gnueabi-gcc

.PHONY : clean all
all : clean load_apu

clean:
	-@rm *.o

%.o: %.c
	$(CROSS_CC) -c -Wall -o $@ $<

load_apu: load_apu.o mmio.o
	$(CROSS_CC) -static -o $@ $^
