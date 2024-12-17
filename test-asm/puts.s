.extern _puts, _exit
.global _main

_main:
    adrp x0, fmt@PAGE
    add x0, x0, fmt@PAGEOFF
    
    bl _puts

    ldr X0, =0        ; return code
    bl _exit

.data
    fmt: .asciz    "Hello World!"
