#include "load_elf.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

// open
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <elf.h>
#include "elf_rv.h"

#include "memorymap.h"
#include "mmio.h"
#include "apu.h"

extern int g_elf_debug_level;
extern bool g_elf_force_reloc;
extern bool g_elf_ignore_unaligned;
extern char* g_dump_file;

#define local_debug(level, fmt, ...)      \
    {                                     \
        if (g_elf_debug_level >= (level)) \
            printf((fmt), ##__VA_ARGS__); \
    }


static bool check_alignment(uint32_t offset, int alignment, struct memory_map_entry* mme_offset)
{
    if (((offset & (alignment - 1))) != 0) {
        fprintf(
            stderr,
            "%s: Relocation of unaligned data requested for: %s @ %04x\n",
            g_elf_ignore_unaligned ? "Critical" : "Error",
            mme_offset->name,
            offset);

        if (!g_elf_ignore_unaligned)
            return false;
    }

    return true;
}

#define ERROR "\033[31mError\033[0m: "
#define WARN "\033[33mWarning\033[0m: "
#define CYAN(x) "\033[36m" x "\033[0m"

//  https://github.com/riscv-non-isa/riscv-elf-psabi-doc/blob/master/riscv-elf.adoc
// ## Relocations
//
// 0   NONE                                                 Do nothing
// 1   32                  word32      S + A                32-bit relocation
// 2   64                  word64      S + A                64-bit relocation
// 3   RELATIVE            wordclass   B + A                Adjust a link address (A) to its load address (B + A)
// 4   COPY                                                 Must be in executable; not allowed in shared library
// 5   JUMP_SLOT           wordclass   S                    Indicates the symbol associated with a PLT entry
// 6   TLS_DTPMOD32        word32      TLSMODULE                ?
// 7   TLS_DTPMOD64        word64      TLSMODULE                ?
// 8   TLS_DTPREL32        word32      S + A - TLS_DTV_OFFSET   ?
// 9   TLS_DTPREL64        word64      S + A - TLS_DTV_OFFSET   ?
// 10  TLS_TPREL32         word32      S + A + TLSOFFSET        ?
// 11  TLS_TPREL64         word64      S + A + TLSOFFSET        ?
// 12  TLSDESC             *           TLSDESC(S+A)         *: See TLS Descriptors
// 16  BRANCH              B-Type      S + A - P            12-bit PC-relative branch offset
// 17  JAL                 J-Type      S + A - P            20-bit PC-relative jump offset
// 18  CALL                U+I-Type    S + A - P            Deprecated, please use CALL_PLT instead.
// 19  CALL_PLT            U+I-Type    S + A - P            32-bit PC-relative function call, macros call, tail (PIC)
// 20  GOT_HI20            U-Type      G + GOT + A - P      High 20 bits of 32-bit PC-relative GOT access,
//                                                          %got_pcrel_hi(symbol)
// 21  TLS_GOT_HI20        U-Type                           High 20 bits of 32-bit PC-relative TLS IE GOT access,
//                                                          macro la.tls.ie
// 22  TLS_GD_HI20         U-Type                           High 20 bits of 32-bit PC-relative TLS GD GOT reference,
//                                                          macro la.tls.gd
// 23  PCREL_HI20          U-Type      S + A - P            High 20 bits of 32-bit PC-relative reference,
//                                                          %pcrel_hi(symbol)
// 24  PCREL_LO12_I        I-type      S - P                Low 12 bits of a 32-bit PC-relative,
//                                                          %pcrel_lo(address of %pcrel_hi), the addend must be 0
// 25  PCREL_LO12_S        S-Type      S - P                Low 12 bits of a 32-bit PC-relative,
//                                                          %pcrel_lo(address of %pcrel_hi), the addend must be 0
// 26  HI20                U-Type      S + A                High 20 bits of 32-bit absolute address, %hi(symbol)
// 27  LO12_I              I-Type      S + A                Low 12 bits of 32-bit absolute address, %lo(symbol)
// 28  LO12_S              S-Type      S + A                Low 12 bits of 32-bit absolute address, %lo(symbol)
// 29  TPREL_HI20          U-Type                           High 20 bits of TLS LE thread pointer offset,
//                                                          %tprel_hi(symbol)
// 30  TPREL_LO12_I        I-Type                           Low 12 bits of TLS LE thread pointer offset,
//                                                          %tprel_lo(symbol)
// 31  TPREL_LO12_S        S-Type                           Low 12 bits of TLS LE thread pointer offset,
//                                                          %tprel_lo(symbol)
// 32  TPREL_ADD                                            TLS LE thread pointer usage, %tprel_add(symbol)
// 33  ADD8                word8       V + S + A            8-bit label addition
// 34  ADD16               word16      V + S + A            16-bit label addition
// 35  ADD32               word32      V + S + A            32-bit label addition
// 36  ADD64               word64      V + S + A            64-bit label addition
// 37  SUB8                word8       V - S - A            8-bit label subtraction
// 38  SUB16               word16      V - S - A            16-bit label subtraction
// 39  SUB32               word32      V - S - A            32-bit label subtraction
// 40  SUB64               word64      V - S - A            64-bit label subtraction
// 41  GOT32_PCREL         word32      G + GOT + A - P      32-bit difference between the GOT entry for a symbol and
//                                                          the current location
// 43  ALIGN                                                Alignment statement. The addend indicates the number of
//                                                          bytes occupied by nop instructions at the relocation
//                                                          offset. The alignment boundary is specified by the addend
//                                                          rounded up to the next power of two.
// 44  RVC_BRANCH          CB-Type     S + A - P            8-bit PC-relative branch offset
// 45  RVC_JUMP            CJ-Type     S + A - P            11-bit PC-relative jump offset

// 47  GPREL_I                         S - GP + A
// 48  GPREL_S                         S - GP + A
// 49  TPREL_I                         S - GP + A
// 50  TPREL_S                         S - GP + A

// 51  RELAX                                                Instruction can be relaxed, paired with a normal relocation
//                                                          at the same address
// 52  SUB6                word6       V - S - A            Local label subtraction
// 53  SET6                word6       S + A                Local label assignment
// 54  SET8                word8       S + A                Local label assignment
// 55  SET16               word16      S + A                Local label assignment
// 56  SET32               word32      S + A                Local label assignment
// 57  32_PCREL            word32      S + A - P            32-bit PC relative
// 58  IRELATIVE           wordclass   ifunc_resolv(B + A)  Relocation against a non-preemptible ifunc symbol
// 59  PLT32               word32      S + A - P            32-bit relative offset to a function or its PLT entry
// 60  SET_ULEB128         ULEB128     S + A                Must be placed immediately before a SUB_ULEB128 with the
//                                                          same offset. Local label assignment *note
// 61  SUB_ULEB128         ULEB128     V - S - A            Must be placed immediately after a SET_ULEB128 with the
//                                                          same offset. Local label subtraction *note
// 62  TLSDESC_HI20        U-Type      S + A - P            High 20 bits of a 32-bit PC-relative offset into a TLS
//                                                          descriptor entry, %tlsdesc_hi(symbol)
// 63  TLSDESC_LOAD_LO12   I-Type      S - P                Low 12 bits of a 32-bit PC-relative offset into a TLS
//                                                          descriptor entry, %tlsdesc_load_lo(address of %tlsdesc_hi),
//                                                          the addend must be 0
// 64  TLSDESC_ADD_LO12    I-Type      S - P                Low 12 bits of a 32-bit PC-relative offset into a TLS
//                                                          descriptor entry, %tlsdesc_add_lo(address of %tlsdesc_hi),
//                                                          the addend must be 0
// 65  TLSDESC_CALL                                         Annotate call to TLS descriptor resolver function,
//                                                          %tlsdesc_call(address of %tlsdesc_hi),
//                                                          for relaxation purposes only
//
// *Note: The assembler must allocate sufficient space to accommodate the final value for the R_RISCV_SET_ULEB128
//        and R_RISCV_SUB_ULEB128 relocation pair and fill the space with a single ULEB128-encoded value. This is
//        achieved by prepending the redundant 0x80 byte as necessary. The linker must not alter the length of the
//        ULEB128-encoded value.

// ## Calculation Symbols
// Variables used in relocation calculation provides details on the variables used in relocation calculation:
// Table 5. Variables used in relocation calculation
// Variable     Description
//  A           Addend field in the relocation entry associated with the symbol
//  B           Base address of a shared object loaded into memory
//  G           Offset of the symbol into the GOT (Global Offset Table)
//  GOT         Address of the GOT (Global Offset Table)
//  P           Position of the relocation
//  S           Value of the symbol in the symbol table
//  V           Value at the position of the relocation
//  GP          Value of __global_pointer$ symbol
//  TLSMODULE   TLS module index for the object containing the symbol
//  TLSOFFSET   TLS static block offset (relative to tp) for the object containing the symbol

// ## Global Pointer
// It is assumed that program startup code will load the value of the __global_pointer$ symbol into register gp
// (aka x3).

// ## Field Symbols
// Variables used in relocation fields provides details on the variables used in relocation fields:
// Table 6. Variables used in relocation fields
// Variable     Description
//  word6       Specifies the 6 least significant bits of a word8 field
//  word8       Specifies an 8-bit word
//  word16      Specifies a 16-bit word
//  word32      Specifies a 32-bit word
//  word64      Specifies a 64-bit word
//  ULEB128     Specifies a variable-length data encoded in ULEB128 format.
//  wordclass   Specifies a word32 field for ILP32 or a word64 field for LP64
//  B-Type      Specifies a field as the immediate field in a B-type instruction
//  CB-Type     Specifies a field as the immediate field in a CB-type instruction
//  CI-Type     Specifies a field as the immediate field in a CI-type instruction
//  CJ-Type     Specifies a field as the immediate field in a CJ-type instruction
//  I-Type      Specifies a field as the immediate field in an I-type instruction
//  S-Type      Specifies a field as the immediate field in an S-type instruction
//  U-Type      Specifies a field as the immediate field in an U-type instruction
//  J-Type      Specifies a field as the immediate field in a J-type instruction
//  U+I-Type    Specifies a field as the immediate fields in a U-type and I-type instruction pair

// ## Constants
// Constants used in relocation fields provides details on the constants used in relocation fields:
// Table 7. Constants used in relocation fields
// Name             Value
//  TLS_DTV_OFFSET  0x800

// ## Absolute Addresses
// 32-bit absolute addresses in position dependent code are loaded with a pair of instructions which have an associated
// pair of relocations: R_RISCV_HI20 plus R_RISCV_LO12_I or R_RISCV_LO12_S.
// The R_RISCV_HI20 refers to an LUI instruction containing the high 20-bits to be relocated to an absolute symbol
// address. The LUI instruction is used in conjunction with one or more I-Type instructions (add immediate or load)
// with R_RISCV_LO12_I relocations or S-Type instructions (store) with R_RISCV_LO12_S relocations. The addresses for
// pair of relocations are calculated like this:
//
//  HI20            (symbol_address + 0x800) >> 12
//  LO12	        symbol_address


static inline uint32_t replace_imm(
    uint32_t* instr,
    const uint32_t instr_bit_hi,
    const uint32_t instr_bit_lo,
    const uint32_t imm,
    const uint32_t imm_bit_hi,
    const uint32_t imm_bit_lo)
{
    assert(imm_bit_hi < 32);
    assert(imm_bit_lo < 32);
    assert(imm_bit_lo <= imm_bit_hi);
    assert(instr_bit_hi < 32);
    assert(instr_bit_lo < 32);
    assert(instr_bit_lo <= instr_bit_hi);
    assert((imm_bit_hi - imm_bit_lo) == (instr_bit_hi - instr_bit_lo));

    uint32_t result = *instr;
    uint32_t bits = imm_bit_hi - imm_bit_lo;
    uint32_t mask_bits = (1 << bits) - 1;
    uint32_t imm_masked = (imm >> imm_bit_lo) & mask_bits;
    result &= ~(mask_bits << instr_bit_lo);
    result |= imm_masked << instr_bit_lo;
    *instr = result;
    return result;
}

static inline uint32_t extract_imm(
    const uint32_t instr,
    const uint32_t instr_bit_hi,
    const uint32_t instr_bit_lo,
    const uint32_t imm_bit_hi,
    const uint32_t imm_bit_lo)
{
    assert(imm_bit_hi < 32);
    assert(imm_bit_lo < 32);
    assert(imm_bit_lo <= imm_bit_hi);
    assert(instr_bit_hi < 32);
    assert(instr_bit_lo < 32);
    assert(instr_bit_lo <= instr_bit_hi);
    assert((imm_bit_hi - imm_bit_lo) == (instr_bit_hi - instr_bit_lo));

    uint32_t bits = imm_bit_hi - imm_bit_lo;
    uint32_t mask_bits = (1 << bits) - 1;
    return ((instr >> instr_bit_lo) & mask_bits) << imm_bit_lo;
}

static uint32_t Imm_U_Type(uint32_t instr, uint32_t imm)
{
    // U-Type
    //  - instruction[31:12]= imm[31:12]
    replace_imm(&instr, 31, 12, imm, 31, 12);
    return instr;
}

static uint32_t Get_Imm_U_Type(uint32_t instr)
{
    // U-Type
    //  - imm[31:12] in instruction[31:12]
    return extract_imm(31, 12, instr, 31, 12);
}


static uint32_t Imm_I_Type(uint32_t instr, uint32_t imm)
{
    // I-Type
    //  - instruction[31:20] = imm[11:0]
    replace_imm(&instr, 31, 20, imm, 11, 0);
    return instr;
}

static uint32_t Get_Imm_I_Type(uint32_t instr)
{
    // U-Type
    //  - imm[31:12] is in instruction[31:12]
    return extract_imm(31, 12, instr, 31, 12);
}

static uint32_t Imm_S_Type(uint32_t instr, uint32_t imm)
{
    // S-Type:
    //  - instruction[31:25] = imm[11:5]
    //  - instruction[11:7]  = imm[ 4:0]
    replace_imm(&instr, 31, 25, imm, 11, 5);
    replace_imm(&instr, 11, 7, imm, 4, 0);
    return instr;
}

static uint32_t Get_Imm_S_Type(uint32_t instr)
{
    // S-Type:
    //  - instruction[31:25] = imm[11:5]
    //  - instruction[11:7]  = imm[ 4:0]
    uint32_t res1 = extract_imm(instr, 31, 25, 11, 5);
    uint32_t res2 = extract_imm(instr, 11, 7, 4, 0);
    return res1 | res2;
}

static uint32_t Imm_J_Type(uint32_t instr, uint32_t imm)
{
    // J-Type:
    //  - instruction[31]    = imm[20]
    //  - instruction[30:21] = imm[10:1]
    //  - instruction[20]    = imm[11]
    //  - instruction[19:12] = imm[19:12]
    replace_imm(&instr, 31, 31, imm, 20, 20);
    replace_imm(&instr, 30, 21, imm, 10, 1);
    replace_imm(&instr, 20, 20, imm, 11, 11);
    replace_imm(&instr, 19, 12, imm, 19, 12);
    return instr;
}

static uint32_t Get_Imm_J_Type(uint32_t instr)
{
    // J-Type:
    //  - instruction[31]    = imm[20]
    //  - instruction[30:21] = imm[10:1]
    //  - instruction[20]    = imm[11]
    //  - instruction[19:12] = imm[19:12]
    uint32_t res1 = extract_imm(instr, 31, 31, 20, 20);
    uint32_t res2 = extract_imm(instr, 30, 21, 10, 1);
    uint32_t res3 = extract_imm(instr, 20, 20, 11, 11);
    uint32_t res4 = extract_imm(instr, 19, 12, 19, 12);
    return res1 | res2 | res3 | res4;
}

const uint16_t Offset_CJ_Type(uint16_t cinstr, uint16_t offset)
{
    // CJ-Type:
    //  - instruction[12]   = offset[11]
    //  - instruction[11]   = offset[4]
    //  - instruction[10:7] = offset[9:8]
    //  - instruction[6]    = offset[10]
    //  - instruction[5]    = offset[6]
    //  - instruction[4]    = offset[7]
    //  - instruction[3:1]  = offset[3:1]
    //  - instruction[0]    = offset[5]
    uint32_t instr = cinstr;
    replace_imm(&instr, 12, 12, offset, 11, 11);
    replace_imm(&instr, 11, 11, offset, 4, 4);
    replace_imm(&instr, 10, 9, offset, 9, 8);
    replace_imm(&instr, 8, 8, offset, 10, 10);
    replace_imm(&instr, 7, 7, offset, 6, 6);
    replace_imm(&instr, 6, 6, offset, 7, 7);
    replace_imm(&instr, 5, 3, offset, 3, 1);
    replace_imm(&instr, 2, 2, offset, 5, 5);
    return instr & 0xffff;
}

const uint16_t Get_Offset_CJ_Type(uint16_t cinstr)
{
    // CJ-Type:
    //  - instruction[12]   = offset[11]
    //  - instruction[11]   = offset[4]
    //  - instruction[10:9] = offset[9:8]
    //  - instruction[8]    = offset[10]
    //  - instruction[7]    = offset[6]
    //  - instruction[6]    = offset[7]
    //  - instruction[5:3]  = offset[3:1]
    //  - instruction[2]    = offset[5]
    uint32_t res1 = extract_imm(cinstr, 12, 12, 11, 11);
    uint32_t res2 = extract_imm(cinstr, 11, 11, 4, 4);
    uint32_t res3 = extract_imm(cinstr, 10, 9, 9, 8);
    uint32_t res4 = extract_imm(cinstr, 8, 8, 10, 10);
    uint32_t res5 = extract_imm(cinstr, 7, 7, 6, 6);
    uint32_t res6 = extract_imm(cinstr, 6, 6, 7, 7);
    uint32_t res7 = extract_imm(cinstr, 5, 3, 3, 1);
    uint32_t res8 = extract_imm(cinstr, 2, 2, 5, 5);
    return (res1 | res2 | res3 | res4 | res5 | res6 | res7 | res8) & 0xffff;
}


const uint16_t Offset_CB_Type(uint16_t cinstr, uint16_t offset)
{
    // CB-Type:
    //  - instruction[12]    = offset[8]
    //  - instruction[11:10] = offset[4:3]
    //  - instruction[6:5]   = offset[7:6]
    //  - instruction[4:3]   = offset[2:1]
    //  - instruction[2]     = offset[5]
    uint32_t instr = cinstr;
    replace_imm(&instr, 12, 12, offset, 8, 8);
    replace_imm(&instr, 11, 10, offset, 4, 3);
    replace_imm(&instr, 6, 5, offset, 7, 6);
    replace_imm(&instr, 4, 3, offset, 2, 1);
    replace_imm(&instr, 2, 2, offset, 5, 5);
    return instr & 0xffff;
}

const uint16_t Get_Offset_CB_Type(uint16_t cinstr)
{
    // CB-Type:
    //  - instruction[12]    = offset[8]
    //  - instruction[11:10] = offset[4:3]
    //  - instruction[6:5]   = offset[7:6]
    //  - instruction[4:3]   = offset[2:1]
    //  - instruction[2]     = offset[5]
    uint32_t res1 = extract_imm(cinstr, 12, 12, 8, 8);
    uint32_t res2 = extract_imm(cinstr, 11, 10, 4, 3);
    uint32_t res3 = extract_imm(cinstr, 6, 5, 7, 6);
    uint32_t res4 = extract_imm(cinstr, 4, 3, 2, 1);
    uint32_t res5 = extract_imm(cinstr, 2, 2, 5, 5);
    return (res1 | res2 | res3 | res4 | res5) & 0xffff;
}


static uint32_t Imm_B_Type(uint32_t instr, uint32_t imm)
{
    // B-Type:
    // instruction[31]    = imm[12]
    // instruction[30:25] = imm[10:5]
    // instruction[11:8]  = imm[4:1]
    // instruction[7]     = imm[11]
    replace_imm(&instr, 31, 31, imm, 12, 12);
    replace_imm(&instr, 30, 25, imm, 10, 5);
    replace_imm(&instr, 11, 8, imm, 4, 1);
    replace_imm(&instr, 7, 7, imm, 11, 11);
    return instr;
}

static uint32_t Get_Imm_B_Type(uint32_t instr)
{
    // B-Type:
    // instruction[31]    = imm[12]
    // instruction[30:25] = imm[10:5]
    // instruction[11:8]  = imm[4:1]
    // instruction[7]     = imm[11]
    uint32_t res1 = extract_imm(instr, 31, 31, 12, 12);
    uint32_t res2 = extract_imm(instr, 30, 25, 10, 5);
    uint32_t res3 = extract_imm(instr, 11, 8, 4, 1);
    uint32_t res4 = extract_imm(instr, 7, 7, 11, 11);
    return res1 | res2 | res3 | res4;
}

struct riscv_context {
    uint32_t value;
    uint32_t offset;
};


bool do_rel_rv(struct relocation_context* context, struct memory_map_entry* mme_offset, Elf32_Rela* rela)
{
    Elf32_Sym* symbol = &context->symtab[ELF32_R_SYM(rela->r_info)];
    uint32_t offset = rela->r_offset - mme_offset->apu_linked;

    if (context->private == NULL) {
        context->private = malloc(sizeof(struct riscv_context));
    }

    struct riscv_context* priv = context->private;

    struct memory_map_entry* mme_value = mm_lookup(context->mm, symbol->st_value);
    uint32_t value = symbol->st_value;
    if (mme_value) {
        value -= mme_value->apu_linked;
        value += mme_value->apu_loaded;
    }

    local_debug(
        2,
        "%s symbol \"%s\": bind %d, type %d, other:%d, value: %08x, r_offset: %08x, r_addend: %08x\n",
        mme_offset != mme_value ? "Foreign" : "Local",
        &context->strtab[symbol->st_name],
        ELF32_ST_BIND(symbol->st_info),
        ELF32_ST_TYPE(symbol->st_info),
        symbol->st_other,
        symbol->st_value,
        rela->r_offset,
        rela->r_addend);

    value += rela->r_addend;

    uint32_t old_value;
    uint32_t new_value;

    switch (ELF32_R_TYPE(rela->r_info)) {
        case R_RISCV_NONE:
            // Do nothing
            priv->value += rela->r_addend;
            local_debug(3, "  priv->value is %08x\n", priv->value);

            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_NONE: addend: %u\n", rela->r_addend);
            return true;

        case R_RISCV_RELAX:
            // Do nothing
            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_RELAX: addend: %u\n", rela->r_addend);
            return true;

        case R_RISCV_32:
            // 32-bit relocation
            // word32      S + A
            old_value = ioread32(mme_offset->cpu_virtual, offset);
            new_value = value;
            iowrite32(mme_offset->cpu_virtual, offset, new_value);

            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_32: %08x -> %08x\n", old_value, new_value);
            return true;

        case R_RISCV_64:
            // 64-bit relocation
            // word64      S + A
            old_value = ioread32(mme_offset->cpu_virtual, offset);
            new_value = value;
            iowrite32(mme_offset->cpu_virtual, offset, new_value);

            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_64: %08x -> %08x\n", old_value, new_value);
            fprintf(stderr, WARN "R_RISCV_64: Partial apply of 64-Bit value!\n");
            return true;

        case R_RISCV_BRANCH:
            // 12-bit PC-relative branch offset
            // B-Type      S + A - P
            value -= offset;
            value -= mme_offset->apu_loaded;

            old_value = ioread32(mme_offset->cpu_virtual, offset);
            new_value = Imm_B_Type(old_value, value);
            iowrite32(mme_offset->cpu_virtual, offset, new_value);

            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_BRANCH: PC + %x: %08x -> %08x\n", value, old_value, new_value);
            return true;

        case R_RISCV_JAL:
            // 20-bit PC-relative jump offset
            // J-Type      S + A - P
            value -= offset;
            value -= mme_offset->apu_loaded;

            old_value = ioread32(mme_offset->cpu_virtual, offset);
            new_value = Imm_J_Type(old_value, value);
            iowrite32(mme_offset->cpu_virtual, offset, new_value);

            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_JAL: PC + %x: %08x -> %08x\n", value, old_value, new_value);
            return true;

        case R_RISCV_CALL:
            // Deprecated, please use CALL_PLT instead.
            // U+I-Type    S + A - P
            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_CALL\n");
            return false;

        case R_RISCV_CALL_PLT:
            // 32-bit PC-relative function call, macros call, tail (PIC)
            // U+I-Type    S + A - P
            // value -= offset;
            // value -= mme_offset->apu_loaded;

            local_debug(2, CYAN("%s @ %04x: "), mme_offset->name, offset);
            local_debug(2, "R_RISCV_CALL_PLT: PC + 0x%x: ", value);

            old_value = ioread32(mme_offset->cpu_virtual, offset + 0);
            new_value = Imm_U_Type(old_value, value + 0x800);
            iowrite32(mme_offset->cpu_virtual, offset + 0, new_value);

            local_debug(2, "+0: %08x -> %08x, ", old_value, new_value);

            old_value = ioread32(mme_offset->cpu_virtual, offset + 4);
            new_value = Imm_I_Type(old_value, value);
            iowrite32(mme_offset->cpu_virtual, offset + 4, new_value);
            local_debug(2, "+4: %08x -> %08x\n", old_value, new_value);

            return false;

        case R_RISCV_GOT_HI20:
            // High 20 bits of 32-bit PC-relative GOT access, %got_pcrel_hi(symbol)
            // U-Type      G + GOT + A - P
            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_GOT_HI20\n");
            return false;

        case R_RISCV_PCREL_HI20:
            // High 20 bits of 32-bit PC-relative reference, %pcrel_hi(symbol)
            // U-Type      S + A - P
            value -= offset;
            value -= mme_offset->apu_loaded;

            priv->value = value;
            priv->offset = rela->r_offset;

            old_value = ioread32(mme_offset->cpu_virtual, offset);
            // See notes on "Absolute Addresses" above
            new_value = Imm_U_Type(old_value, value + 0x800);
            iowrite32(mme_offset->cpu_virtual, offset, new_value);

            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_PCREL_HI20: PC + 0x%x: %08x -> %08x\n", value, old_value, new_value);
            return true;

        case R_RISCV_PCREL_LO12_I:
            // Low 12 bits of a 32-bit PC-relative, %pcrel_lo(address of %pcrel_hi), the addend must be 0
            // I-type      S - P
            if (priv->offset != symbol->st_value) {
                fprintf(stderr, ERROR "R_RISCV_PCREL_LO12_I: Unexpected linked instruction\n");
                return false;
            }
            value = priv->value;
            local_debug(3, "  Reusing stored address: 0x%08x\n", value);

            old_value = ioread32(mme_offset->cpu_virtual, offset);
            new_value = Imm_I_Type(old_value, value);
            iowrite32(mme_offset->cpu_virtual, offset, new_value);

            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_PCREL_LO12_I: PC + 0x%x: %08x -> %08x\n", value, old_value, new_value);
            return true;

        case R_RISCV_PCREL_LO12_S:
            // Low 12 bits of a 32-bit PC-relative, %pcrel_lo(address of %pcrel_hi), the addend must be 0
            // S-Type      S - P
            if (priv->offset != symbol->st_value) {
                fprintf(stderr, ERROR "R_RISCV_PCREL_LO12_S: Unexpected linked instruction\n");
                return false;
            }
            value = priv->value;
            local_debug(3, "  Reusing stored address: 0x%08x\n", value);

            old_value = ioread32(mme_offset->cpu_virtual, offset);
            new_value = Imm_S_Type(old_value, value);
            iowrite32(mme_offset->cpu_virtual, offset, new_value);

            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_PCREL_LO12_S: PC + 0x%x: %08x -> %08x\n", value, old_value, new_value);
            return true;

        case R_RISCV_HI20:
            // High 20 bits of 32-bit absolute address,  %hi(symbol)
            // U-Type      S + A
            old_value = ioread32(mme_offset->cpu_virtual, offset);
            // See notes on "Absolute Addresses" above
            new_value = Imm_U_Type(old_value, value + 0x800);
            iowrite32(mme_offset->cpu_virtual, offset, new_value);

            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_HI20: @0x%08x: %08x -> %08x\n", value, old_value, new_value);
            return true;

        case R_RISCV_LO12_I:
            // Low 12 bits of 32-bit absolute address, %lo(symbol)
            // I-Type      S + A
            old_value = ioread32(mme_offset->cpu_virtual, offset);
            new_value = Imm_I_Type(old_value, value);
            iowrite32(mme_offset->cpu_virtual, offset, new_value);

            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_LO12_I: @0x%08x: %08x -> %08x\n", value, old_value, new_value);
            return true;

        case R_RISCV_LO12_S:
            // Low 12 bits of 32-bit absolute address, %lo(symbol)
            // S-Type      S + A
            old_value = ioread32(mme_offset->cpu_virtual, offset);
            new_value = Imm_S_Type(old_value, value);
            iowrite32(mme_offset->cpu_virtual, offset, new_value);

            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_LO12_S: @0x%08x: %08x -> %08x\n", value, old_value, new_value);
            return true;

        case R_RISCV_ADD8:
            // 8-bit label addition
            // word8       V + S + A
            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_ADD8\n");
            return false;

        case R_RISCV_ADD16:
            // 16-bit label addition
            // word16      V + S + A
            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_ADD16\n");
            return false;

        case R_RISCV_ADD32:
            // 32-bit label addition
            // word32      V + S + A

            old_value = ioread32(mme_offset->cpu_virtual, offset);
            new_value = old_value + value;
            iowrite32(mme_offset->cpu_virtual, offset, new_value);

            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_ADD32: %08x + %08x = %08x\n", old_value, value, new_value);
            fprintf(stderr, WARN "Yolo R_RISCV_ADD32 @ %s:%04x\n", mme_offset->name, offset);
            return true;

        case R_RISCV_ADD64:
            // 64-bit label addition
            // word64      V + S + A
            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_ADD64\n");
            return false;

        case R_RISCV_SUB8:
            // 8-bit label subtraction
            // word8       V - S - A
            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_SUB8\n");
            return false;

        case R_RISCV_SUB16:
            // 16-bit label subtraction
            // word16      V - S - A
            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_SUB16\n");
            return false;

        case R_RISCV_SUB32:
            // 32-bit label subtraction
            // word32      V - S - A

            old_value = ioread32(mme_offset->cpu_virtual, offset);
            new_value = old_value - value;
            iowrite32(mme_offset->cpu_virtual, offset, new_value);

            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_SUB32: %08x - %08x -> %08x\n", old_value, value, new_value);
            fprintf(stderr, WARN "Yolo R_RISCV_SUB32 @ %s:%04x\n", mme_offset->name, offset);
            return true;

        case R_RISCV_SUB64:
            // 64-bit label subtraction
            // word64      V - S - A
            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_SUB64\n");
            return false;

        case R_RISCV_RVC_BRANCH:
            // 8-bit PC-relative branch offset
            // CB-Type     S + A - P
            value -= mme_offset->apu_loaded;
            value -= offset;

            if (value > 0x0ff && value < 0xffffff00) {
                fprintf(stderr, ERROR "R_RISCV_RVC_BRANCH: Target %x does not fit in 8 bits!\n", value);
                return false;
            }

            old_value = ioread16(mme_offset->cpu_virtual, offset);
            new_value = Offset_CB_Type(old_value, value);
            iowrite16(mme_offset->cpu_virtual, offset, new_value);

            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_RVC_BRANCH: PC + 0x%03x %04x -> %04x\n", value, old_value, new_value);
            local_debug(
                3,
                "  old offset: %03x, new offset: %03x\n",
                Get_Offset_CB_Type(old_value),
                Get_Offset_CB_Type(new_value));
            return true;

        case R_RISCV_RVC_JUMP:
            // 11-bit PC-relative jump offset
            // CJ-Type     S + A - P
            value -= mme_offset->apu_loaded;
            value -= offset;

            if (value > 0x7ff && value < 0xfffff800) {
                fprintf(stderr, ERROR "R_RISCV_RVC_JUMP: Target %x does not fit in 12 bits!\n", value);
                return false;
            }

            old_value = ioread16(mme_offset->cpu_virtual, offset);
            new_value = Offset_CJ_Type(old_value, value);
            iowrite16(mme_offset->cpu_virtual, offset, new_value);

            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_RVC_JUMP: PC + 0x%03x %04x -> %04x\n", value, old_value, new_value);
            local_debug(
                3,
                "  old offset: %03x, new offset: %03x\n",
                Get_Offset_CJ_Type(old_value),
                Get_Offset_CJ_Type(new_value));
            return true;

        case R_RISCV_GPREL_I:
            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_GPREL_I");
            return false;

        case R_RISCV_SUB6:
            // Local label subtraction
            // word6       V - S - A
            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_SUB6\n");
            return false;

        case R_RISCV_SET6:
            // Local label assignment
            // word6       S + A
            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_SET6\n");
            return false;

        case R_RISCV_SET8:
            // Local label assignment
            // word8       S + A
            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_SET8\n");
            return false;

        case R_RISCV_SET16:
            // Local label assignment
            // word16      S + A
            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_SET16\n");
            return false;

        case R_RISCV_SET32:
            // Local label assignment
            // word32      S + A
            old_value = ioread32(mme_offset->cpu_virtual, offset);
            new_value = symbol->st_value - rela->r_addend;
            iowrite32(mme_offset->cpu_virtual, offset, new_value);

            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_SET32: %08x -> %08x\n", old_value, new_value);
            fprintf(stderr, WARN "Yolo R_RISCV_SET32 @ %s:%04x\n", mme_offset->name, offset);
            return false;

        case R_RISCV_32_PCREL:
            // 32-bit PC relative
            // word32      S + A - P
            value -= mme_offset->apu_loaded;
            value -= offset;

            old_value = ioread32(mme_offset->cpu_virtual, offset);
            new_value = value;
            iowrite32(mme_offset->cpu_virtual, offset, new_value);

            priv->value = new_value;
            priv->offset = rela->r_offset;

            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_32_PCREL: %08x -> %08x\n", old_value, new_value);
            fprintf(stderr, WARN "Yolo R_RISCV_32_PCREL @ %s:%04x\n", mme_offset->name, offset);
            return true;

        case R_RISCV_SET_ULEB128:
            // Must be placed immediately before a SUB_ULEB128 with the same offset. Local label assignment
            // ULEB128     S + A
            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_SET_ULEB128\n");
            return false;

        case R_RISCV_SUB_ULEB128:
            // Must be placed immediately after a SET_ULEB128 with the same offset. Local label subtraction
            // ULEB128     V - S - A
            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_RISCV_SUB_ULEB128\n");
            return false;

        case 68:
        case 69:
            fprintf(
                stderr,
                WARN "Skipping undocumented relocation Type %d @ %s:%04x\n",
                ELF32_R_TYPE(rela->r_info),
                mme_offset->name,
                offset);
            return true;

        default:
            fprintf(
                stderr,
                ERROR "Unknown relocation type %d at %s:%04x\n",
                ELF32_R_TYPE(rela->r_info),
                mme_offset->name,
                offset);
            break;
    }

    return false;
}
