        .global idtTimer
        .extern schedTimerHandler
        .extern current_tss
        .equ tempEax , 0x8000
        .equ tempEsp , tempEax + 4
idtTimer:
        cli
        movl %esp, [tempEsp]
        movl current_tss, %eax
        movl 4(%eax), %esp

        call schedTimerHandler
        movl [tempEsp], %esp
        sti
        iret
