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

idtBreakpointHandler:
        cli
        push $0x03
        call idtDefault
        add $4, %esp
        sti
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
