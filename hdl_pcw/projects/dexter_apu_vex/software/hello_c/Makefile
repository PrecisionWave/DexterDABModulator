
CROSS_PREFIX := riscv-none-elf-
CROSS_CFLAGS := -march=rv32imc --specs=nosys.specs
CROSS_AFLAGS := -march=rv32imc 

.PHONY : clean all
all : clean hello_c.elf hello_c.asm

%.o: %.c
	$(CROSS_PREFIX)gcc $(CROSS_CFLAGS) -Wall -c -mcmodel=medany  $< -o $@

%.o: %.S
	$(CROSS_PREFIX)as $(CROSS_AFLAGS) -c $< -o $@

hello_c.elf: main.o entry.o xuartlite_l.o
	$(CROSS_PREFIX)gcc $(CROSS_CFLAGS)  -Wl,-Tlinker.ld -Wl,--emit-reloc -o $@ $^

hello_c.asm: hello_c.elf
	$(CROSS_PREFIX)objdump -D -h -r $< > $@

clean:
	-@rm *.o
