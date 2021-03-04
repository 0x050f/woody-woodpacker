section.text:
	global _inject:function
	global _get_inject_size:function

_get_inject_size:
	mov rax, _end - _inject
	ret

_inject:
	mov rax, 1
	mov rdi, 1
	mov rsi, msg
	mov rdx, len
	syscall
	mov rax, 60 ; exit
	syscall
	ret

section .data:
	msg db `....WOODY....\n`
	len equ $-msg

_end:
