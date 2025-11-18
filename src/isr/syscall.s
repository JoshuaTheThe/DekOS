        .global idtSysCall
        .extern sysReply
        .equ tempEax   , 0x1000
	.equ tempEbx   , 0x1004
	.equ tempEcx   , 0x1008
	.equ tempEdx   , 0x100C
	.equ tempEsp   , 0x1010
	.equ tempEbp   , 0x1014
	.equ tempEsi   , 0x1018
	.equ tempEdi   , 0x101C
	.equ tempEip   , 0x1020
	.equ tempCs    , 0x1024
	.equ tempDs    , 0x1028
	.equ tempEs    , 0x102C
	.equ tempSs    , 0x1030
	.equ tempFs    , 0x1034
	.equ tempGs    , 0x1038
	.equ tempFl    , 0x103C
	.equ tempStack , 0x2000
idtSysCall:
        cli
	# registers
	movl %eax, (tempEax)
	movl %ebx, (tempEbx)
	movl %ecx, (tempEcx)
	movl %edx, (tempEdx)
	movl %ebp, (tempEbp)
	movl %esi, (tempEsi)
	movl %edi, (tempEdi)
	popl (tempEip)
	popl (tempCs)
	popl (tempFl)
	movl %esp, (tempEsp)
        movl $tempStack, %esp
	# segments
	movw %ds, %ax
	movw %es, %bx
	movw %ss, %cx
	movw %fs, %dx
	movw %gs, %si
	
	movw %ax, (tempDs)
	movw %bx, (tempEs)
	movw %cx, (tempSs)
	movw %dx, (tempFs)
	movw %si, (tempGs)

	movw $0x10, %ax
       	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss
	movw %ax, %fs
	movw %ax, %gs

	call schedSaveContext
        call sysReply
	movl %eax, (tempEax)

        # now we just need to load the registers and jump
	movw (tempDs) , %ax
	movw (tempEs) , %bx
	movw (tempSs) , %cx
	movw (tempFs) , %dx
	movw (tempGs) , %si
	movw %ax, %ds
	movw %bx, %es
	movw %cx, %ss
	movw %dx, %fs
	movw %si, %gs

	movl (tempFl), %eax
        andl $0xFFFFFEFF, %eax
        orl $0x202, %eax
        movl %eax, (tempFl)
	movl (tempEax), %eax
	movl (tempEbx), %ebx
	movl (tempEcx), %ecx
	movl (tempEdx), %edx
	movl (tempEbp), %ebp
	movl (tempEsi), %esi
	movl (tempEdi), %edi
	movl (tempEsp), %esp

	pushl (tempFl)
	pushl (tempCs)
	pushl (tempEip)
        iret
