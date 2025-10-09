# Microblaze Axi Stream instructions
The MicroBlaze AXI4-Stream interfaces (M0_AXIS, M15_AXIS, S0_AXIS, S15_AXIS) are implemented as 32-bit masters and slaves.

Previous versions of the Microblaze processor implemented the stream interfaces as Fast Simplex Link (FSL).

From a software point of view there is no difference between the two implementations.

The term FSL is omnipresent on the software side (Instruction names, documentation, include files, macro names,...)

| FSL Name | AXIS Type | AXIS Name |
| -------- | --------- | --------- |
| Read     | Slave     | TREADY    |
| Write    | Master    | TVALID    |
| Control  | Both      | TLAST     |


## Write Operation
A write to the stream interface is performed by MicroBlaze using one of the put or putd instructions. A write operation transfers the register contents to an output AXI4 interface. The transfer is completed in a single clock cycle for blocking mode writes (put and cput instructions) as long as the interface is not busy. If the interface is busy, the processor stalls until it becomes available. The non-blocking instructions (with prefix n), always complete in a single clock cycle even if the interface is busy. If the interface was busy, the write is inhibited and the carry bit is set in the MSR.

The control instructions (with prefix c) set the AXI4-Stream TLAST output, to ‘1’, which is used to indicate the boundary of a packet.

## Read Operation
A read from the stream interface is performed by MicroBlaze using one of the get or getd instructions. A read operations transfers the contents of an input AXI4 interface to a general purpose register. The transfer is typically completed in two clock cycles for blocking mode reads as long as data is available. If data is not available, the processor stalls at this instruction until it becomes available. In the non-blocking mode (instructions with prefix n), the transfer is completed in one or two clock cycles irrespective of whether or not data was available. In case data was not available, the transfer of data does not take place and the carry bit is set in the MSR.

The data get instructions (without prefix c) expect the AXI4-Stream TLAST input to be cleared to ‘0’, otherwise the instructions will set MSR[FSL] to ‘1’. Conversely, the control get instructions (with prefix c) expect the TLAST input to be set to ‘1’, otherwise the instructions will set MSR[FSL] to ‘1’. This can be used to check for the boundary of a packet.

## Usage in C programs
In a Vitis project, simply insert `#include "fsl.h"` and start using the `fslx` macros:
```c
    #include <stdint.h>
    #include "fsl.h"

    int main() {
        uint32_t value;

        for (;;) {
            getfslx(value, 0, FSL_DEFAULT);
            putfslx(~value, 0, FSL_DEFAULT);
        }

        return 0;
    }
```

### Macros
#### Static Link ID
The static versions of the FSL instructions have the stream id hardcoded in the instruction.  
The flags parameter control which instruction is generated and needs to be known at compile time (when the preprocessor
runs).

```c
#define getfslx(val, id, flags)
#define tgetfslx(val, id, flags)

#define putfslx(val, id, flags)
#define tputfslx(id, flags)
```
| Macro parameter | Description      | Type                      | Range / Bits                   |
| --------------- | ---------------- | ------------------------- | ------------------------------ |
| val             | Value            | variable                  | 32-Bits                        |
| id              | Link ID          | constant for preprocessor | 0 - 15                         |
| flags           | Instruction type | constant for preprocessor | **one** of the FSL_xxx defines |

##### Example
```c
    uint32_t value;
    getfslx(value, 0, FSL_DEFAULT);
```

#### Dynamic Link ID
The dynamic versions of the FSL instructions get the stream id in a register and can be therefore modified at runtime.  
The flags parameter control which instruction is generated and needs to be known at compile time (when the preprocessor
runs).

```c
#define getdfslx(val, var, flags)
#define tgetdfslx(val, var, flags)

#define putdfslx(val, var, flags)
#define tputdfslx(var, flags)
```
| Macro parameter | Description      | Type                      | Range / Bits                   |
| --------------- | ---------------- | ------------------------- | ------------------------------ |
| val             | Value            | variable                  | 32-Bits                        |
| var             | Link ID          | variable                  | 0 - 15                         |
| flags           | Instruction type | constant for preprocessor | **one** of the FSL_xxx defines |

##### Example
```c
    uint32_t value;
    uint32_t id = 3;
    getdfslx(value, id, FSL_DEFAULT);
```


#### Error checking: Is invalid
```c
#define fsl_isinvalid(result)        asm volatile ("addic\t%0,r0,0"  : "=d" (result))
```
This macro will set result to 1 if MSR[Carry] bit was set and to 0 if not set.  
MSR[Carry] is usually used for the non-blocking version of the instructions to signal
if data was available (0) or no data was read (1).

As this macro uses the carry flag it should be used immediately after the XXXfslx macro.

##### Example
```c
    uint32_t invalid;
    uint32_t value;
    uint32_t id = 3;
    // Peek if data is available
    getdfslx(value, id, FSL_NONBLOCKING);
    fsl_isinvalid(invalid);
    if (!invalid) {
        // Get data here will not block as we know that data is available
        getdfslx(value, id, FSL_DEFAULT);
        putdfslx(value, id, FSL_DEFAULT);
    }
```


#### Error checking: Is Error
```c
#define fsl_iserror(error)           asm volatile ("mfs\t%0,rmsr\n\t"  \
                                                   "andi\t%0,%0,0x10" : "=d" (error))
```
This macro will set error to 16 (0x10) if MSR[FSL_Error] bit in the MSR was set and to 0 if not set.  
FSL_Error usually means there was a mismatch between expected TLAST and actual TLAST.

**Note**: The MSR[FSL_Error] only get set by the XXXfslx instructions. It needs to be manually cleared by the user.
```c
#define fsl_clearerror()             asm volatile ("msrclr\tr0,%0\n"  \
                                                   "nop;" : : "i" (0x10))
```

##### Example
```c
    #define fsl_clearerror() asm volatile ("msrclr\tr0,%0\n"  \
                                           "nop;" : : "i" (0x10))
    uint32_t error;
    uint32_t value;
    uint32_t id = 3;
    // Enure MSR[FSL] is cleared
    fsl_clearerror();
    // Get a word from the stream
    getdfslx(value, id, FSL_DEFAULT);
    fsl_iserror(error);
    if (error) printf("FSL_Error set!\n");
```

##### Error checking (both)
As the macros directly translate to inline assembly some care must be taken:
- fsl_isinvalid is using the carry flag to set the result.  
  It should be directly used after the XXXfslx instruction.
- fsl_iserror is using the exclusive FSL_Error MSR flag and can be evaluated anytime later before
  calling another FSL instruction.
- the FSL_Error MSR flag needs to be reset manually by the user.

```c
    #define fsl_clearerror() asm volatile ("msrclr\tr0,%0\n"  \
                                           "nop;" : : "i" (0x10))
    uint32_t invalid;
    uint32_t error;
    uint32_t value;
    uint32_t id = 0;
    fsl_clearerror();
    for (;;) {
        getdfslx(value, id, FSL_NONBLOCKING);
        fsl_isinvalid(invalid);
        fsl_iserror(error);
        printf("Error Demo: invalid: %lu, error: %lu, value: 0x%08lx\n",
                invalid, error, value);
        if (error)   fsl_clearerror();
        if (invalid) usleep(500 * 1000);
    }
```

#### Complete Example
This example will "pump" inverted data from S0_AXIS to M0_AXIS, It keeps TLAST intact.
fsl_clearerror macro can need to be copied over.

```c
    #define fsl_clearerror() asm volatile ("msrclr\tr0,%0\n"  \
                                           "nop;" : : "i" (0x10))
    uint32_t invalid;
    uint32_t value;
    uint32_t error;
    uint32_t id = 0;
    fsl_clearerror();
    for (;;) {
        // Peek if data is available
        getdfslx(value, id, FSL_NONBLOCKING);
        fsl_isinvalid(invalid);
        fsl_iserror(error);
        printf("Pump Demo: invalid: %lu, error: %lu, value: 0x%08lx\n",
                invalid, error, value);
        if (!invalid) {
            // Get data here will not block as we know that data is available
            getdfslx(value, id, FSL_DEFAULT);
            // Put data might block
            if (error) putdfslx(~value, id, FSL_CONTROL);
            else       putdfslx(~value, id, FSL_DEFAULT);
        } else {
            usleep(500 * 1000);
        }
        // clear MSR[FSL] in case of error
        if (error) fsl_clearerror();
    }
```

### fsl.h
```c
/* Extended FSL macros. These now replace all of the previous FSL macros */
#define FSL_DEFAULT
#define FSL_NONBLOCKING                          n
#define FSL_EXCEPTION                            e
#define FSL_CONTROL                              c
#define FSL_ATOMIC                               a

#define FSL_NONBLOCKING_EXCEPTION                ne
#define FSL_NONBLOCKING_CONTROL                  nc
#define FSL_NONBLOCKING_ATOMIC                   na
#define FSL_EXCEPTION_CONTROL                    ec
#define FSL_EXCEPTION_ATOMIC                     ea
#define FSL_CONTROL_ATOMIC                       ca

#define FSL_NONBLOCKING_EXCEPTION_CONTROL        nec
#define FSL_NONBLOCKING_EXCEPTION_ATOMIC         nea
#define FSL_NONBLOCKING_CONTROL_ATOMIC           nca
#define FSL_EXCEPTION_CONTROL_ATOMIC             eca

#define FSL_NONBLOCKING_EXCEPTION_CONTROL_ATOMIC neca

#define getfslx(val, id, flags)      asm volatile (stringify(flags) "get\t%0,rfsl" stringify(id) : "=d" (val))
#define putfslx(val, id, flags)      asm volatile (stringify(flags) "put\t%0,rfsl" stringify(id) :: "d" (val))

#define tgetfslx(val, id, flags)     asm volatile ("t" stringify(flags) "get\t%0,rfsl" stringify(id) : "=d" (val))
#define tputfslx(id, flags)          asm volatile ("t" stringify(flags) "put\trfsl" stringify(id))

#define getdfslx(val, var, flags)    asm volatile (stringify(flags) "getd\t%0,%1" : "=d" (val) : "d" (var))
#define putdfslx(val, var, flags)    asm volatile (stringify(flags) "putd\t%0,%1" :: "d" (val), "d" (var))

#define tgetdfslx(val, var, flags)   asm volatile ("t" stringify(flags) "getd\t%0,%1" : "=d" (val) : "d" (var))
#define tputdfslx(var, flags)        asm volatile ("t" stringify(flags) "putd\t%0" :: "d" (var))

/* FSL valid and error check macros. */
#define fsl_isinvalid(result)        asm volatile ("addic\t%0,r0,0"  : "=d" (result))
#define fsl_iserror(error)           asm volatile ("mfs\t%0,rmsr\n\t"  \
                                                   "andi\t%0,%0,0x10" : "=d" (error))

#define fsl_clearerror()             asm volatile ("msrclr\tr0,%0\n"  \
                                                   "nop;" : : "i" (0x10))
```

### flags fsl.h macros
| macro parameter `flags`                  | prefix | prefix with **t** macro |
| ---------------------------------------- | ------ | ----------------------- |
| FSL_DEFAULT                              | *none* | t                       |
| FSL_NONBLOCKING                          | n      | tn                      |
| FSL_EXCEPTION                            | e      | te                      |
| FSL_CONTROL                              | c      | tc                      |
| FSL_ATOMIC                               | a      | ta                      |
| FSL_NONBLOCKING_EXCEPTION                | ne     | tne                     |
| FSL_NONBLOCKING_CONTROL                  | nc     | tnc                     |
| FSL_NONBLOCKING_ATOMIC                   | na     | tna                     |
| FSL_EXCEPTION_CONTROL                    | ec     | tec                     |
| FSL_EXCEPTION_ATOMIC                     | ea     | tea                     |
| FSL_CONTROL_ATOMIC                       | ca     | tca                     |
| FSL_NONBLOCKING_EXCEPTION_CONTROL        | nec    | tnec                    |
| FSL_NONBLOCKING_EXCEPTION_ATOMIC         | nea    | tnea                    |
| FSL_NONBLOCKING_CONTROL_ATOMIC           | nca    | tnca                    |
| FSL_EXCEPTION_CONTROL_ATOMIC             | eca    | teca                    |
| FSL_NONBLOCKING_EXCEPTION_CONTROL_ATOMIC | neca   | tneca                   |


## Special Registers
Special registers for reference in usual "correct" bit order.  
Warning: The bit order in the xilinx manuals are "reversed".

### Machine-Status-Register (MSR)
| Bits | Manual | Name | Alternate Name | Description                                                     |
| ---- | ------ | ---- | -------------- | --------------------------------------------------------------- |
| 2    | 29     | C    | Carry          | Arithmetic Carry                                                |
|      |        |      |                | 0 = No Carry (Borrow)                                           |
|      |        |      |                | 1 = Carry (No Borrow)                                           |
|      |        |      |                | Read/Write                                                      |
| 4    | 27     | FSL  | FSL_Error      | FSL Error                                                       |
|      |        |      |                | 0 = FSL get/getd/put/putd had no error                          |
|      |        |      |                | 1 = FSL get/getd/put/putd control type mismatch                 |
|      |        |      |                | Only available if configured to use FSL links (C_FSL_LINKS > 0) |
|      |        |      |                | Read/Write                                                      |
| 11   | 20     | UM   |                | User Mode                                                       |
|      |        |      |                | 0 = Privileged Mode, all instructions are allowed               |
|      |        |      |                | 1 = User Mode, certain instructions are not allowed             |
|      |        |      |                | Only available when configured with an MMU                      |
|      |        |      |                | (if C_USE_MMU > 0 and C_AREA_OPTIMIZED = 0 or 2)                |

[UG984/MSR](https://docs.amd.com/r/en-US/ug984-vivado-microblaze-ref/Machine-Status-Register)

### Exception-Status-Register (ESR)
| Bits | Manual | Name | Description                                  |
| ---- | ------ | ---- | -------------------------------------------- |
| 4:0  | 27:31  | EC   | Exception Cause                              |
|      |        |      | 00000 = Stream exception                     |
|      |        |      | 00001 = Unaligned data access exception      |
|      |        |      | 00010 = Illegal op-code exception            |
|      |        |      | 00011 = Instruction bus error exception      |
|      |        |      | 00100 = Data bus error exception             |
|      |        |      | 00101 = Divide exception                     |
|      |        |      | 00110 = Floating point unit exception        |
|      |        |      | 00111 = Privileged instruction exception     |
|      |        |      | 00111 = Stack protection violation exception |
|      |        |      | 10000 = Data storage exception               |
|      |        |      | 10001 = Instruction storage exception        |
|      |        |      | 10010 = Data TLB miss exception              |
|      |        |      | 10011 = Instruction TLB miss exception       |
| 11:5 | 20:26  | ESS  | Exception Specific Status                    |

[UG984/ECR](https://docs.amd.com/r/en-US/ug984-vivado-microblaze-ref/Exception-Status-Register)

### Exception Data Register
| Bits | Manual | Name | Description             |
| ---- | ------ | ---- | ----------------------- |
| 31:0 | 0:31   | EDR  | Exception Data Register |

[UG984/EDR](https://docs.amd.com/r/en-US/ug984-vivado-microblaze-ref/Exception-Data-Register)

## GET
Get from Stream Interface

| prefix      | instruction | parameters | description                      | macro                                    |
| ----------- | ----------- | ---------- | -------------------------------- | ---------------------------------------- |
| *tnea*      | **GET**     | rD, FSLx   | get data from FSL x              | getfslx                                  |
| t           |             |            | test-only                        | tgetfslx                                 |
| n           |             |            | non-blocking                     |                                          |
| e           |             |            | exception if control bit set     |                                          |
| a           |             |            | atomic                           |                                          |
| *tne**c**a* | **GET**     | rD, FSLx   | get control from FSL x           | getfslx  with flags containing 'CONTROL' |
| t           |             |            | test-only                        | tgetfslx with flags containing 'CONTROL' |
| n           |             |            | non-blocking                     |                                          |
| e           |             |            | exception if control bit not set |                                          |
| **c**       |             |            | control                          |                                          |
| a           |             |            | atomic                           |                                          |

Source: [UG984/get](https://docs.amd.com/r/en-US/ug984-vivado-microblaze-ref/get)

### Description
MicroBlaze will read from the FSLx interface and place the result in register rD.

The get instruction has 32 variants (prefix combinations).

When MicroBlaze is configured to use an MMU (C_USE_MMU >= 1) these instructions are privileged. This means that if
these instructions are attempted in User Mode (MSR[UM]=1) a Privileged Instruction exception occurs

#### t: test
The test versions (when ‘t’ bit is ‘1’) will be handled as the normal case, except that the read signal to the FSL
link is not asserted.

#### n: Non blocking
The blocking versions (when ‘n’ bit is ‘0’) will stall microblaze until the data from the FSL interface is valid.
The non-blocking versions will not stall microblaze and will set carry to ‘0’ if the data was valid and to ‘1’ if the
data was invalid. In case of an invalid access the destination register contents is undefined.

#### e: exception
The exception versions (when ‘e’ bit is ‘1’) will generate an exception if there is a control bit mismatch. In this
case ESR is updated with EC set to the exception cause and ESS set to the FSL index. The target register, rD, is not
updated when an exception is generated, instead the FSL data is stored in EDR.

#### c: control
All data get instructions (when ‘c’ bit is ‘0’) expect the control bit from the FSL interface to be ‘0’. If this
is not the case, the instruction will set MSR[FSL_Error] to ‘1’. All control get instructions (when ‘c’ bit is ‘1’)
expect the control bit from the FSL interface to be ‘1’. If this is not the case, the instruction will set
MSR[FSL_Error] to ‘1’.

#### a: atomic
Atomic versions (when ‘a’ bit is ‘1’) are not interruptible. This means that a sequence of atomic FSL
instructions can be grouped together without an interrupt breaking the program flow. However, note
that exceptions may still occur.

### Pseudocode
```
if MSR[UM] = 1 then
   ESR[EC] ← 00111
else
   x ← FSLx
   if x >= C_FSL_LINKS then
        x ← undefined
   (rD) ← Sx_AXIS_TDATA
   if (n = 1) then
        MSR[Carry] ← not(Sx_AXIS_TVALID)
   if Sx_AXIS_TLAST ≠ c and Sx_AXIS_TVALID then
        MSR[FSL] ← 1
        if (e = 1) then
            ESR[EC] ← 00000
            ESR[ESS] ← instruction bits [28:31]
            EDR ← Sx_AXIS_TDATA
```


### Registers Altered
- rD, unless an exception is generated, in which case the register is unchanged
- MSR[FSL_Error]
- MSR[Carry]
- ESR[EC], in case an FSL exception or a privileged instruction exception is generated
- ESR[ESS], in case an FSL exception is generated
- EDR, in case an FSL exception is generated

### Latency
- 1 cycle with C_AREA_OPTIMIZED=0
- 2 cycles with C_AREA_OPTIMIZED=1

The blocking versions of this instruction will stall the pipeline of MicroBlaze until the instruction
can be completed. Interrupts are served when the parameter C_USE_EXTENDED_FSL_INSTR is
set to 1, and the instruction is not atomic.

### Notes
To refer to an FSLx interface in assembly language, use rfsl0, rfsl1, ... rfsl15.

The blocking versions of this instruction should not be placed in a delay slot when the parameter
C_USE_EXTENDED_FSL_INSTR is set to 1, since this prevents interrupts from being served.

For non-blocking versions, an rsubc instruction can be used to decrement an index variable.

The ‘e’ bit does not have any effect unless C_FSL_EXCEPTION is set to 1.

These instructions are only available when the MicroBlaze parameter C_FSL_LINKS is greater than 0.

The extended instructions (exception, test and atomic versions) are only available when the
MicroBlaze parameter C_USE_EXTENDED_FSL_INSTR is set to 1.

## GETD
Get from Stream Interface Dynamic

| prefix      | instruction | parameters | description                      | macro                                     |
| ----------- | ----------- | ---------- | -------------------------------- | ----------------------------------------- |
| *tnea*      | **GETD**    | rD, rB     | get data from FSL rB[28:31]      | getdfslx                                  |
| t           |             |            | test-only                        | tgetdfslx                                 |
| n           |             |            | non-blocking                     |                                           |
| e           |             |            | exception if control bit set     |                                           |
| a           |             |            | atomic                           |                                           |
| *tne**c**a* | **GETD**    | rD, rB     | get control from FSL rB[28:31]   | getdfslx  with flags containing 'CONTROL' |
| t           |             |            | test-only                        | tgetdfslx with flags containing 'CONTROL' |
| n           |             |            | non-blocking                     |                                           |
| e           |             |            | exception if control bit not set |                                           |
| **c**       |             |            | control                          |                                           |
| a           |             |            | atomic                           |                                           |

Source: [UG984/get](https://docs.amd.com/r/en-US/ug984-vivado-microblaze-ref/getd)

### Description
MicroBlaze will read from the FSL interface defined by the four least significant bits in rB and place the result
in register rD.

The getd instruction has 32 variants.

When MicroBlaze is configured to use an MMU (C_USE_MMU >= 1) these instructions are privileged. This means that
if these instructions are attempted in User Mode (MSR[UM] = 1) a Privileged Instruction exception occurs.

#### t: test
The test versions (when ‘t’ bit is ‘1’) will be handled as the normal case, except that the read signal
to the FSL link is not asserted.

#### n: Non blocking
The blocking versions (when ‘n’ bit is ‘0’) will stall microblaze until the data from the FSL interface is valid.
The non-blocking versions will not stall microblaze and will set carry to ‘0’ if the data was valid and to ‘1’ if
the data was invalid. In case of an invalid access the destination register contents is undefined.

#### e: use exception
The exception versions (when ‘e’ bit is ‘1’) will generate an exception if there is a control bit mismatch. In this
case ESR is updated with EC set to the exception cause and ESS set to the FSL index. The target register, rD, is
not updated when an exception is generated, instead the FSL data is stored in EDR.

#### c: control
All data get instructions (when ‘c’ bit is ‘0’) expect the control bit from the FSL interface to be ‘0’. If this is
not the case, the instruction will set MSR[FSL_Error] to ‘1’. All control get instructions (when ‘c’ bit is ‘1’) 
expect the control bit from the FSL interface to be ‘1’. If this is not the case, the instruction will set
MSR[FSL_Error] to ‘1’.

### a: atomic 
Atomic versions (when ‘a’ bit is ‘1’) are not interruptible. This means that a sequence of atomic FSL
instructions can be grouped together without an interrupt breaking the program flow. However, note
that exceptions may still occur.


### Pseudocode
```
if MSR[UM] = 1 then
   ESR[EC] ← 00111
else
   x ← rB[28:31]
   if x >= C_FSL_LINKS then
        x ← undefined
   (rD) ← Sx_AXIS_TDATA
   if (n = 1) then
        MSR[Carry] ← not(Sx_AXIS_TVALID)
   if Sx_AXIS_TLAST ≠ c and Sx_AXIS_TVALID then
        MSR[FSL] ← 1
        if (e = 1) then
            ESR[EC] ← 00000
            ESR[ESS] ← rB[28:31]
            EDR ← Sx_AXIS_TDATA
```

### Registers Altered
- rD, unless an exception is generated, in which case the register is unchanged
- MSR[FSL_Error]
- MSR[Carry]
- ESR[EC], in case an FSL exception or a privileged instruction exception is generated
- ESR[ESS], in case an FSL exception is generated
- EDR, in case an FSL exception is generated

### Latency
- 1 cycle with C_AREA_OPTIMIZED=0
- 2 cycles with C_AREA_OPTIMIZED=1

The blocking versions of this instruction will stall the pipeline of MicroBlaze until the instruction
can be completed. Interrupts are served unless the instruction is atomic, which ensures that the
instruction cannot be interrupted.

### Notes
The blocking versions of this instruction should not be placed in a delay slot, since this prevents interrupts
from being served.
For non-blocking versions, an rsubc instruction can be used to decrement an index variable.
The ‘e’ bit does not have any effect unless C_FSL_EXCEPTION is set to 1.
These instructions are only available when the MicroBlaze parameter C_FSL_LINKS is greater than 0 and the parameter
C_USE_EXTENDED_FSL_INSTR is set to 1.

## PUT
Put to Stream Interface

| prefix            | instruction | parameters | description                    | macro                                    |
| ----------------- | ----------- | ---------- | ------------------------------ | ---------------------------------------- |
| *n a*             | **PUT**     | rA, FSLx   | put data to FSL x              | putfslx                                  |
| n                 |             |            | non-blocking                   |                                          |
| a                 |             |            | atomic                         |                                          |
| **t** *n a*       | **PUT**     | FSLx       | put data to FSL x test-only    | tputfslx                                 |
| **t**             |             |            | **test-only**                  |                                          |
| *n*               |             |            | non-blocking                   |                                          |
| *a*               |             |            | atomic                         |                                          |
| *n* **c** *a*     | **PUT**     | rA, FSLx   | put control to FSL x           | putfslx with flags containing 'CONTROL'  |
| *n*               |             |            | non-blocking                   |                                          |
| **c**             |             |            | **control**                    |                                          |
| *a*               |             |            | atomic                         |                                          |
| ***t** n **c** a* | **PUT**     | FSLx       | put control to FSL x test-only | tputfslx with flags containing 'CONTROL' |
| **t**             |             |            | **test-only**                  |                                          |
| *n*               |             |            | non-blocking                   |                                          |
| **c**             |             |            | **control**                    |                                          |
| *a*               |             |            | atomic                         |                                          |

Source: [UG984/put](https://docs.amd.com/r/en-US/ug984-vivado-microblaze-ref/put)

### Description
MicroBlaze will write the value from register rA to the FSLx interface.

The put instruction has 16 variants.

When MicroBlaze is configured to use an MMU (C_USE_MMU >= 1) these instructions are
privileged. This means that if these instructions are attempted in User Mode (MSR[UM] = 1) a
Privileged Instruction exception occurs.

#### t: test
The test versions (when ‘t’ bit is ‘1’) will be handled as the normal case, except that the write signal
to the FSL link is not asserted (thus no source register is required).

#### n: non Blocking
The blocking versions (when ‘n’ is ‘0’) will stall MicroBlaze until there is space available in the FSL
interface. The non-blocking versions will not stall MicroBlaze and will set carry to ‘0’ if space was
available and to ‘1’ if no space was available.

#### c: control
All data put instructions (when ‘c’ is ‘0’) will set the control bit to the FSL interface to ‘0’ and all
control put instructions (when ‘c’ is ‘1’) will set the control bit to ‘1’.

#### a: atomic
Atomic versions (when ‘a’ bit is ‘1’) are not interruptible. This means that a sequence of atomic FSL
instructions can be grouped together without an interrupt breaking the program flow. However, note
that exceptions may still occur.

### Pseudocode
```
if MSR[UM] = 1 then
    ESR[EC] ← 00111
else
    x ← FSLx
    if x >= C_FSL_LINKS then
        x ← undefined
    if (t = 0) then
        Mx_AXIS_TDATA ← (rA)
    if (n = 1) then
        MSR[Carry] ← Mx_AXIS_TVALID xor not(Mx_AXIS_TREADY)
    if (t = 0) then
        Mx_AXIS_TLAST ← C
```

### Registers altered
- MSR[Carry]
- ESR[EC], in case a privileged instruction exception is generated

### Latency
- 1 cycle with C_AREA_OPTIMIZED=0
- 2 cycles with C_AREA_OPTIMIZED=1

The blocking versions of this instruction will stall the pipeline of MicroBlaze until the instruction
can be completed. Interrupts are served when the parameter C_USE_EXTENDED_FSL_INSTR is
set to 1, and the instruction is not atomic

### Notes
To refer to an FSLx interface in assembly language, use rfsl0, rfsl1, ... rfsl15.

The blocking versions of this instruction should not be placed in a delay slot when the parameter
C_USE_EXTENDED_FSL_INSTR is set to 1, since this prevents interrupts from being served.

These instructions are only available when the MicroBlaze parameter C_FSL_LINKS is greater
than 0.

The extended instructions (atomic versions) are only available when the MicroBlaze parameter
C_USE_EXTENDED_FSL_INSTR is set to 1.

## PUTD
Put to Stream Interface Dynamic

| prefix              | instruction | parameters | description                            | macro                                     |
| ------------------- | ----------- | ---------- | -------------------------------------- | ----------------------------------------- |
| *n a*               | **PUTD**    | rA, rB     | put data to FSL rB[28:31]              | putdfslx                                  |
| *n*                 |             |            | non-blocking                           |                                           |
| *a*                 |             |            | atomic                                 |                                           |
| **t** *n a*         | **PUTD**    | rB         | put data to FSL rB[28:31] test-only    | tputdfslx                                 |
| **t**               |             |            | **test-only**                          |                                           |
| *n*                 |             |            | non-blocking                           |                                           |
| *a*                 |             |            | atomic                                 |                                           |
| *n* **c** *a*       | **PUTD**    | rA, rB     | put control to FSL rB[28:31]           | putdfslx with flags containing 'CONTROL'  |
| *n*                 |             |            | non-blocking                           |                                           |
| **c**               |             |            | **control**                            |                                           |
| *a*                 |             |            | atomic                                 |                                           |
| **t** *n* **c** *a* | **PUTD**    | rB         | put control to FSL rB[28:31] test-only | tputdfslx with flags containing 'CONTROL' |
| **t**               |             |            | **test-only**                          |                                           |
| *n*                 |             |            | non-blocking                           |                                           |
| **c**               |             |            | **control**                            |                                           |
| *a*                 |             |            | atomic                                 |                                           |

Source: [UG984/putd](https://docs.amd.com/r/en-US/ug984-vivado-microblaze-ref/putd)

### Description
MicroBlaze will write the value from register rA to the FSL interface defined by the four least significant bits in rB.

The putd instruction has 16 variants.

When MicroBlaze is configured to use an MMU (C_USE_MMU >= 1) these instructions are
privileged. This means that if these instructions are attempted in User Mode (MSR[UM] = 1) a
Privileged Instruction exception occurs.

#### t: test
The test versions (when ‘t’ bit is ‘1’) will be handled as the normal case, except that the write signal
to the FSL link is not asserted (thus no source register is required).

#### n: non Blocking
The blocking versions (when ‘n’ is ‘0’) will stall MicroBlaze until there is space available in the FSL
interface. The non-blocking versions will not stall MicroBlaze and will set carry to ‘0’ if space was
available and to ‘1’ if no space was available.

#### c: control
All data putd instructions (when ‘c’ is ‘0’) will set the control bit to the FSL interface to ‘0’ and all
control putd instructions (when ‘c’ is ‘1’) will set the control bit to ‘1’.

#### a: atomic
Atomic versions (when ‘a’ bit is ‘1’) are not interruptible. This means that a sequence of atomic FSL
instructions can be grouped together without an interrupt breaking the program flow. However, note
that exceptions may still occur.

### Pseudocode
```
if MSR[UM] = 1 then
    ESR[EC] ← 00111
else
    x ← rB[28:31]
    if x >= C_FSL_LINKS then
        x ← undefined
    if (t = 0) then
        Mx_AXIS_TDATA ← (rA)
    if (n = 1) then
        MSR[Carry]  ← Mx_AXIS_TVALID xor not(Mx_AXIS_TREADY)
    if (t = 0) then
        Mx_AXIS_TLAST ← C
```

### Registers Altered
- MSR[Carry]
- ESR[EC], in case a privileged instruction exception is generated

### Latency
- 1 cycle with C_AREA_OPTIMIZED=0
- 2 cycles with C_AREA_OPTIMIZED=1

The blocking versions of this instruction will stall the pipeline of MicroBlaze until the instruction
can be completed. Interrupts are served unless the instruction is atomic, which ensures that the
instruction cannot be interrupted.

### Notes
The blocking versions of this instruction should not be placed in a delay slot, since this prevents
interrupts from being served.
These instructions are only available when the MicroBlaze parameter C_FSL_LINKS is greater
than 0 and the parameter C_USE_EXTENDED_FSL_INSTR is set to 1.
