.extern _fabsf
.global _main

_main:
    stp    FP, LR, [sp, #-16]!  // push stack frame

    adrp x0, fnum@PAGE
    add x0, x0, fnum@PAGEOFF
    ldr s0, [x0]
    bl _fabsf
    fcvt d0, s0

    adrp X0, fmt@PAGE
    add X0, X0, fmt@PAGEOFF

    // push stack vars (16-byte aligned)
    str d0, [SP, #-16]!

    bl _printf

    add SP, SP, #16             // pop stack vars

    ldp FP, LR, [sp], #16       // pop stack frame

    ldr X0, =0      // return code 0
    ret LR

.data
    fmt: .asciz    "fabs returned %f\n"
    fnum: .single -1.2345
