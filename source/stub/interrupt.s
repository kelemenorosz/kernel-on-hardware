.section .text
.global interrupt_wrapper_keyboard
.type interrupt_wrapper_keyboard, @function
.global interrupt_wrapper_PIT
.type interrupt_wrapper_PIT, @function
.global interrupt_wrapper_spurious
.type interrupt_wrapper_spurious, @function

interrupt_wrapper_keyboard:

	push %ebp
	mov %esp, %ebp

	call interrupt_function_keyboard

	mov %ebp, %esp 
	pop %ebp
	iret

interrupt_wrapper_PIT:

	push %ebp
	mov %esp, %ebp

	call interrupt_function_PIT

	mov %ebp, %esp 
	pop %ebp
	iret

interrupt_wrapper_spurious:

	push %ebp
	mov %esp, %ebp

	call interrupt_function_spurious

	mov %ebp, %esp 
	pop %ebp
	iret
