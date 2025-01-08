.section .text
.global interrupt_wrapper_PIT
.type interrupt_wrapper_PIT, @function

interrupt_wrapper_PIT:

	push %ebp
	mov %esp, %ebp

	call interrupt_function_PIT

	mov %ebp, %esp 
	pop %ebp
	iret
