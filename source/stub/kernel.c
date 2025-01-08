#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

const size_t VGA_WIDTH = 0x50;
const size_t VGA_HEIGHT = 0x19;

#include "io.h"
#include "print.h"
#include "interrupt.h"
#include "pci.h"
#include "time.h"
#include "string.h"
#include "ehci.h"
#include "fs.h"
#include "memory.h"
#include "elf.h"

typedef struct __attribute__((__packed__)) idt_entry {

	uint16_t offset_1;
	uint16_t segment_selector;
	uint8_t reserved;
	uint8_t type;
	uint16_t offset_2;

} IDT_ENTRY; 

void print_PCI_header(uint8_t bus, uint8_t device, uint8_t function, uint16_t* vga_buffer, size_t index);

void interrupt_function_keyboard();
void interrupt_function_PIT();
void interrupt_function_spurious();

void loadIDTR(uint32_t memory_location, uint16_t offset);
void remapPICs();
void setIDT(uint32_t memory_location);
void PIT_Init();
void enable_interrupts();

void kernel_main(void) {

	uint16_t first_char = 0x0F59;
	uint16_t second_char = 0xF4F;

	uint16_t* vga_buffer = (uint16_t*)0xB8000;

	for (size_t i = 0; i < VGA_HEIGHT; ++i) {
		for (size_t j = 0; j < VGA_WIDTH; ++j) {
			const size_t index = i * VGA_WIDTH + j;
			vga_buffer[index] = 0x0F20;
		}
	}

	*(vga_buffer + 0x4E) = first_char;
	*(vga_buffer + 0x4F) = second_char;

	*(vga_buffer + VGA_WIDTH + 0x4E) = first_char;
	*(vga_buffer + VGA_WIDTH + 0x4F) = second_char;

	// uint8_t* mem_location = (uint8_t*)0x21008;
	// *(vga_buffer + VGA_WIDTH * 0x2 + 0x4E) = *mem_location + 0x0F00;
	// *(vga_buffer + VGA_WIDTH * 0x2 + 0x4F) = *(mem_location + 0x1) + + 0x0F00;

	uint32_t* PIT_count_down = (uint32_t*)0x30000;
	*PIT_count_down = 0x0;

	//Set up IDT

	loadIDTR(0x70000, 0x7FF);
	remapPICs();
	setIDT(0x70000);
	PIT_Init();
	enable_interrupts();

	size_t index;
	size_t index_c = 0x0;
	index = index_c * VGA_WIDTH;

	uint16_t vendorID = 0x0;
	uint8_t header_type = 0x0;

	PCI_CLASS pci_class = {}; 			

	uint8_t ehci_bus = 0x0;
	uint8_t ehci_device = 0x0;
	uint8_t ehci_function = 0x0;

	for (size_t i = 0; i < 0x100; ++i) {

	 	for (size_t j = 0; j < 0x20; ++j) {

	 		uint8_t bus = (uint8_t)i;
	 		uint8_t device = (uint8_t)j;
			
	 		vendorID = get_PCI_vendorID(bus, device, 0x0);
	 		if (vendorID != 0xFFFF) {

	 			header_type = get_PCI_header_type(bus, device, 0x0);

	 			get_PCI_class(bus, device, 0x0, &pci_class);
	 			if (pci_class.class_code == 0xC && pci_class.subclass == 0x3 && pci_class.progIF == 0x20) {

 					ehci_bus = bus;
	 				ehci_device = device;
	 				ehci_function = 0x0;	

					print_byte(vga_buffer, &index, ehci_bus);
					print_space(vga_buffer, &index, 0x1);
					print_byte(vga_buffer, &index, ehci_device);
					print_space(vga_buffer, &index, 0x1);
					print_byte(vga_buffer, &index, ehci_function);
					print_space(vga_buffer, &index, 0x2);

					uint32_t command_reg = get_PCI_offset(ehci_bus, ehci_device, ehci_function, 0x4);
					print_dword(vga_buffer, &index, command_reg);

					index_c++;
					index = index_c * VGA_WIDTH;

					// uint32_t ehci_bar0 = get_PCI_BAR0(ehci_bus, ehci_device, ehci_function);
					// print_dword(vga_buffer, &index, ehci_bar0);

					EHCI_Reset(ehci_bus, ehci_device, ehci_function, vga_buffer, &index);
					index_c = index / VGA_WIDTH;

	 			}

	 			if (header_type & 0x80) {

	 				for (size_t k = 1; k < 0x8; ++k) {

	 					uint8_t function = (uint8_t)k;
					
	 					vendorID = get_PCI_vendorID(bus, device, function);	
	 					if (vendorID != 0xFFFF) {

	 						get_PCI_class(bus, device, function, &pci_class);
							if (pci_class.class_code == 0xC && pci_class.subclass == 0x3 && pci_class.progIF == 0x20) {

				 				ehci_bus = bus;
				 				ehci_device = device;
				 				ehci_function = function;									

								print_byte(vga_buffer, &index, ehci_bus);
								print_space(vga_buffer, &index, 0x1);
								print_byte(vga_buffer, &index, ehci_device);
								print_space(vga_buffer, &index, 0x1);
								print_byte(vga_buffer, &index, ehci_function);
								print_space(vga_buffer, &index, 0x2);

								uint32_t command_reg = get_PCI_offset(ehci_bus, ehci_device, ehci_function, 0x4);
								print_dword(vga_buffer, &index, command_reg);

								index_c++;
								index = index_c * VGA_WIDTH;

								// uint32_t ehci_bar0 = get_PCI_BAR0(ehci_bus, ehci_device, ehci_function);
								// print_dword(vga_buffer, &index, ehci_bar0);

								EHCI_Reset(ehci_bus, ehci_device, ehci_function, vga_buffer, &index);
								index_c = index / VGA_WIDTH;

				 			}

	 					}

	 				}

	 			}

	 		} 

	 	}

	}

	index = index_c * VGA_WIDTH;
	index_c += 1;

	uint32_t ehci_bar0 = get_PCI_BAR0(ehci_bus, ehci_device, ehci_function);
	uint8_t* capability_registers = (uint8_t*)ehci_bar0;
	// print_byte(vga_buffer, &index, *capability_registers);
	// print_space(vga_buffer, &index, 0x1);
	// print_word(vga_buffer, &index, *(uint16_t*)(capability_registers + 0x2));
	// print_space(vga_buffer, &index, 0x1);
	// print_dword(vga_buffer, &index, *(uint32_t*)(capability_registers + 0x4));
	// print_space(vga_buffer, &index, 0x1);
	// print_dword(vga_buffer, &index, *(uint32_t*)(capability_registers + 0x8));
	// print_space(vga_buffer, &index, 0x1);
	// print_dword(vga_buffer, &index, *(uint32_t*)(capability_registers + 0xC));
	// print_space(vga_buffer, &index, 0x1);

	// index = index_c * VGA_WIDTH;
	// index_c += 1;

	// char* string_to_print = "Second string!";

	// print_string(vga_buffer, &index, "String not found!", strlen("String not found!"));

	// index = index_c * VGA_WIDTH;
	// index_c += 1;

	// print_string(vga_buffer, &index, string_to_print, strlen(string_to_print));
	// print_space(vga_buffer, &index, 0x1);
	// print_dword(vga_buffer, &index, (uint32_t)strlen(string_to_print));

	USB_EHCI_MSB_INTERNALS ehci_msb_internals = {};
	USB_EHCI ehci_s = {};
	USB_EHCI_MSB_QUEUE_HEADS ehci_msb_qhs = {};

	EHCI_Init(ehci_bus, ehci_device, ehci_function, &ehci_msb_internals, &ehci_s);

	EHCI_BulkOnly_Setup(&ehci_msb_internals, &ehci_s, &ehci_msb_qhs);

	index_c = 0x8;
	index = index_c * VGA_WIDTH;

	volatile uint8_t read_buf[0x400];
	EHCI_Read(&ehci_s, &ehci_msb_qhs, (void*)read_buf, 0x0, 0x2);

	print_string(vga_buffer, &index, "Bootsector signature from kernel.c: ", strlen("Bootsector signature from kernel.c: "));
	print_byte(vga_buffer, &index, read_buf[510]);
	print_byte(vga_buffer, &index, read_buf[511]);
	index_c++;
	index = index_c * VGA_WIDTH;

	MBR_DATA mbr_data = {};
	parse_MBR((void*)&read_buf, &mbr_data);

	// Parition 0; Sector 0-1
	EHCI_Read(&ehci_s, &ehci_msb_qhs, (void*)read_buf, mbr_data.partitions[0].lbaPartitionStart, 0x2);
	
	BPB_DATA bpb_data = {};
	EBR_DATA ebr_data = {};
	parse_BPB_EBR((void*)&read_buf, &bpb_data, &ebr_data);

	// Partition 0; Sector of FSInfo
	uint32_t fsInfo_sector = mbr_data.partitions[0].lbaPartitionStart + ebr_data.FSInfoSectorNumber;
	EHCI_Read(&ehci_s, &ehci_msb_qhs, (void*)read_buf, fsInfo_sector, 0x1);

	FSINFO_DATA fsinfo_data = {};
	parse_FSINFO((void*)&read_buf, &fsinfo_data);

	// Partition 0; Sector of root dir
	uint32_t rootDir_sector = mbr_data.partitions[0].lbaPartitionStart + bpb_data.reservedSectors + bpb_data.numberOfFATs * ebr_data.sectorsPerFAT;
	uint32_t cluster_size = bpb_data.bytesPerSector * bpb_data.sectorsPerCluster;
	volatile uint8_t rootDir_buf[cluster_size];

	EHCI_Read(&ehci_s, &ehci_msb_qhs, (void*)&rootDir_buf, rootDir_sector, bpb_data.sectorsPerCluster);

	FILE_DATA file_data = {};
	FILE_QUERY_DESCRIPTOR file_query = {};
	memcpy((void*)&file_query.file_name, (void*)"KERNEL", strlen("KERNEL"));
	memcpy((void*)&file_query.file_ext, (void*)"ELF", strlen("ELF"));
	file_query.file_name_size = strlen("KERNEL");
	file_query.file_ext_size =strlen("ELF");
	
	parse_directory_findA((void*)&rootDir_buf, &file_data, &file_query);

	// Partition 0; First FAT sectors
	uint32_t fat_sector = mbr_data.partitions[0].lbaPartitionStart + bpb_data.reservedSectors;
	read_fileA(&ehci_s, &ehci_msb_qhs, (void*)0x05000000, &file_data, fat_sector, rootDir_sector, bpb_data.sectorsPerCluster);

	uint8_t* kernel_buf = (uint8_t*)0x05000000;

	ELF_HEADER elf_header = {};
	parse_ELF_HEADER(kernel_buf, &elf_header);

	ELF_HEADER_32_EXT elf_header_ext = {};
	parse_ELF_HEADER_32_EXT(kernel_buf, &elf_header_ext);
	//print_ELF_HEADER_32_EXT(vga_buffer, &index, &elf_header_ext);

	ELF_PROGRAM_HEADER programHeaders[elf_header_ext.progHTEntryCount];
	parse_ELF_PROGRAM_HEADER32(kernel_buf, &programHeaders[0], &elf_header_ext);
	for (size_t i = 0; i < elf_header_ext.progHTEntryCount; ++i) {

		print_ELF_PROGRAM_HEADER(vga_buffer, &index, &programHeaders[i]);
		index_c = index / VGA_WIDTH;

	}

	for (size_t i = 0; i < elf_header_ext.progHTEntryCount; ++i) {
		memcpy((void*)programHeaders[i].p_vaddr, kernel_buf + programHeaders[i].p_offset, programHeaders[i].p_filesz);
	}

	void (*core_start)(void) = (void(*)(void))0x0A000000;
	core_start();

	while (true) {
	
		sleep(0x10);
		*(vga_buffer + VGA_WIDTH + 0x4F) += 1;

	}

	
	//asm __volatile__ ("int $0x20");

	return;

}

void PIT_Init() {

	uint8_t mode_command_PIT = 0b00110110;

	iowriteb(0x43, mode_command_PIT);

	iowriteb(0x40, 0xFF);
	iowriteb(0x40, 0xFF);

	return;

}

void loadIDTR(uint32_t memory_location, uint16_t offset) {

	struct {

		uint16_t offset;
		uint32_t memory_location;
		
	} __attribute__((__packed__)) LIDT = {offset, memory_location};

	asm __volatile__ ("lidt %0" :: "m"(LIDT));

	return;

}

void remapPICs() {

	iowriteb(0x20, 0x11);
	iowriteb(0xA0, 0x11);

	iowriteb(0x21, 0x20);
	iowriteb(0xA1, 0x28);

	iowriteb(0x21, 0x04);
	iowriteb(0xA1, 0x02);

	iowriteb(0x21, 0x01);
	iowriteb(0xA1, 0x01);

	iowriteb(0x21, 0xFC);
	iowriteb(0xA1, 0xFF);

	return;

}

void setIDT(uint32_t memory_location) {

	IDT_ENTRY* idt_entry = NULL;

	//Set interrupt wrapper for the PIT
	uint32_t PIT_interrupt_offset = (uint32_t)interrupt_wrapper_PIT;
	idt_entry = (IDT_ENTRY*)(memory_location + 0x100);
	idt_entry->offset_1 = (uint16_t)PIT_interrupt_offset;
	idt_entry->offset_2 = (uint16_t)(PIT_interrupt_offset >> 0x10);
	idt_entry->segment_selector = 0x08;
	idt_entry->reserved = 0x0;
	idt_entry->type = 0x8E;

	//Set interrupt wrapper for the keyboard
	uint32_t keyboard_interrupt_offset = (uint32_t)interrupt_wrapper_keyboard;
	idt_entry = (IDT_ENTRY*)(memory_location + 0x100 + 0x8);
	idt_entry->offset_1 = (uint16_t)keyboard_interrupt_offset;
	idt_entry->offset_2 = (uint16_t)(keyboard_interrupt_offset >> 0x10);
	idt_entry->segment_selector = 0x08;
	idt_entry->reserved = 0x0;
	idt_entry->type = 0x8E;

	//Set interrupt wrapper for spurious interrupts
	uint32_t spurious_interrupt_offset = (uint32_t)interrupt_wrapper_spurious;
	idt_entry = (IDT_ENTRY*)(memory_location + 0x100 + 0x8 * 0x7);
	idt_entry->offset_1 = (uint16_t)spurious_interrupt_offset;
	idt_entry->offset_2 = (uint16_t)(spurious_interrupt_offset >> 0x10);
	idt_entry->segment_selector = 0x08;
	idt_entry->reserved = 0x0;
	idt_entry->type = 0x8E;
	
	return;

}

void enable_interrupts() {

	asm __volatile__ ("sti");

	return;

}

void interrupt_function_keyboard() {

 	uint16_t* vga_buffer = (uint16_t*)0xB8000;

 	uint8_t keyboard_input = ioreadb(0x60);
 	keyboard_input += 1;

 	*(vga_buffer + 0x4F) += 1;
	
 	iowriteb(0x20, 0x20);

 	return;

}

void interrupt_function_PIT() {

	uint32_t* PIT_count_down = (uint32_t*)0x30000;
	if (*PIT_count_down > 0) (*PIT_count_down)--;

	iowriteb(0x20, 0x20);

	return;

}

void interrupt_function_spurious() {

	return;

}

void print_PCI_header(uint8_t bus, uint8_t device, uint8_t function, uint16_t* vga_buffer, size_t index) {

	print_byte(vga_buffer, &index, bus);
	print_space(vga_buffer, &index, 0x1);
	print_byte(vga_buffer, &index, device);
	print_space(vga_buffer, &index, 0x1);
	print_byte(vga_buffer, &index, function);
	print_space(vga_buffer, &index, 0x2);

	PCI_CLASS pci_class = {};
	get_PCI_class(bus, device, function, &pci_class);
				
	print_byte(vga_buffer, &index, pci_class.class_code);
	print_space(vga_buffer, &index, 0x1);
	print_byte(vga_buffer, &index, pci_class.subclass);
	print_space(vga_buffer, &index, 0x1);
	print_byte(vga_buffer, &index, pci_class.progIF);
	print_space(vga_buffer, &index, 0x2);

	uint8_t header_type = get_PCI_header_type(bus, device, function);
	print_byte(vga_buffer, &index, header_type);
	print_space(vga_buffer, &index, 0x2);

	if (header_type & 0x01) {

		PCI_BRIDGE_BUS_IDS pci_bridge_bus_ids = {};
		get_PCI_bridge_bus_IDs(bus, device, function, &pci_bridge_bus_ids);
		print_byte(vga_buffer, &index, pci_bridge_bus_ids.subordinate_bus);
		print_space(vga_buffer, &index, 0x1);
		print_byte(vga_buffer, &index, pci_bridge_bus_ids.secondary_bus);
		print_space(vga_buffer, &index, 0x1);
		print_byte(vga_buffer, &index, pci_bridge_bus_ids.primary_bus);
		
	}
	else {

		PCI_INTERRUPT pci_interrupt = {};
		get_PCI_interrupt(bus, device, function, &pci_interrupt);
		print_byte(vga_buffer, &index, pci_interrupt.interrupt_pin);
		print_space(vga_buffer, &index, 0x1);
		print_byte(vga_buffer, &index, pci_interrupt.interrupt_line);

	}

	return;

}


