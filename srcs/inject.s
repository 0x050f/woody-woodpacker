section.text:
	global _start:function

_start:
	call _inject ; push addr to stack
	db `....WOODY....\n`, 0x0

_ft_strlen:
	mov rax, 0
	_count_char:
		cmp byte [rsi + rax], 0
		jz _end_count_char
		inc rax
		jmp _count_char
	_end_count_char:
	ret

_inject:
	pop rsi ; pop addr from stack
	push rdx ; save register

	call _ft_strlen
	mov rdx, rax
	mov rax, 1 ; write
	mov rdi, 1
	syscall

	sub rsi, 0x5 ; sub size of call instruction

	mov rax, 10 ; mprotect
	mov rdi, rsi
	mov rsi, [rel new_entry]
	sub rsi, [rel vaddr]
	sub rdi, rsi
	mov rdx, 0x7 ; PROT_READ | PROT_WRITE | PROT_EXEC
	syscall

	mov rax, rdi
	add rax, [rel offset]
	mov rcx, 0
	mov rdx, [rel key]
	_decrypt:
		cmp rcx, [rel size]
		jz _end_decrypt
		xor byte[rax + rcx], dl
		ror rdx, 8
		inc rcx
		jmp _decrypt
	_end_decrypt:

	mov rax, rdi
	add rax, [rel old_entry]
	pop rdx ; get register
	jmp rax

_params_xor:
	vaddr dq 0x9999999999999999
	offset dq 0xAAAAAAAAAAAAAAAA
	size dq 0xBBBBBBBBBBBBBBBB
	new_entry dq 0xDDDDDDDDDDDDDDDD
	old_entry dq 0xEEEEEEEEEEEEEEEE
	key dq 0xCCCCCCCCCCCCCCCC
