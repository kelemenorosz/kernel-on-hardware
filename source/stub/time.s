.section .text
.global PIT_Interrupt
.type PIT_Interrupt, @function
.global Sleep
.type Sleep, @function

PIT_Interrupt:

	push %ebp
	mov %esp, %ebp
	push %eax
	push %edx

	mov PIT_Count_Down, %eax
	cmp %eax, 0x0
	je PIT_Interrupt_exit
	dec %eax
	mov %eax, PIT_Count_Down

	xor %edx, %edx
	mov $0xB81FE, %eax
	mov $0x0F, %dh
	mov PIT_Count_Down, %dl
	mov %dx, (%eax)

PIT_Interrupt_exit: 

	mov $0x20, %eax
	mov $0x20, %edx
	out %al, (%dx)

	pop %edx
	pop %eax
	mov %ebp, %esp
	pop %ebp
	iret

Sleep:

	push %ebp
	mov %esp, %ebp
	push %eax
	push %edx

	mov 0x8(%ebp), %eax
	mov %eax, PIT_Count_Down

Sleep_continue:

	mov PIT_Count_Down, %eax
	cmp %eax, 0x0
	jne Sleep_halt
	jmp Sleep_exit
	
Sleep_halt:
	hlt
	xor %edx, %edx
	mov $0xB81FC, %eax
	mov (%eax), %dx
	dec %edx
	mov %dx, (%eax)
	jmp Sleep_continue

Sleep_exit:

	pop %edx
	pop %eax
	mov %ebp, %esp
	pop %ebp
	ret
