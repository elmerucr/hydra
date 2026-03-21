; compiles with vasm 2.0e

        .org            0x8000
        .section        text

start:
        ldr     r2,=0xdeadbeef
        mov     r1,r2
        b       start
        .ltorg
