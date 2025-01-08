#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "print.h"
#include "memory.h"

void print_byte(uint16_t* const vga_buffer, size_t* const index, const uint8_t to_print) {

	uint8_t lowBits = to_print & 0x0F;
	uint8_t highBits = (to_print & 0xF0) >> 4;

	if (lowBits < 0xA) lowBits += 0x30; else lowBits += 0x37;
	if (highBits < 0xA) highBits += 0x30; else highBits += 0x37;

	vga_buffer[*index] = 0x0F00 + highBits;
	vga_buffer[*index + 1] = 0x0F00 + lowBits;

	*index += 2;

	return;

}

void print_word(uint16_t* const vga_buffer, size_t* const index, const uint16_t to_print) {

	print_byte(vga_buffer, index, (uint8_t)(to_print >> 0x8));
	print_byte(vga_buffer, index, (uint8_t)to_print);

	return;

}

void print_dword(uint16_t* const vga_buffer, size_t* const index, const uint32_t to_print) {

	print_word(vga_buffer, index, (uint16_t)(to_print >> 0x10));
	print_word(vga_buffer, index, (uint16_t)to_print);

	return;

}

void print_space(uint16_t* const vga_buffer, size_t* const index, const uint32_t space_len) {

	for (size_t i = 0; i < space_len; ++i) {
		vga_buffer[*index] = 0x0F20;
		*index += 1;
	}

	return;

}

void print_string(uint16_t* const vga_buffer, size_t* const index, const char* const string, const uint32_t string_len) {

	for (size_t i = 0; i < string_len; ++i) {

		vga_buffer[*index] = 0x0F00 | string[i]; 
		(*index)++;

	}

	return;

}