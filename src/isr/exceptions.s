        .global idtPageFaultHandler
        .global idtInvalidOpcodeHandler
        .global idtDivideByZeroHandler
        .global idtGeneralProtectionFaultHandler
        .global idtDefaultHandler
        .global idtDebugHandler
        .global idtNMIHandler
        .global idtBreakpointHandler
        .global idtOverflowHandler
        .global idtBoundRangeHandler
        .global idtDeviceNotAvailableHandler
        .global idtDoubleFaultHandler
        .global idtCoprocessorSegmentOverrunHandler
        .global idtInvalidTSSHandler
        .global idtSegmentNotPresentHandler
        .global idtStackSegmentFaultHandler
        .global idtFloatingPointHandler
        .global idtAlignmentCheckHandler
        .global idtMachineCheckHandler
        .global idtSIMDFloatingPointHandler
        .global idtVirtualizationHandler
        .global idtControlProtectionHandler
        .extern idtDefault
        .extern sysBreakpoint
        .extern schedSaveContext
        .extern schedLoadContext

idtDivideByZeroHandler:
        cli
        push $0x00
        call idtDefault
        add $4, %esp
        sti
        iret

idtDebugHandler:
        cli
        push $0x01
        call idtDefault
        add $4, %esp
        sti
        iret

idtNMIHandler:
        cli
        push $0x02
        call idtDefault
        add $4, %esp
        sti
        iret

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
idtBreakpointHandler:
        cli
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
        call sysBreakpoint
        call schedNextContext
	call schedLoadContext

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

idtOverflowHandler:
        cli
        push $0x04
        call idtDefault
        add $4, %esp
        sti
        iret

idtBoundRangeHandler:
        cli
        push $0x05
        call idtDefault
        add $4, %esp
        sti
        iret

idtInvalidOpcodeHandler:
        cli
        push $0x06
        call idtDefault
        add $4, %esp
        sti
        iret

idtDeviceNotAvailableHandler:
        cli
        push $0x07
        call idtDefault
        add $4, %esp
        sti
        iret

idtDoubleFaultHandler:
        cli
        push $0x08
        call idtDefault
        add $4, %esp
        sti
        iret

idtCoprocessorSegmentOverrunHandler:
        cli
        push $0x09
        call idtDefault
        add $4, %esp
        sti
        iret

idtInvalidTSSHandler:
        cli
        push $0x0A
        call idtDefault
        add $4, %esp
        sti
        iret

idtSegmentNotPresentHandler:
        cli
        push $0x0B
        call idtDefault
        add $4, %esp
        sti
        iret

idtStackSegmentFaultHandler:
        cli
        push $0x0C
        call idtDefault
        add $4, %esp
        sti
        iret

idtGeneralProtectionFaultHandler:
        cli
        push $0x0D
        call idtDefault
        add $4, %esp
        sti
        iret

idtPageFaultHandler:
        cli
        push $0x0E
        call idtDefault
        add $4, %esp
        sti
        iret

idtFloatingPointHandler:
        cli
        push $0x10
        call idtDefault
        add $4, %esp
        sti
        iret

idtAlignmentCheckHandler:
        cli
        push $0x11
        call idtDefault
        add $4, %esp
        sti
        iret

idtMachineCheckHandler:
        cli
        push $0x12
        call idtDefault
        add $4, %esp
        sti
        iret

idtSIMDFloatingPointHandler:
        cli
        push $0x13
        call idtDefault
        add $4, %esp
        sti
        iret

idtVirtualizationHandler:
        cli
        push $0x14
        call idtDefault
        add $4, %esp
        sti
        iret

idtControlProtectionHandler:
        cli
        push $0x15
        call idtDefault
        add $4, %esp
        sti
        iret

idtDefaultHandler:
        cli
        push $0xFF
        call idtDefault
        add $4, %esp
        sti
        iret
