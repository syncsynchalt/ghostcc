.global _main

_main:
    ldr x9, =2
    ldr x10, =1
    add x9, x9, x10
    mov x0, x9
    ret
