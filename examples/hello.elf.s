        bits 32
        section .text
        global _start
_start:
        mov eax, 8
        mov ebx, hello
        mov ecx, hello.end-hello
        xor edx, edx
        int 0x80

        xor eax, eax
        xor ebx, ebx
        int 0x80

        section .data
hello:
        db "Hello, World!", 10, 0
.end:

