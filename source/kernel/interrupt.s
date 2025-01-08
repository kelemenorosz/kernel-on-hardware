.section .text
.global interrupt_wrapper_spurious
.type interrupt_wrapper_spurious, @function

interrupt_wrapper_spurious:

	push %ebp
	mov %esp, %ebp

	call interrupt_function_spurious

	mov %ebp, %esp 
	pop %ebp
	iret
