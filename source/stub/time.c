#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

void sleep(uint32_t ticks_to_sleep) {

	uint32_t* PIT_count_down = (uint32_t*)0x30000;
	*PIT_count_down = ticks_to_sleep;
	
	while (*PIT_count_down > 0) {

		asm __volatile__ ("hlt");

	} 

	return;

}