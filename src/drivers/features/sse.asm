        global SSEIsAvailable
        global SSEEnable
SSEIsAvailable:
        push ebp
        mov ebp, esp
        ; FPU && SSE2 && SSE && FXSR
        mov eax, 0x01
        cpuid
        test edx, (7<<24) | 1
        jz .noSSE
        jmp .return
.noSSE:
        mov eax, 0x00
.return:
        mov esp, ebp
        pop ebp
        ret
SSEEnable:
        push ebp
        mov ebp, esp
        mov eax, cr0
        and ax, 0xFFFB
        or ax, 0x2
        mov cr0, eax
        mov eax, cr4
        or ax, 3 << 9
        mov cr4, eax
.return:
        mov esp, ebp
        pop ebp
        ret