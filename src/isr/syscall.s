        .global idtSysCall
        .extern sysReply
        .equ tempEax   , 0x9000
	.equ tempEbx   , 0x9004
	.equ tempEcx   , 0x9008
	.equ tempEdx   , 0x900C
	.equ tempEsp   , 0x9010
	.equ tempEbp   , 0x9014
	.equ tempEsi   , 0x9018
	.equ tempEdi   , 0x901C
	.equ tempEip   , 0x9020
	.equ tempCs    , 0x9024
	.equ tempDs    , 0x9028
	.equ tempEs    , 0x902C
	.equ tempSs    , 0x9030
	.equ tempFs    , 0x9034
	.equ tempGs    , 0x9038
	.equ tempFl    , 0x903C
	.equ tempStack , 0x8000
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
