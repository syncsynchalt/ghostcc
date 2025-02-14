#include "regs.h"
#include "die.h"

reg_t alloc_int_register(regs_in_use_t in_use)
{
    reg_t used = {0};
    int i;
    for (i = 9; i < 16; i++) {
        if (!(in_use.iregs & 1 << i)) {
            used.ireg = i;
            return used;
        }
    }
    die("out of integer registers");
}

reg_t alloc_flt_register(regs_in_use_t in_use)
{
    reg_t used = {0};
    int f;
    for (f = 9; f < 16; f++) {
        if (!(in_use.fregs & 1 << f)) {
            used.freg = f;
            return used;
        }
    }
    die("out of float registers");
}
