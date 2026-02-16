.section .vectors.reset, "ax"
.global _reset
_reset:
    lui t0, %hi(_start)
    addi t0, t0, %lo(_start)
    jr t0


.section .init, "ax"
.global _start

_start:

    lui t0, %hi(0x40600000)
    addi t0, t0, %lo(0x40600000)

    sw x0, 12(t0)       ; /* XUL_CONTROL_REG_OFFSET = 12 */

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
    j __done

__msg:
    .ascii "** Reset **\r\n\0"

