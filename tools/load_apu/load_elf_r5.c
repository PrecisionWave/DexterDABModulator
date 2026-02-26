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

#define ERROR "\033[31mError\033[0m: "
#define WARN "\033[33mWarning\033[0m: "
#define CYAN(x) "\033[36m" x "\033[0m"

static uint32_t SExt(uint32_t val, size_t sign_bit)
{
    bool S = val & (1UL << sign_bit) ? 1 : 0;
    uint32_t retval = val & ((1UL << sign_bit) - 1);
    uint32_t signs = ((1UL << (32 - sign_bit)) - 1) << sign_bit;
    return S ? retval | signs : retval;
}

// Branch, A1-Type
//  imm24     = instruction [23:0]
static uint32_t Imm_B_A1_Type(uint32_t instr, uint32_t imm)
{
    replace_imm(&instr, 23, 0, imm, 23, 0);
    return instr;
}

static uint32_t Get_Imm_B_A1_Type(uint32_t instr)
{
    return extract_imm(instr, 23, 0, 23, 0);
}

// Branch, T1-Type
//  tins + 0 [10]     = S     = imm32[24 - 31]
//  tins + 0 [9:0]    = imm10 = imm32[21:12]
//  tins + 1 [13]     = J1    = imm32[22]
//  tins + 1 [11]     = J2    = imm32[23]
//  tins + 1 [10:0]  = imm11  = imm32[11:1]
// I1 = not(J1 xor S)
// I2 = not(J2 xor S)
//  imm32 = SignExtend(S:I1:I2:imm10:imm11:’0’, 32);
static uint32_t Imm_B_T1_Type(uint32_t instr, uint32_t imm)
{
    // 15         0 31       16
    // | instr                |
    // | tins + 0  | tins + 1 |
    // 15         0 15        0
    uint32_t S = imm & (1 << 24);
    uint32_t IJ1_mask = (1 << 22);
    uint32_t IJ2_mask = (1 << 23);
    uint32_t J1 = (~(imm & IJ1_mask) ^ (S ? IJ1_mask : 0)) & IJ1_mask;
    uint32_t J2 = (~(imm & IJ2_mask) ^ (S ? IJ2_mask : 0)) & IJ2_mask;
    replace_imm(&instr, 16 + 13, 16 + 13, J1, 22, 22);
    replace_imm(&instr, 16 + 11, 16 + 11, J2, 23, 23);

    replace_imm(&instr, 10, 10, imm, 24, 24);
    replace_imm(&instr, 9, 0, imm, 21, 12);
    replace_imm(&instr, 16 + 10, 16 + 0, imm, 11, 1);
    return instr;
}

static uint32_t Get_Imm_B_T1_Type(uint32_t instr)
{
    uint32_t S = extract_imm(instr, 10, 10, 24, 24);
    uint32_t J1 = extract_imm(instr, 16 + 13, 16 + 13, 22, 22);
    uint32_t J2 = extract_imm(instr, 16 + 11, 16 + 11, 23, 23);
    uint32_t I1 = extract_imm(S ? ~J1 : J1, 22, 22, 22, 22);
    uint32_t I2 = extract_imm(S ? ~J2 : J2, 23, 23, 23, 23);
    uint32_t imm10 = extract_imm(instr, 9, 0, 21, 12);
    uint32_t imm11 = extract_imm(instr, 16 + 10, 16 + 0, 11, 1);
    return SExt(S + I1 + I2 + imm10 + imm11, 24);
}

// Movt, A1-Type
//  imm12 = imm16[11:0]  = instruction [11:0]
//  imm4  = imm16[15:12] = instruction [19:16]
//  imm16 = imm4:imm12
static uint32_t Imm_MOVT_A1_Type(uint32_t instr, uint32_t imm)
{
    replace_imm(&instr, 11, 0, imm, 11, 0);
    replace_imm(&instr, 19, 16, imm, 15, 12);
    return instr;
}

static uint32_t Get_Imm_MOVT_A1_Type(uint32_t instr)
{
    uint32_t res1 = extract_imm(instr, 11, 0, 11, 0);
    uint32_t res2 = extract_imm(instr, 19, 16, 15, 12);
    return res1 | res2;
}

// Movt, T1-Type
//  tins + 0 [ 3:0]  = imm4  = imm16[15:12]
//  tins + 0 [  10]  = imm   = imm16[11]
//  tins + 1 [14:12] = imm3  = imm16[10:s8]
//  tins + 1 [ 7:0]  = imm8  = imm16[7:0]
//  imm16 = imm4:i:imm3:imm8
static uint32_t Imm_MOVT_T1_Type(uint32_t instr, uint32_t imm)
{
    // 15         0 31       16
    // | instr                |
    // | tins + 0  | tins + 1 |
    // 15         0 15        0

    // tins + 0
    replace_imm(&instr, 10, 10, imm, 11, 11);
    replace_imm(&instr, 3, 0, imm, 15, 12);

    // tins + 1
    replace_imm(&instr, 16 + 7, 16 + 0, imm, 7, 0);
    replace_imm(&instr, 16 + 14, 16 + 12, imm, 10, 8);

    return instr;
}

static uint32_t Get_Imm_MOVT_T1_Type(uint32_t instr)
{
    // 15         0 31       16
    // | instr                |
    // | tins + 0  | tins + 1 |
    // 15         0 15        0
    uint32_t imm = extract_imm(instr, 10, 10, 11, 11);
    uint32_t imm4 = extract_imm(instr, 3, 0, 15, 12);
    uint32_t imm8 = extract_imm(instr, 16 + 7, 16 + 0, 7, 0);
    uint32_t imm3 = extract_imm(instr, 16 + 14, 16 + 12, 10, 8);
    return imm8 | imm3 | imm | imm4;
}


bool do_rel_r5(struct relocation_context* context, struct memory_map_entry* mme_offset, Elf32_Rela* rela)
{
    Elf32_Sym* symbol = &context->symtab[ELF32_R_SYM(rela->r_info)];
    uint32_t offset = rela->r_offset - mme_offset->apu_linked;

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
        case R_ARM_NONE:
            // Do nothing
            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_ARM_NONE: addend: %u\n", rela->r_addend);
            return true;

        case R_ARM_ABS32:
            // Data (S + A) | T
            old_value = ioread32(mme_offset->cpu_virtual, offset);
            new_value = old_value;
            if (mme_value) {
                new_value -= mme_value->apu_linked;
                new_value += mme_value->apu_loaded;
            }
            iowrite32(mme_offset->cpu_virtual, offset, new_value);

            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_ARM_ABS32: %08x -> %08x\n", old_value, new_value);
            return true;

        case R_ARM_PREL31:
            // Data ((S + A) | T) – P
            old_value = ioread32(mme_offset->cpu_virtual, offset);
            local_debug(3, "R_ARM_PREL31: Imm = %d\n", SExt(old_value, 30));
            new_value = old_value;
            iowrite32(mme_offset->cpu_virtual, offset, new_value);

            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_ARM_PREL31: %08x -> %08x\n", old_value, new_value);
            return true;


        case R_ARM_CALL:
        //case R_ARM_JUMP24:
            // ARM ((S + A) | T) – P
            old_value = ioread32(mme_offset->cpu_virtual, offset);
            local_debug(3, "R_ARM_CALL: Diff: %d\n", symbol->st_value - rela->r_offset - 8);
            local_debug(3, "R_ARM_CALL: Imm = %d\n", SExt(Get_Imm_B_A1_Type(old_value), 23));
            value = symbol->st_value - rela->r_offset - 8;
            new_value = Imm_B_A1_Type(old_value, value >> 2);
            iowrite32(mme_offset->cpu_virtual, offset, new_value);

            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_ARM_CALL: %08x ->  %08x\n", old_value, new_value);
            return true;

        case R_ARM_MOVT_ABS:
        //case R_ARM_MOVW_ABS_NC:
            //  ARM     S + A
            old_value = ioread32(mme_offset->cpu_virtual, offset);
            local_debug(3, "R_ARM_MOVT_ABS: Imm = %04x\n", Get_Imm_MOVT_A1_Type(old_value));
            new_value = Imm_MOVT_A1_Type(old_value, value >> 16);
            iowrite32(mme_offset->cpu_virtual, offset, new_value);

            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_ARM_MOVT_ABS: %08x ->  %08x\n", old_value, new_value);
            return true;

        case R_ARM_THM_MOVT_ABS:
        //case R_ARM_THM_MOVW_ABS_NC:
            // Thumb32 S + A
            old_value = ioread32(mme_offset->cpu_virtual, offset);
            local_debug(3, "R_ARM_THM_MOVT_ABS: Imm = %04x\n", Get_Imm_MOVT_T1_Type(old_value));
            new_value = Imm_MOVT_T1_Type(old_value, value >> 16);
            local_debug(3, "R_ARM_THM_MOVT_ABS: Imm = %04x\n", Get_Imm_MOVT_T1_Type(new_value));
            iowrite32(mme_offset->cpu_virtual, offset, new_value);

            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_ARM_THM_MOVT_ABS: %08x ->  %08x\n", old_value, new_value);
            return true;

        /* R_ARM_THM_PC22 or R_ARM_THM_CALL */
        case 10:
        //case R_ARM_THM_JUMP24:
            // Thumb32 ((S + A) | T) – P
            if ( ELF32_ST_BIND(symbol->st_info) == STB_WEAK && symbol->st_value == 0) {
                fprintf(stderr, "R_ARM_THM_CALL: Skipping unavailable weak symbol!");
                return true;
            }
            old_value = ioread32(mme_offset->cpu_virtual, offset);
            local_debug(3, "R_ARM_THM_CALL: Imm = %x\n", Get_Imm_B_T1_Type(old_value));
            value = symbol->st_value - rela->r_offset - 4;
            new_value = Imm_B_T1_Type(old_value, value);
            local_debug(3, "R_ARM_THM_CALL: Imm = %x\n", Get_Imm_B_T1_Type(new_value));
            iowrite32(mme_offset->cpu_virtual, offset, new_value);

            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_ARM_THM_CALL: %08x ->  %08x\n", old_value, new_value);
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
