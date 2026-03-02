
CROSS_ARM := ../../bsp_pcw/bin/arm-none-linux-gnueabi-static/bin/arm-none-linux-gnueabi-
CROSS_RV := ../../hdl_pcw/projects/dexter_apu_vex/software/xpacks/.bin/riscv-none-elf-
CROSS_R5 := armr5-none-eabi-

SRCS := load_apu.c \
		mmio.c \
		mmap.c \
		memorymap.c \
		load_elf.c \
		load_elf_mb.c \
		load_elf_rv.c \
		load_elf_r5.c

.PHONY : clean all
all : clean load_apu load_apu_sim

clean:
	-@rm *.o
	-@rm *.oh
	-@rm load_apu
	-@rm load_apu_sim

# Zynq loader
%.o: %.c
	$(CROSS_ARM)gcc -c -Wall -march=armv7 -g3 -o $@ $<

load_apu: $(SRCS:.c=.o)
	$(CROSS_ARM)gcc -static -g3 -o $@ $^

# Host simulation
%.oh: %.c
	gcc -c -Wall -DSIM -g3 -o $@ $<

load_apu_sim: $(SRCS:.c=.oh)
	gcc -static -DSIM -g3 -o $@ $^

# test 
test_rv1: load_apu_sim load_apu
	-@./load_apu_sim -f main_10k.elf -vvv -x 10k
	$(CROSS_RV)objdump -D -r main_10k.elf > main_10k.asm
	$(CROSS_RV)objcopy main_10k.elf --adjust-vma=0x16800000 -O binary --remove-section vectors.reset main_10k.bin
	$(CROSS_RV)objdump -b binary --adjust-vma 0x16900000 -m riscv -D main_10k.bin > main_10k.adj
	$(CROSS_RV)objdump -b binary --adjust-vma=0x16900000 -m riscv -D 10k.DDR > main_10k.dis

test_rv2:
	$(CROSS_RV)objdump -b binary --adjust-vma 0x16900000 -m riscv -D reloc_10.DDR > reloc_10.asm
	$(CROSS_RV)objdump -b binary --adjust-vma=0x16900000 -m riscv -D reloc_169.DDR > reloc_169.asm

test_r5: load_apu_sim load_apu
	-@./load_apu_sim -f hello_r5_10k.elf -vvv -x r5
	$(CROSS_R5)objdump -D -r hello_r5_10k.elf > r5.asm
	$(CROSS_R5)objcopy hello_r5_169.elf -O binary --remove-section .vectors r5.bin
	$(CROSS_R5)objdump -b binary --adjust-vma 0x16900000 -m cortex-r5 -D r5.bin > r5.adj
	$(CROSS_R5)objdump -b binary --adjust-vma=0x16900000 -m cortex-r5 -D r5.DDR > r5.dis
