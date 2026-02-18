.section .vectors.reset, "ax"
.global __reset
__reset:

.cfi_startproc
.cfi_undefined ra

    .option push
    .option norelax
        la gp, __global_pointer$
    .option pop

    la sp, _stack
    add s0, sp, zero

    la      t0, scratch
    csrrw   zero, mscratch, t0
    la      t0, trap_handler
    csrrw   zero, mtvec, t0

    lui     t0, %hi(_start)
    addi    t0, t0, %lo(_start)

    ebreak

1:
    j   1b
.cfi_endproc

.macro UartLite_Wait_TransmitNotFull UART_BASE, TEMP_REG
1:
    lw      \TEMP_REG, 0x8(\UART_BASE)      ; /* XUL_STATUS_REG_OFFSET = 8*/
    andi    \TEMP_REG, \TEMP_REG, 0x08      ; /* XUL_SR_TX_FIFO_FULL  = 0x08 */
    bnez    \TEMP_REG, 1b
.endm

.macro UartLite_TransmitChar UART_BASE, TEMP_REG, CHAR
    UartLite_Wait_TransmitNotFull \UART_BASE \TEMP_REG
    li      \TEMP_REG, \CHAR
    sw      \TEMP_REG, 4(\UART_BASE)        ; /* XUL_TX_FIFO_OFFSET = 4 */
.endm

.macro UartLite_TransmitString UART_BASE, TEMP_REG, MSG
1:
    UartLite_Wait_TransmitNotFull \UART_BASE \TEMP_REG

    lbu     \TEMP_REG, 0(\MSG)
    beqz    \TEMP_REG, 2f

    sw      \TEMP_REG, 4( \UART_BASE)       ; /* XUL_TX_FIFO_OFFSET = 4 */

    addi    \MSG, \MSG, 1
    j 1b
2:
.endm

.macro UartLite_TransmitHex UART_BASE, VAL_REG, HEXTAB
    add     \VAL_REG, \HEXTAB, \VAL_REG
    lbu     \VAL_REG, 0(\VAL_REG)
    sw      \VAL_REG, 4(\UART_BASE)        ; /* XUL_TX_FIFO_OFFSET = 4 */
.endm

.global trap_handler
trap_handler:
        # Trap handler preamble (save registers).
        csrrw   a0, mscratch, a0
        sw      a1, 0(a0)
        sw      a2, 8(a0)
        sw      a3, 16(a0)
        sw      a4, 24(a0)

        # Decode the cause of the interrupt.
        csrr    a1, mcause

        lui a2, %hi(0x40600000)
        addi a2, a2, %lo(0x40600000)

        lui a3, %hi(msg_exc)
        addi a3, a3, %lo(msg_exc)

        UartLite_TransmitString a2, a4, a3

        lui a3, %hi(msg_hex)
        addi a3, a3, %lo(msg_hex)

        UartLite_Wait_TransmitNotFull a2, a4
        srli a4, a1, 24
        andi a4, a4, 0xf
        UartLite_TransmitHex a2, a4, a3

        UartLite_Wait_TransmitNotFull a2, a4
        srli a4, a1, 20
        andi a4, a4, 0xf
        UartLite_TransmitHex a2, a4, a3

        UartLite_Wait_TransmitNotFull a2, a4
        srli a4, a1, 16
        andi a4, a4, 0xf
        UartLite_TransmitHex a2, a4, a3

        UartLite_Wait_TransmitNotFull a2, a4
        srli a4, a1, 12
        andi a4, a4, 0xf
        UartLite_TransmitHex a2, a4, a3

        UartLite_Wait_TransmitNotFull a2, a4
        srli a4, a1, 8
        andi a4, a4, 0xf
        UartLite_TransmitHex a2, a4, a3

        UartLite_Wait_TransmitNotFull a2, a4
        srli a4, a1, 4
        andi a4, a4, 0xf
        UartLite_TransmitHex a2, a4, a3

        UartLite_Wait_TransmitNotFull a2, a4
        addi a4, a1, 0
        andi a4, a4, 0xf
        UartLite_TransmitHex a2, a4, a3

        UartLite_Wait_TransmitNotFull a2, a4
        UartLite_TransmitChar a2, a4, '\r'

        UartLite_Wait_TransmitNotFull a2, a4
        UartLite_TransmitChar a2, a4, '\n'
3:
        j 3b
trap_handler_restore_state:
        lw      a4, 24(a0)
        lw      a3, 16(a0)
        lw      a2, 8(a0)
        lw      a1, 0(a0)
        csrrw   a0, mscratch, a0
        mret

.global main
main:


.section .bss
scratch:        .dword 0, 0, 0, 0

.section .data
msg_exc:        .asciz "\r\n\033[0;31mException:\033[0m 0x"
msg_hex:        .ascii "0123456789abcdef"

