//
// Assembler program to convert a string to
// all upper case.
//
// X0-X2 - parameters to libc functions
// X3 - address of output string
// X4 - address of input string
// W5 - current character being processed
//

.extern _puts
.global _main

_main:
    adrp x4, instr@PAGE
    add x4, x4, instr@PAGEOFF
    adrp x3, outstr@PAGE
    add x3, x3, outstr@PAGEOFF

    // The loop is until byte pointed to by X1 is non-zero
loop:
    LDRB W5, [X4], #1 // load character and increment pointer
    // If W5 > 'z' then goto cont
    CMP W5, #'z' // is letter > 'z'?
    B.GT cont
    // Else if W5 < 'a' then goto end if
    CMP W5, #'a'
    B.LT cont // goto to end if
    // if we got here then the letter is lower case, so convert it.
    SUB W5, W5, #('a'-'A')
cont: // end if

    STRB W5, [X3], #1   // store character to output string
    CMP W5, #0          // check for nul ending
    B.NE loop

    // print outstr
    adrp x0, outstr@PAGE
    add x0, x0, outstr@PAGEOFF
    bl _puts

    ldr x0, #0
    bl _exit

.data
    instr: .asciz "This is our Test String that we will convert"
    outstr: .fill 255, 1, 0
