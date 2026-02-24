.section .vectors.reset, "ax"
.global _reset
_reset:
    lui t0, %hi(_start)
    addi t0, t0, %lo(_start)
    jr t0

.rept 0x800
    .byte 0
.endr


.section .init, "ax"
.global _start

_start:
.cfi_startproc
.cfi_undefined ra

    .option push
    .option norelax
        la gp, __global_pointer$
    .option pop

    la sp, _stack
    add s0, sp, zero
    jal zero, main

.cfi_endproc

/* relocation tests */
    lui t0, %hi(_reset)
    addi    t0, t0, %lo(_reset)
    lui t0, %hi(_reset+0x7ff)
    addi    t0, t0, %lo(_reset+0x7ff)
    lui t0, %hi(_reset+0x800)
    addi    t0, t0, %lo(_reset+0x800)
    lui t0, %hi(_reset+0x801)
    addi    t0, t0, %lo(_reset+0x801)

.rept 512
1:
    auipc t0, %pcrel_hi(_test1)
    addi    t0, t0, %pcrel_lo(1b)
.endr
_test1:

.rept 512
2:
    auipc t0, %pcrel_hi(_test2)
    addi    t0, t0, %pcrel_lo(2b)
.endr
_test2:
.rept 512
2:
    auipc t0, %pcrel_hi(_test2)
    addi    t0, t0, %pcrel_lo(2b)
.endr

.end
