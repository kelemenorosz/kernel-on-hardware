#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "print.h"
#include "memory.h"
#include "string.h"
#include "pci.h"
#include "interrupt.h"
#include "io.h"
#include "ahci.h"
#include "cpuid.h"
#include "msr.h"
#include "mtrr.h"
#include "time.h"
#include "keyboard.h"

const size_t VGA_WIDTH = 0x50;
const size_t VGA_HEIGHT = 0x19;

const uint32_t g_memoryMapPtr 	= 0x60000; 

size_t g_consoleIndex = 0x0; 
size_t g_consoleRow = 0x0;
uint16_t* const g_VGABuffer = (uint16_t*)0xB8000;

void kernel_main() {

	cls();

	disable_interrupts();
	
	interrupt_init();
	time_init();
	keyboard_init();
	memory_init();

	// -- Find the ethernet card

	PCI_ENUM_TOKEN* pci_tokens = (PCI_ENUM_TOKEN*)alloc_page(1);
	uint32_t tokens_count = enum_PCI_count();
	enum_PCI(pci_tokens);

	for (PCI_ENUM_TOKEN* token = pci_tokens; token < pci_tokens + tokens_count; ++token) {
		uint32_t class_code = (uint32_t)get_PCI_offset_token(token, 0x8);
		if ((uint8_t)(class_code >> 0x18) == 0x2 && (uint8_t)(class_code >> 0x10) == 0x0) {			
			print_string("Ethernet card. Bus: ");
			print_byte(token->bus);
			print_string(", device: ");
			print_byte(token->device);
			print_string(", function: ");
			print_byte(token->function);
			print_newline();
		}
	}

	// PCI_ENUM_TOKEN ethernet_pci_token = {};
	// PCI_DEVICE_CLASS ethernet_pci_class = {};

	// ethernet_pci_class.classCode = 0x2;
	// ethernet_pci_class.subClass = 0x0;
	// ethernet_pci_class.progIF = 0x0;

	// find_PCI_device_no_progif(&ethernet_pci_token, &ethernet_pci_class);

	// print_string("Ethernet card. Bus: ");
	// print_byte(ethernet_pci_token.bus);
	// print_string(", device: ");
	// print_byte(ethernet_pci_token.device);
	// print_string(", function: ");
	// print_byte(ethernet_pci_token.function);
	// print_newline();

	// uint32_t ethernet_pci_device_vendor = get_PCI_offset_token(&ethernet_pci_token, 0x0);

	// print_string("Ethernet card pci device and vendor ID: ");
	// print_dword(ethernet_pci_device_vendor);
	// print_newline();

	// -- Find and initialize the AHCI controller

	// PCI_ENUM_TOKEN ahci_device_token = {};
	// PCI_DEVICE_CLASS ahci_device_class = {};

	// ahci_device_class.classCode = 0x1;
	// ahci_device_class.subClass = 0x6;
	// ahci_device_class.progIF	= 0x1;

	// find_PCI_device(&ahci_device_token, &ahci_device_class);

	// print_string("AHCI device. Bus: ");
	// print_byte(ahci_device_token.bus);
	// print_string(", device: ");
	// print_byte(ahci_device_token.device);
	// print_string(", function: ");
	// print_byte(ahci_device_token.function);
	// print_newline();

	// uint32_t device_class = get_PCI_offset(ahci_device_token.bus, ahci_device_token.device, ahci_device_token.function, 0x8);

	// print_string("AHCI PCI class code: ");
	// print_dword(device_class);
	// print_newline();

	// AHCI_init(&ahci_device_token);
	// AHCI_print_devices();

	while (true) {
	
		sleep(0x10);
		*(g_VGABuffer + VGA_WIDTH + 0x4F) += 1;

	}


	// cls();

	// uint32_t cpuid_available = 0x0;

	// disable_interrupts();
	// setIDT(g_IDTPtr);
	// cpuid_available = check_cpuid();
	// enable_interrupts();

	// print_string("CPUID available return code: ");
	// print_dword(cpuid_available);
	// print_newline();

	// uint32_t pci_count = enum_PCI_count();
	
	// PCI_ENUM_TOKEN pci_devices[pci_count];
	// enum_PCI(&pci_devices);

	// print_string("Count of PCI devices: ");
	// print_dword(pci_count);
	// print_newline();

	// PCI_ENUM_TOKEN* token = &pci_devices[0];
	// PCI_ENUM_TOKEN* token_end = token + pci_count;
	// PCI_ENUM_TOKEN* ahci_device = NULL;

	// while (token < token_end) {

	// 	uint32_t deviceClass = get_PCI_offset(token->bus, token->device, token->function, 0x8);
	// 	if ((uint8_t)(deviceClass >> 0x18) == 0x1 && (uint8_t)(deviceClass >> 0x10) == 0x6 && (uint8_t)(deviceClass >> 0x8) == 0x1) {
	// 		ahci_device = token;
	// 		break;
	// 	}
	// 	token++;

	// }

	// if (ahci_device == NULL) {
	// 	print_string("AHCI controller not found.");
	// 	print_newline();
	// }
	// else {
	// 	print_string("AHCI controller: ");
	// 	print_byte(ahci_device->bus);
	// 	print_space(1);
	// 	print_byte(ahci_device->device);
	// 	print_space(1);
	// 	print_byte(ahci_device->function);
	// 	print_newline();

	// 	// AHCI_Init(ahci_device);
	// }

	// if (cpuid_available != 0x0) {
	// 	print_string("ID bit not supported in EFLAGS.");
	// 	print_newline();
	// }
	// else {

	// 	uint32_t eax = 0x0;
	// 	uint32_t ebx = 0x0;
	// 	uint32_t ecx = 0x0;
	// 	uint32_t edx = 0x0;

	// 	char cpuid_string[5];
	// 	cpuid_string[4] = '\0';

	// 	get_cpuid(0x0, &eax, &ebx, &ecx, &edx);

	// 	print_string("CPU Vendor ID String: ");
	// 	memcpy((void*)&cpuid_string[0], &ebx, 0x4);
	// 	print_string(cpuid_string);
	// 	memcpy((void*)&cpuid_string[0], &edx, 0x4);
	// 	print_string(cpuid_string);
	// 	memcpy((void*)&cpuid_string[0], &ecx, 0x4);
	// 	print_string(cpuid_string);
	// 	print_newline();

	// 	uint32_t msr_available = check_msr();
	// 	print_string("MSR available return code: ");
	// 	print_dword(msr_available);
	// 	print_newline();

	// 	uint32_t mtrr_available = check_mtrr();
	// 	print_string("MTRR available return code: ");
	// 	print_dword(mtrr_available);
	// 	print_newline();		

	// 	uint32_t mtrrcap_available = check_mtrrcap();
	// 	print_string("MTRRcap available return code: ");
	// 	print_dword(mtrrcap_available);
	// 	print_newline();		

	// 	ecx = 0xFE;
	// 	read_msr(ecx, &eax, &edx);
	// 	print_string("MTRRcap register: ");
	// 	print_dword(edx);		
	// 	print_dword(eax);
	// 	print_newline();		

	// 	ecx = 0x2FF;
	// 	read_msr(ecx, &eax, &edx);
	// 	print_string("MTRRdefType register: ");
	// 	print_dword(edx);		
	// 	print_dword(eax);
	// 	print_newline();		

	// 	for (uint8_t i = 0; i < 7; ++i) {

	// 		ecx = 0x200 + i * 2;
	// 		read_msr(ecx, &eax, &edx);
	// 		print_string("MTRRphysBase ");
	// 		print_byte(i);
	// 		print_string(": ");
	// 		print_dword(edx);		
	// 		print_dword(eax);
	// 		print_newline();		

	// 		ecx = 0x201 + i * 2;
	// 		read_msr(ecx, &eax, &edx);
	// 		print_string("MTRRphysMask ");
	// 		print_byte(i);
	// 		print_string(": ");
	// 		print_dword(edx);		
	// 		print_dword(eax);
	// 		print_newline();		
			
	// 	}

	// }
	
	return;

}
