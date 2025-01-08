#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "print.h"

void memset(void* ptr, uint8_t value, size_t num) {

	uint8_t* ptr_cast = ptr;
	for (size_t i = 0; i < num; ++i) {

		*ptr_cast = value;
		++ptr_cast;

	}

	return;

}

void memcpy(void* destination, const void* source, size_t num) {

	uint8_t* destination_cast = destination;
	const uint8_t* source_cast = source;

	// uint32_t vga_width = 0x50;
	// uint32_t index_c = 0xB;
	// uint16_t* vga_buffer = (uint16_t*)0xB8000;
	// size_t index = vga_width * index_c;

	// uint8_t* mem_location = (uint8_t*)0x21008;
	// print_byte(vga_buffer, &index, *mem_location);

	for (size_t i = 0; i < num; ++i) {

		// print_dword(vga_buffer, &index, (uint32_t)(destination_cast));
		// print_space(vga_buffer, &index, 0x1);
		// print_dword(vga_buffer, &index, (uint32_t)(source_cast));
		// print_space(vga_buffer, &index, 0x1);
		
		*destination_cast = *source_cast;
		
		// print_byte(vga_buffer, &index, *destination_cast);
		// print_space(vga_buffer, &index, 0x1);
		// print_byte(vga_buffer, &index, *source_cast);

		// index_c++;
		// index = index_c * vga_width;

		++destination_cast;
		++source_cast;

	}

	return;

}