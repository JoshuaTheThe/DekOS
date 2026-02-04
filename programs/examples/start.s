        bits 32
        section .text
        global _start
        extern Main
_start:
        ; in future, we will get arguments using syscalls, and then wwe have argc+argv
        call Main
        xor eax, eax
        xor ebx, ebx
        int 0x80

