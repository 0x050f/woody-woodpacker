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
	add rdi, [rel p_offset]
	sub rdi, [rel p_offset]
	mov rdx, 0x7 ; PROT_READ | PROT_WRITE | PROT_EXEC
	syscall

	jmp _key

	_decryption:
	pop rsi

	mov rax, rdi
	sub rax, [rel p_offset]
	add rax, [rel offset]
	mov rcx, 0
	mov rdx, 0
	_decrypt:
		cmp rcx, [rel size]
		jz _end_decrypt
		cmp rdx, [rel key_size]
		jnz _continue_decrypt
		mov rdx, 0
		_continue_decrypt:
		mov r11b, byte[rsi + rdx]
		xor byte[rax + rcx], r11b
		inc rcx
		inc rdx
		jmp _decrypt
	_end_decrypt:

	mov rax, rdi
	add rax, [rel old_entry]
	sub rax, [rel vaddr] ; old_entry depend on vaddr
	pop rdx ; bring back register
	jmp rax ; jump to old_entry

_params:
	vaddr dq 0x0
	p_offset dq 0x0
	offset dq 0x0
	size dq 0x0
	new_entry dq 0x0
	old_entry dq 0x0
	key_size dq 0x0
_key:
	call _decryption
	db ``
