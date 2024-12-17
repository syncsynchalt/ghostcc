.extern _printf
.global _main
.align 4

_main:
    stp    FP, LR, [sp, #-16]!  // push stack frame

    adrp X0, fmt@PAGE
    add X0, X0, fmt@PAGEOFF

    mov x1, #123
    mov x2, #124
    mov x3, #125

    // push stack vars (16-byte aligned)
    sub SP, SP, #32
    str x1, [SP, #0]
    str x2, [SP, #8]
    str x3, [SP, #16]

    bl _printf

    add SP, SP, #32             // pop stack vars

    ldp FP, LR, [sp], #16       // pop stack frame

    ldr X0, =0      // return code 0
    ret LR

.data
    fmt: .asciz    "Hello World %d %d %d\n"

.align 4
.text
