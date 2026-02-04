        ; Broader General function for testing ANY* feature
        global FeatureIsPresentEDX
        global FeatureIsPresentECX
        global FeatureCR4Enable
        global FeatureCR0Enable
        global GetCR3
        global SetCR3
FeatureIsPresentEDX:
        push ebp
        mov ebp, esp
        push dword [ebp+8]
        mov eax, 0x01
        cpuid
        pop ecx
        mov eax, 0x01
        shl eax, cl
        test edx, eax
        jz .notPresent
        mov eax, 0x01
        jmp .return
.notPresent:
        mov eax, 0x00
.return:
        mov esp, ebp
        pop ebp
        ret
FeatureIsPresentECX:
        push ebp
        mov ebp, esp
        push dword [ebp+8]
        mov eax, 0x01
        cpuid
        mov edx, ecx ; Test ecx instead of edx
        pop ecx
        mov eax, 0x01
        shl eax, cl
        test edx, eax
        jz .notPresent
        mov eax, 0x01
        jmp .return
.notPresent:
        mov eax, 0x00
.return:
        mov esp, ebp
        pop ebp
        ret

FeatureCR0Enable:
        push ebp
        mov ebp, esp
        mov ebx, cr0
        mov eax, 0x0001
        mov cl, byte [ebp+8]
        shl eax, cl
        or ebx, eax
        mov cr0, ebx
.return:
        mov esp, ebp
        pop ebp
        ret
SetCR3:
        push ebp
        mov ebp, esp
        mov ebx, cr3
        mov eax, 0x0001
        mov cl, byte [ebp+8]
        shl eax, cl
        or ebx, eax
        mov cr3, ebx
.return:
        mov esp, ebp
        pop ebp
        ret
FeatureCR4Enable:
        push ebp
        mov ebp, esp
        mov ebx, cr4
        mov eax, 0x0001
        mov cl, byte [ebp+8]
        shl eax, cl
        or ebx, eax
        mov cr4, ebx
.return:
        mov esp, ebp
        pop ebp
        ret

GetCR3:
        push ebp
        mov ebp, esp
        mov eax, cr3
.return:
        mov esp, ebp
        pop ebp
        ret