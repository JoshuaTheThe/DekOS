        .extern main
        .extern exit
        .global _start
_start:
        mov $stack_top, %esp
        and $-16, %esp        # 16-byte alignment
        pushl %edx
        pushl %ecx
        pushl %ebx
        pushl %eax
        call main
        add $16, %esp
        push %eax
        call exit
        .section .bss
        .align 16
stack:
        .skip 16384
stack_top: