
CROSS_CC := ../../bsp_pcw/bin/arm-none-linux-gnueabi-static/bin/arm-none-linux-gnueabi-gcc

.PHONY : clean all
all : clean load_apu load_apu_sim

clean:
	-@rm *.o
	-@rm *.oh
	-@rm load_apu
	-@rm load_apu_sim

%.o: %.c
	$(CROSS_CC) -c -Wall -march=armv7 -o $@ $<

load_apu: load_apu.o mmio.o mmap.o memorymap.o load_elf.o load_elf_mb.o load_elf_rv.o
	$(CROSS_CC) -static -o $@ $^


%.oh: %.c
	gcc -c -Wall -DSIM -o $@ $<

load_apu_sim: load_apu.oh mmio.oh mmap.oh memorymap.oh load_elf.oh load_elf_mb.oh load_elf_rv.oh
	gcc -static -DSIM -o $@ $^

