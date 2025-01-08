.section .text
.global _start
.type _start, @function

_start:

	mov $0x0A000000, %eax
	mov %eax, %esp 

	call kernel_main
	
hang:
	
	hlt
	jmp hang
