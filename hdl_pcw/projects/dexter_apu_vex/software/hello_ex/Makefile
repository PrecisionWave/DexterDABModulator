
CROSS_PREFIX := riscv-none-elf-
CROSS_CFLAGS := -march=rv32imc_zicsr --specs=nosys.specs
CROSS_AFLAGS := -march=rv32imc_zicsr 

.PHONY : clean all
all : clean hello_exc.elf hello_exc.asm

%.o: %.c
	$(CROSS_PREFIX)gcc $(CROSS_CFLAGS) -Wall -c -mcmodel=medany  $< -o $@

%.o: %.S
	$(CROSS_PREFIX)as $(CROSS_AFLAGS) -c $< -o $@

hello_exc.elf: entry.o
	$(CROSS_PREFIX)gcc $(CROSS_CFLAGS)  -Wl,-Tlinker.ld -Wl,--emit-reloc -o $@ $^

hello_exc.asm: hello_exc.elf
	$(CROSS_PREFIX)objdump -D -h -r $< > $@

clean:
	-@rm *.o
