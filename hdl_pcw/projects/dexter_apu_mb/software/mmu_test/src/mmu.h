#ifndef MMU_H_DEFINED
#define MMU_H_DEFINED

//  Yes, it is possible to disable/enable the MMU. When MicroBlaze is configured to use the MMU by setting C_USE_MMU to 3
//  (Virtual), the processor is in real privileged mode after reset, which means that the MMU is disabled. All addresses
//  are physical, no protection checks are done, and all instructions are allowed.
//  To go to virtual mode, you must set the VM bit in MSR, and to go to user mode you must set the UM bit. 
//  Note that when setting VM, you must always do a synchronizing branch (usually "bri 4"), so typically you would enable
//  the MMU by "msrset r0, 0x2800 ; bri 4" in assembly. Before doing this you must also have set up at least one TLB
//  entry, using "mts rtlbhi" and "mts rtlblo".
//  When you are in virtual user mode, the only way to get back to real privileged mode is with a system call, typically
//  using "brki r16, 0x8". That will jump to the user vector at physical address 0x8 and clear VM and UM, disabling the
//  MMU again. To make this work, you would have to set up a user vector that will jump to a system call handler in the
//  assembler code. You could do this with ".section .vectors.sw_exception, "ax" ; bri handler". When returning from the
//  handler with "rtbd r16, 4", VM and UM are set from the saved values in VMS and UMS, which will normally return to
//  virtual user mode at the instruction following the system call.

#define MICROBLAZE_TLB_ENTRIES  64

// TLBHI Register Bits
#define TLBHI_TAG_MASK          (((1<<22) - 1) << 10)
#define TLB_ADDR_MASK_1KB       (((1<<22) - 1) << 10)
#define TLB_ADDR_MASK_4KB       (((1<<20) - 1) << 12)
#define TLB_ADDR_MASK_16KB      (((1<<18) - 1) << 14)
#define TLB_ADDR_MASK_64KB      (((1<<16) - 1) << 16)
#define TLB_ADDR_MASK_256KB     (((1<<14) - 1) << 18)
#define TLB_ADDR_MASK_1MB       (((1<<12) - 1) << 20)
#define TLB_ADDR_MASK_4MB       (((1<<10) - 1) << 22)
#define TLB_ADDR_MASK_16MB      (((1<<8) - 1) << 24)

#define TLBHI_SIZE_1KB          (0 << 7)
#define TLBHI_SIZE_4KB          (1 << 7)
#define TLBHI_SIZE_16KB         (2 << 7)
#define TLBHI_SIZE_64KB         (3 << 7)
#define TLBHI_SIZE_256KB        (4 << 7)
#define TLBHI_SIZE_1MB          (5 << 7)
#define TLBHI_SIZE_4MB          (6 << 7)
#define TLBHI_SIZE_16MB         (7 << 7)
#define TLBHI_SIZE_MASK         ((1<<3) - 1)
#define TLBHI_V                 (1<<6)
#define TLBHI_E                 (1<<5)

// TLBLO Register Bits
#define TLBLO_G                 (1<<0)
#define TLBLO_M                 (1<<1)
#define TLBLO_I                 (1<<2)
#define TLBLO_W                 (1<<3)
#define TLBLO_ZSEL_POS          4
#define TLBLO_ZSEL_MASK         ((1<<4) - 1)
#define TLBLO_WR                (1<<8)
#define TLBLO_EX                (1<<9)
#define TLBLO_RPN_MASK          (((1<<22) - 1) << 10)

// TLBHI - Translation Look-Aside Buffer High Register
//   Manual    | 0                21 | 22   24 | 25 | 26 | 27 | 28  31 |
//   Bit       | 31               10 |  9    7 |  6 |  5 |  4 |  3   0 |
//   Field     |          TAG        |   SIZE  |  V |  E | U0 |   RSVD |
//
//   - TAG   TLB-entry tag   Read/Write
//               Is compared with the page number portion of the virtual memory address under the control of the SIZE
//               field.
//   - SIZE  Size            Read/Write
//               Specifies the page size. The SIZE field controls the bit range used in comparing the TAG field with
//               the page number portion of the virtual memory address. The page sizes defined by this field are listed
//               here:
//
//               Page    SIZE    Tag Comparison Bit                          PAE Disabled 
//               Size    Field   Range                       Page Offset     Physical Page Number    RPN Bits Clear to 0
//                 1 KB  000     TAG and Address[31:10]      Address[9:0]    RPN[31:10]              -
//                 4 KB  001     TAG and Address[31:12]      Address[11:0]   RPN[31:12]              11:10
//                16 KB  010     TAG and Address[31:14]      Address[13:0]   RPN[31:14]              13:10
//                64 KB  011     TAG and Address[31:16]      Address[15:0]   RPN[31:16]              15:10
//               256 KB  100     TAG and Address[31:18]      Address[17:0]   RPN[31:18]              17:10
//                 1 MB  101     TAG and Address[31:20]      Address[19:0]   RPN[31:20]              19:10
//                 4 MB  110     TAG and Address[31:22]      Address[21:0]   RPN[31:22]              21:10
//                16 MB  111     TAG and Address[31:24]      Address[23:0]   RPN[31:24]              23:10


//   - V     Valid           Read/Write
//               When this bit is set to 1, the TLB entry is valid and contains a pagetranslation entry.
//               When cleared to 0, the TLB entry is invalid.
//   - E     Endian          Read/Write
//               When this bit is set to 1, the page is accessed as a big endian page. When cleared to 0, the page is
//               accessed as a little endian page.
//               The E bit only affects data read or data write accesses. Instruction accesses are not affected.
//               The E bit is only implemented when the parameter C_USE_REORDER_INSTR is set to 1, otherwise it is
//               fixed to 0.
//   - U0    User Defined    Read Only
//               This bit is fixed to 0, because there are no user defined storage
//   - Reserved  


// TLBLO - Translation Look-Aside Buffer Low Register
//   Manual    | 0                21 | 22 | 23 | 24   27 | 28 | 29 | 30 | 31 |
//   Bit       | 31               10 |  9 |  8 |  7    4 |  3 |  2 |  1 |  0 |
//   Field     |        RPN          | EX | WR |   ZSEL  |  W |  I |  M |  G |
//
//   - RPN   Real Page Number or Physical Page Number        Read/Write
//               When a TLB hit occurs, this field is read from the TLB entry and is used to form the physical address.
//               Depending on the value of the SIZE field, some of the RPN bits are not used in the physical address.
//               Software must clear unused bits in this field to zero. Only defined when C_USE_MMU=3 (Virtual).
//   - EX    Executable          Read/Write
//               When bit is set to 1, the page contains executable code, and instructions can be fetched from the page.
//               When bit is cleared to 0, instructions cannot be fetched from the page. Attempts to fetch instructions
//               from a page with a clear EX bit cause an instructionstorage exception.
//   - WR    Writable            Read/Write
//               When bit is set to 1, the page is writable and store instructions can be used to store data at addresses
//               within the page. When bit is cleared to 0, the page is read-only (not writable).
//               Attempts to store data into a page with a clear WR bit cause a data storage exception.
//   - ZSEL  Zone Select         Read/Write
//               This field selects one of 16 zone fields (Z0-Z15) from the zoneprotection register (ZPR).
//               For example, if ZSEL 0x5, zone field Z5 is selected. The selected ZPR field is used to modify the access
//               protection specified by the TLB entry EX and WR fields. It is also used to prevent access to a page by
//               overriding the TLB V (valid) field.
//   - W     Write Through       Read/Write
//               When the parameter C_DCACHE_USE_WRITEBACK is set to 1, this bit controls caching policy. A write-through
//               policy is selected when set to 1, and a write-back policy is selected otherwise.
//               This bit is fixed to 1, and write-through is always used, when C_DCACHE_USE_WRITEBACK is cleared to 0.
//   - I     Inhibit Caching     Read/Write
//               When bit is set to 1, accesses to the page are not cached (caching is inhibited).
//               When cleared to 0, accesses to the page are cacheable.
//   - M     Memory Coherent     Read Only
//               This bit is fixed to 0, because memory coherence is not implemented on MicroBlaze.
//   - G     Guarded             Read/Write
//               When bit is set to 1, speculative page accesses are not allowed (memory is guarded).
//               When cleared to 0, speculative page accesses are allowed.
//               The G attribute can be used to protect memory-mapped I/O devices
//               from inappropriate instruction accesses.

// TLBX - Translation Look-Aside Buffer Index Register
//   Manual    | 0    | 1            25 | 26   31 |
//   Bit       | 31   | 30            6 |  5    0 |
//   Field     | MISS | Reserved        |  INDEX  |
//
//   - MISS  TLB Miss    Read Only
//               This bit is cleared to 0 when the TLBSX register is written with a virtual address, and the virtual
//               address is found in a TLB entry. The bit is set to 1 if the virtual address is not found. It is also
//               cleared when the TLBX register itself is written.
//   - Reserved  0
//   - INDEX TLB Index   Read/Write
//               This field is used to index the Translation Look-Aside Buffer entry accessed by the TLBLO and TLBHI
//               registers. The field is updated with a TLB index when the TLBSX register is written with a virtual
//               address, and the virtual address is found in the corresponding TLB entry.

// TLBSX - Translation Look-Aside Buffer Search Index Register
//   Manual    | 0                   21 | 22     31 |
//   Bit       | 31                  10 |  9      0 |
//   Field     |        VPN             |  Reserved |
//
//   - VPN   Virtual Page Number     Write Only
//               This field represents the page number portion of the virtual memory
//               address. It is compared with the page number portion of the virtual
//               memory address under the control of the SIZE field, in each of the
//               Translation Look-Aside Buffer entries that have the V bit set to 1.
//               If the virtual page number is found, the TLBX register is written with
//               the index of the TLB entry and the MISS bit in TLBX is cleared to 0.
//               If the virtual page number is not found in any of the TLB entries, the
//               MISS bit in the TLBX register is set to 1.
//               

// PID - Process Identifier Register
//    Manual    | 0                   23 | 24     31 |
//    Bit       | 31                   8 |  7      0 |
//    Field     |        Reserved        |     PID   |
// 
//    - PID   Peiocess ID     Read/Write
//            Used to uniquely identify a software process during MMU address translation.


// MSR - Machine Status Register
//   Manual    |  0 |  1       16 |  17 | 18 |  19 | 20 |  21 |  22 | 23 |  24 |  25 |  26 |  27 |  28 | 29 | 30 |  31 |
//   Bit       | 31 | 30       15 |  14 | 13 |  12 | 11 |  10 |   9 |  8 |   7 |   6 |   5 |   4 |   3 |  2 |  1 |   0 |
//   Field     | CC |  Reserved   | VMS | VM | UMS | UM | PVR | EIP | EE | DCE | DZO | ICE | FSL | BIP |  C | IE | RES |
//
//   - VMS   Virtual Protected Mode Save
//   - VM    Virtual Protected Mode
//               0 = MMU address translation and access protection disabled, with C_USE_MMU = 3 (Virtual).
//                   Access protection disabled with C_USE_MMU = 2 (Protection)
//               1 = MMU address translation and access protection enabled, with C_USE_MMU = 3 (Virtual).
//                   Access protection enabled, with C_USE_MMU = 2 (Protection).
//   - UMS   User Mode Save
//   - UM    User Mode
//               0 = Privileged Mode, all instructions are allowed
//               1 = User Mode, certain instructions are not allowed
//               Only available when configured with an MMU
//   - PVR    Processor Version Register exists
//               0 = No Processor Version Register
//               1 = Processor Version Register exists
//   - EIP   Exception In Progress
//               0 = No hardware exception in progress
//               1 = Hardware exception in progress
//   - EE    Exception Enable
//               0 = Hardware exceptions disabled2
//               1 = Hardware exceptions enabled
//   - DCE   Data Cache Enable
//               0 = Data Cache disabled
//               1 = Data Cache enabled
//   - DZO   Division by Zero or Division Overflow
//               0 = No division by zero or division overflow has occurred
//               1 = Division by zero or division overflow has occurred
//               Only available if configured to use hardware divider
//   - ICE   Instruction Cache Enable
//               0 = Instruction Cache disabled
//               1 = Instruction Cache enabled
//               Only available if configured to use instruction cach
//   - FSL   AXI4-Stream Error
//               0 = get or getd had no error
//               1 = get or getd control type mismatch
//               This bit is sticky, that is it is set by a get or getd instruction when a control bit mismatch occurs.
//               To clear it an MTS or MSRCLR instruction must be used.
//               Only available if configured to use stream links
//   - BIP   Break in Progress
//               0 = No Break in Progress
//               1 = Break in Progress
//               Break Sources can be software break instruction or hardware break from Ext_Brk or Ext_NM_Brk pin.
//   - C     Arithmetic Carry
//               0 = No Carry (Borrow)
//               1 = Carry (No Borrow)
//   - IE    Interrupt Enable
//               0 = Interrupts disabled
//               1 = Interrupts enabled
#define MSR_CC  (1<<31)
#define MSR_VMS (1<<14)
#define MSR_VM  (1<<13)
#define MSR_UMS (1<<12)
#define MSR_UM  (1<<11)
#define MSR_PVR (1<<10)
#define MSR_EIP (1<<9)
#define MSR_EE  (1<<8)
#define MSR_DCE (1<<7)
#define MSR_DZO (1<<6)
#define MSR_ICE (1<<5)
#define MSR_FSL (1<<4)
#define MSR_BIP (1<<3)
#define MSR_C   (1<<2)
#define MSR_IE  (1<<1)
#define MSR_RES (1<<0)


#endif // #define MMU_H_DEFINED
