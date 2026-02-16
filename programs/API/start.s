        .extern main
        .extern exit
        .global _start
_start:
        and $0xfffffff0, %esp
        pushl %ebx
        pushl %eax
        pushl %edx
        pushl %ecx
        call main
        add $16, %esp
        push %eax
        call exit