.section .vectors.reset, "ax"
.global _reset
_reset:
    lui t0, %hi(_start)
    addi t0, t0, %lo(_start)
    jr t0


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

.end
