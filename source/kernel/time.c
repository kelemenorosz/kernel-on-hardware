#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "interrupt.h"
#include "io.h"

uint32_t g_PIT_count_down = 0x0;

void PIT_init();
void interrupt_function_PIT();
extern void interrupt_wrapper_PIT();

void sleep(uint32_t ticks_to_sleep) {

	g_PIT_count_down = ticks_to_sleep;
	
	while (g_PIT_count_down > 0) {

		asm __volatile__ ("hlt");

	} 

	return;

}

/*
Function: 		time_init
Description: 	Initialize time.h: initialize the PIT; register interrupt handler for the PIT.
				Call after interrupt.h has been initialized.
Return:			NONE
*/
void time_init() {

	PIT_init();
	register_interrupt(interrupt_wrapper_PIT, 0x0);
	PIC_line_enable(0x0);

	return;

}

void PIT_init() {

	uint8_t mode_command_PIT = 0b00110110;

	iowriteb(0x43, mode_command_PIT);

	iowriteb(0x40, 0xFF);
	iowriteb(0x40, 0xFF);

	return;

}

void interrupt_function_PIT() {

	if (g_PIT_count_down > 0) g_PIT_count_down--;

	iowriteb(0x20, 0x20);

	return;

}
