.global _main
.extern _puts, _printf, _exit

_main:
    adrp x0, msg@PAGE
    add x0, x0, msg@PAGEOFF
    ldr x1, #0
    bl _printf

    ldr x0, #0
    bl _exit

.data
    msg: .asciz "Hello, world!\n"
