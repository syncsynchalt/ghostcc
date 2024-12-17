.extern _printf
.global _main
.align 4

_main:
    stp    FP, LR, [sp, #-16]!  // push stack frame

    adrp X0, fmt@PAGE
    add X0, X0, fmt@PAGEOFF

    mov x1, #123
    mov x2, #-2
    mul x3, x1, x2

    // push stack vars (16-byte aligned)
    str x3, [SP, #-16]!

    bl _printf

    add SP, SP, #16             // pop stack vars

    ldp FP, LR, [sp], #16       // pop stack frame

    ldr X0, =0      // return code 0
    ret LR

.data
    fmt: .asciz    "Mult result 123 * -2 = %d\n"
