#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "cpuid.h"

uint32_t check_mtrr() {

	uint32_t eax = 0x0; 
	uint32_t ebx = 0x0; 
	uint32_t ecx = 0x0; 
	uint32_t edx = 0x0; 

	get_cpuid(0x1, &eax, &ebx, &ecx, &edx);

	if (edx & 0x1000) return 0x0;

	return 0x1;

}

uint32_t check_mtrrcap() {

	uint32_t eax = 0x0; 
	uint32_t ebx = 0x0; 
	uint32_t ecx = 0x0; 
	uint32_t edx = 0x0; 

	get_cpuid(0x1, &eax, &ebx, &ecx, &edx);

	uint8_t model 			= (uint8_t)(eax >> 0x4)  & 0xF;
	uint8_t family_id 		= (uint8_t)(eax >> 0x8)  & 0xF;
	uint8_t ext_model 		= (uint8_t)(eax >> 0x10) & 0xF;
	uint8_t ext_family_id	= (uint8_t)(eax >> 0x14);

	uint8_t display_family	= 0x0;
	uint8_t display_model  	= 0x0;

	if (family_id == 0xF) {
		display_family = family_id + ext_family_id;
	}
	else  {
		display_family = family_id; 
	}

	if (family_id == 0x6 || family_id == 0xF) {
		display_model = (ext_model << 0x4) + model;
	}
	else {
		display_model = model;
	}

	if (display_family == 0x6 && display_model >= 0x1) {
		return 0x0;
	}

	if (display_family == 0xF) {
		return 0x0;
	}

	return 0x1;	

}