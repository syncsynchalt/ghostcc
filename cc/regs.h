#pragma once

typedef struct {
    int ireg; // an integer register
    int freg; // or a float register
} reg_t;

typedef struct {
    int iregs; // bitmask of general-use integer registers in use
    int fregs; // bitmask of general-use float registers in use
} regs_in_use_t;

extern reg_t alloc_int_register(regs_in_use_t r);
extern reg_t alloc_flt_register(regs_in_use_t r);

#define mark_register(regs, reg) do { (regs)->iregs |= (1 << reg.ireg); (regs)->fregs |= (1 << reg.freg); } while (0)
#define unmark_register(regs, reg) do { (regs)->iregs &= ~(1 << reg.ireg); (regs)->fregs &= ~(1 << reg.freg); } while (0)
