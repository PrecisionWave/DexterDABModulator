.section .vectors.reset, "ax"
.global __reset
__reset:

    lui t0, %hi(0x40600000)
    addi t0, t0, %lo(0x40600000)

    lui t1, %hi(__msg)
    addi t1, t1, %lo(__msg)

__loop:
    lw t2, 0x8(t0)      ; /* XUL_STATUS_REG_OFFSET = 8*/
    andi t2, t2, 0x08   ; /* XUL_SR_TX_FIFO_FULL  = 0x08 */
    bnez t2, __loop

    lbu t2, 0(t1)
    beqz t2, __done

    sw t2, 4(t0)        ; /* XUL_TX_FIFO_OFFSET = 4 */

    addi t1, t1, 1
    j __loop

__done:
    lui t0, %hi(_start)
    addi t0, t0, %lo(_start)
    jr t0

__msg:
    .ascii "** hello_c reset **\r\n\0"

/* The GCC port might not emit a __register_frame_info symbol, which eventually
 * results in a weak undefined reference that eventually causes crash when it
 * is dereference early in boot.  We really shouldn't need to put this here,
 * but to deal with what I think is probably a bug in the linker script I'm
 * going to leave this in for now.  At least it's fairly cheap :) */
.weak __register_frame_info
.section .text.init.__register_frame_info
__register_frame_info:
    .cfi_startproc
    ret
    .cfi_endproc

.weak __deregister_frame_info
.section .text.init.__deregister_frame_info
__deregister_frame_info:
    .cfi_startproc
    ret
    .cfi_endproc
