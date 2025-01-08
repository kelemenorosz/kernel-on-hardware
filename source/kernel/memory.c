#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "print.h"

#define G_HEAP_PAGE_SIZE 0x1000

extern uint32_t* const g_memoryMapPtr; 
uint8_t* g_heapStart 	= 0x0;
uint32_t g_pageCount 	= 0x0;

typedef struct __attribute__((__packed__)) ADDRESS_RANGE_DESCRIPTOR {

	uint32_t baseL;
	uint32_t baseH;
	uint32_t lengthL;
	uint32_t lengthH;
	uint32_t type;
	uint32_t acpi;

} ADDRESS_RANGE_DESCRIPTOR;

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

	for (size_t i = 0; i < num; ++i) {

		*destination_cast = *source_cast;

		++destination_cast;
		++source_cast;

	}

	return;

}

/*
Function:		alloc_page
Description: 	Allocates 'count' contiguous pages.
				Doesn't check for heap space.
Return:			Pointer to first page start.
*/
void* alloc_page(size_t count) {

	void* ptr = (void*)(g_heapStart + g_pageCount * G_HEAP_PAGE_SIZE);
	g_pageCount += count;

	return ptr;

}

/* 
Function: 		memory_init
Description: 	Initialize heap from memory map information.
				Doesn't check for existence of memory map. 
				Sets heap pointer to largest type 1 memory start.
Return:			NONE
*/
void memory_init() {

	uint16_t memory_map_length	= *((uint16_t*)g_memoryMapPtr) - 0x2;
	uint16_t memory_map_count 	= memory_map_length / sizeof(ADDRESS_RANGE_DESCRIPTOR);

	uint8_t* memmap_ptr = ((uint8_t*)g_memoryMapPtr) + 0x2;
	uint32_t maxLen = 0x0;

	for (size_t i = 0; i < memory_map_count; ++i) {
		ADDRESS_RANGE_DESCRIPTOR* addr_range_desc = (ADDRESS_RANGE_DESCRIPTOR*)memmap_ptr;
		if (addr_range_desc->lengthL > maxLen && addr_range_desc->type == 0x1) {
			g_heapStart = (uint8_t*)addr_range_desc->baseL;
			maxLen = addr_range_desc->lengthL;
		}
		memmap_ptr += sizeof(ADDRESS_RANGE_DESCRIPTOR);
	}

	return;

}

void print_memmap() {

	uint16_t memory_map_length	= *((uint16_t*)g_memoryMapPtr) - 0x2;
	uint16_t memory_map_count 	= memory_map_length / sizeof(ADDRESS_RANGE_DESCRIPTOR);
	
	print_string("Memory map entry count: ");
	print_word(memory_map_count);
	print_newline();

	uint8_t* memmap_ptr = ((uint8_t*)g_memoryMapPtr) + 0x2;

	for (size_t i = 0; i < memory_map_count; ++i) {

		ADDRESS_RANGE_DESCRIPTOR* addr_range_desc = (ADDRESS_RANGE_DESCRIPTOR*)memmap_ptr;
		print_dword(addr_range_desc->baseL);
		print_space(0x1);
		print_dword(addr_range_desc->baseH);
		print_space(0x1);
		print_dword(addr_range_desc->lengthL);
		print_space(0x1);
		print_dword(addr_range_desc->lengthH);
		print_space(0x1);
		print_dword(addr_range_desc->type);
		print_space(0x1);
		print_dword(addr_range_desc->acpi);
		print_space(0x1);

		print_newline();
	
		memmap_ptr += sizeof(ADDRESS_RANGE_DESCRIPTOR);

	}

	return;

}