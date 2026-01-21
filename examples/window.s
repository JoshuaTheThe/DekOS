	bits 32
	section .text
	global _start
_start:
	; Puts
	mov eax, 8
	mov ebx, hello
	mov ecx, hello.end-hello
	xor edx, edx
	int 0x80

	; X=CreateWindow(Tit;e, 128, 128)
	mov eax, 19
	mov ebx, title
	mov ecx, 128
	mov edx, 128
	int 0x80
        mov edi, eax
	test eax, eax
	jz .error
.mainloop:
        mov eax, 8
	mov ebx, title
	mov ecx, title.end-title
	xor edx, edx
	int 0x80
	jmp .mainloop
        ; ReleaseResource(X)
        mov eax, 19
	mov ebx, edi
	int 0x80
.end:
	xor eax, eax
	xor ebx, ebx
	int 0x80
.error:
	mov eax, 8
	mov ebx, error
	mov ecx, error.end-error
	xor edx, edx
	int 0x80
	jmp .end
	section .data
hello:
	db "Creating Window...", 10
.end:
title:
	db "Window", 0
title.end:
error:
	db "Could not Create Window...", 10
.end:


