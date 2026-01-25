	section .text
	jmp LispTest.m0
	global LispTest
LispTest:
	push ebp
	mov ebp, esp
	extern printf
	jmp foo.m6
	global foo
foo:
	push ebp
	mov ebp, esp
	push 0
	push dword [ebp+8]
	pop eax
	pop ebx
	cmp eax, ebx
	setg al
	movzx eax, al
	push eax
	test eax, eax
	jz foo.m19
	push 1
	push dword [ebp+8]
	pop eax
	pop ebx
	sub eax, ebx
	push eax
	call foo
	add esp, 4
	push eax
	push dword [ebp+8]
	push _string_1
	call printf
	add esp, 8
	push eax
	jmp foo.m11
foo.m19:
foo.m11:
	pop eax
	mov esp, ebp
	pop ebp
	ret
foo.m6:
	push 10
	call foo
	add esp, 4
	push eax
	pop eax
	mov esp, ebp
	pop ebp
	ret
LispTest.m0:
	section .rodata
_string_1:
	db 72
	db 101
	db 108
	db 108
	db 111
	db 44
	db 32
	db 87
	db 111
	db 114
	db 108
	db 100
	db 44
	db 32
	db 70
	db 114
	db 111
	db 109
	db 32
	db 76
	db 73
	db 83
	db 80
	db 33
	db 32
	db 37
	db 100
	db 10
	db 0
