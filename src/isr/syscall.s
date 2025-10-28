        .global idtSysCall
        .extern sysReply
idtSysCall:
        cli
        pushl %edx
        pushl %ecx
        pushl %ebx
        pushl %eax
        call sysReply
        addl $16, %esp
        sti
        iret