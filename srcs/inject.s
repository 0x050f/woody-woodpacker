section.text:
	global _start:function

_start:
	push rdx
	jmp _message

_inject:
	mov rax, 1 ; write
	mov rdi, 1
	pop rsi
	mov rdx, 0
	_count_char:
		cmp byte [rsi + rdx], 0x0
		jz _end_count_char
		inc rdx
		jmp _count_char
	_end_count_char:
	syscall
	jmp _unpack

_message:
	call _inject
	db `....WOODY....\n`, 0x0


_unpack:
	mov rax, 10 ; mprotect
	mov rdi, [rel paged_offset]
	mov rsi, [rel offset]
	sub rsi, rdi
	mov rdx, [rel size]
	add rsi, rdx
	mov rdx, 0x7 ; PROT_READ | PROT_WRITE
	syscall

	mov rax, [rel offset]
	mov rcx, [rel size]
	add rcx, rax
	mov rdx, [rel key]
	_loop_xor:
		cmp rax, rcx
		jz _end_loop_xor
		xor byte[rax], dl
		ror rdx, 8
		inc rax
		jmp _loop_xor
	_end_loop_xor:

	mov rax, 10 ; mprotect
	mov rdi, [rel paged_offset]
	mov rsi, [rel offset]
	sub rsi, rdi
	mov rdx, [rel size]
	add rsi, rdx
	mov rdx, 0x5 ; PROT_READ | PROT_EXEC
	syscall

	mov rax, [rel offset]
	pop rdx
	jmp rax

_params_xor:
	paged_offset dq 0x9999999999999999
	offset dq 0xAAAAAAAAAAAAAAAA
	size dq 0xBBBBBBBBBBBBBBBB
	key dq 0xCCCCCCCCCCCCCCCC
