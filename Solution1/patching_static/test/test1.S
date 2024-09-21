.intel_syntax noprefix
.global WinMain
.text
WinMain:
	mov rax, 5
	push rax
	or eax, 0x1
	pop rax
	jno end
	nop
	nop
end:
	add rax, rax
	
