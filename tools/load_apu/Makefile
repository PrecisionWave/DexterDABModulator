
CROSS_CC := ../../bsp_pcw/bin/arm-none-linux-gnueabi-static/bin/arm-none-linux-gnueabi-gcc

.PHONY : clean all
all : clean load_apu

clean:
	-@rm *.o

%.o: %.c
	$(CROSS_CC) -c -Wall -march=armv7 -o $@ $<

load_apu: load_apu.o mmio.o mmap.o memorymap.o load_elf.o load_elf_mb.o load_elf_rv.o
	$(CROSS_CC) -static -o $@ $^
