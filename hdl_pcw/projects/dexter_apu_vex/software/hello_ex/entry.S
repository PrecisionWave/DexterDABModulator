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

    li      t0, 0b1<<3
    csrrs   t0, mstatus, t0 

    # setup PMP
    #
    # pmpcfgX
    # 7     L           Lock bit (1 = Locked)
    # 6:5   Reserved    0
    # 4:3   A           Address Matching: 11 = NAPOT
    # 2     X           Execute permission
    # 1     W           Write permission
    # 0     R           Read permission
    # In addition to locking the PMP entry, the L bit indicates whether the R/W/X permissions are
    # enforced on M-mode accesses. When the L bit is set, these permissions are enforced for all privilege
    # modes. When the L bit is clear, any M-mode access matching the PMP entry will succeed; the
    # R/W/X permissions apply only to S and U modes
    #
    # pmpaddrX = (base + size/2 - 1) >> 2
    li      t0, (0x00000000 + 0x100000/2 - 1) >> 2      ; /* null pointer protection */
    csrrw   t0, pmpaddr0, t0
    addi    t0, zero, (0x80 | 0x18)
    csrrw   t0, pmpcfg0, t0

    li      t0, (0x00100000 + 0x3ff00000/2 - 1) >> 2    ; /* RAM */
    csrrw   t0, pmpaddr1, t0
    addi    t0, zero, (0x80 | 0x18 | 0x7) ; /* RWX */
    csrrw   t0, pmpcfg1, t0

    li      t0, (0x40000000 + 0x10000000/2 - 1) >> 2    ; /* Peripherals */
    csrrw   t0, pmpaddr2, t0
    addi    t0, zero, (0x80 | 0x18 | 0x3) ; /* RW */
    csrrw   t0, pmpcfg2, t0

    li      t0, (0x50000000 + 0xb0000000/2 - 1) >> 2    ; /* null pointer protection 2 */
    csrrw   t0, pmpaddr3, t0
    addi    t0, zero, (0x80 | 0x18)
    csrrw   t0, pmpcfg3, t0

    # test cases
    # exception 0x0...7 (store access fault)
    li t0, 0xfffff000
    sw      t0, 0(t0)

    # exception 0x0...6 (store misaligned)
    li t0, 0xfffff001
    sw      t0, 0(t0)
    
    # exception 0x0...5 (load access fault)
    li t0, 0x0
    lw      t0, 0(t0)

    # exception 0x0...4 (load miasligned)
    li t0, 0x1
    lw      t0, 0(t0)

    # exception 0x0...3 (breakpoint)
    ebreak

    # exception 0x0...1 
    li t0, 0x40000000
    jr t0

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

        lui a2, %hi(0x40600000)
        addi a2, a2, %lo(0x40600000)

        lui a3, %hi(msg_exc)
        addi a3, a3, %lo(msg_exc)

        UartLite_TransmitString a2, a4, a3

        # Decode the cause of the interrupt.
        #
        # 0x8...0 = Reserved
        # 0x8...1 = svr sw irq
        # 0x8...2 = Virtual svr sw irq
        # 0x8...3 = Machine sw irq
        # 0x8...4 = Reserved
        # 0x8...5 = svr timer irq
        # 0x8...6 = virtual svr timer irq
        # 0x8...7 = machine timer irq
        # 0x8...8 = Reserved
        # 0x8...9 = svr ext irq
        # 0x8...a = virtual ext irq
        # 0x8...b = machine ext irq
        # 0x8...c = svr guest ext irq
        #
        # 0x0...0 = instr misaligned
        # 0x0...1 = instr access fault
        # 0x0...2 = instr illegal
        # 0x0...3 = breakpoint
        # 0x0...4 = load misaligned
        # 0x0...5 = load access fault
        # 0x0...6 = store misaligned
        # 0x0...7 = store access fault
        # 0x0...8 = ecall from U or VU-mode
        # 0x0...9 = ecall from HS-mode
        # 0x0...a = ecall from VS-mode
        # 0x0...b = ecall from M-mode
        # 0x0...c = instr page fault
        # 0x0...d = load page fault
        # 0x0...f = store page fault
        # 0x0..14 = instr guest page fault
        # 0x0..15 = load guest page fault
        # 0x0..16 = virtual instr
        # 0x0..17 = store guest page fault
        csrr    a1, mcause

        lui a3, %hi(msg_hex)
        addi a3, a3, %lo(msg_hex)

        UartLite_Wait_TransmitNotFull a2, a4
        srli a4, a1, 28
        andi a4, a4, 0xf
        UartLite_TransmitHex a2, a4, a3

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
        UartLite_TransmitChar a2, a4, ' '
        UartLite_TransmitChar a2, a4, 'P'
        UartLite_TransmitChar a2, a4, 'C'
        UartLite_TransmitChar a2, a4, '='
        UartLite_TransmitChar a2, a4, '0'
        UartLite_TransmitChar a2, a4, 'x'

        # trap location
        csrr    a1, mepc

        lui a3, %hi(msg_hex)
        addi a3, a3, %lo(msg_hex)

        UartLite_Wait_TransmitNotFull a2, a4
        srli a4, a1, 28
        andi a4, a4, 0xf
        UartLite_TransmitHex a2, a4, a3

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
        UartLite_TransmitChar a2, a4, '\n'

        # die in case of instr access fault
        csrr    a1, mcause
        li      a2, 1           ; /* instr access fault */
        bne     a2, a1, 3f
2:
        j       2b

3:
        # mepc fixup to return to next instruction
        # This ignores 48 and 64-bit instructions!
        csrr    a3, mepc
        lbu     a1, 0(a3)       ; /* load instruction (lower 8 bits) */
        andi    a1, a1, 0x3
        li      a2, 0x3         ; /* 11b = 32-bit instruction. everything else 2 byte */
        addi    a3, a3, 2       ; /* add 2 bytes */
        beq     a3, a2, 4f      ; /* done if 2 byte instruction */
        addi    a3, a3, 2       ; /* add 2 more bytes for a total of 4 bytes */
4:
        csrrw   a3, mepc, a3

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

