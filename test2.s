.extern _printf
.global _main
.align 4

_main:
    stp    x29, LR, [sp, #-16]!     ; Save LR, FR

    adrp X0, fmt@PAGE
    add X0, X0, fmt@PAGEOFF

    mov x1, #123
    mov x2, #124
    mov x3, #125
    str x1, [SP, #-32]!
    str x2, [SP, #8]
    str x3, [SP, #16]

    bl _printf    // call printf

    add SP, SP, #32            ; Clean up stack
    ldp x29, LR, [sp], #16     ; Restore FR, LR

    ldr X0, #0        ; return code
    ret

.data
    fmt: .asciz    "Hello World %d %d %d\n"

.align 4
.text
