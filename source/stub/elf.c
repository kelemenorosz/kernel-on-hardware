#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "elf.h"
#include "memory.h"
#include "print.h"
#include "string.h"

extern const size_t VGA_WIDTH;

void parse_ELF_HEADER(void* buf, ELF_HEADER* elf_header) {

	memcpy((void*)elf_header, buf, sizeof(ELF_HEADER));

	return;

}

void parse_ELF_HEADER_32_EXT(void* buf, ELF_HEADER_32_EXT* elf_header_ext) {

	memcpy((void*)elf_header_ext, buf + sizeof(ELF_HEADER), sizeof(ELF_HEADER_32_EXT));

	return;

}

void parse_ELF_PROGRAM_HEADER32(void* buf, ELF_PROGRAM_HEADER* progHeader, ELF_HEADER_32_EXT* elf_header_ext) {

	memcpy((void*)progHeader, buf + elf_header_ext->progHeaderTableOffset, sizeof(ELF_PROGRAM_HEADER) * elf_header_ext->progHTEntryCount);

	return;

}

void print_ELF_PROGRAM_HEADER(uint16_t* const vga_buffer, size_t* const index, const ELF_PROGRAM_HEADER* const progHeader) {

	size_t index_c = (*index) / VGA_WIDTH;

	print_string(vga_buffer, index, "ELF_PROGRAM_HEADER p_type: ", strlen("ELF_PROGRAM_HEADER p_type: "));
	print_dword(vga_buffer, index, progHeader->p_type);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "ELF_PROGRAM_HEADER p_offset: ", strlen("ELF_PROGRAM_HEADER p_offset: "));
	print_dword(vga_buffer, index, progHeader->p_offset);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "ELF_PROGRAM_HEADER p_vaddr: ", strlen("ELF_PROGRAM_HEADER p_vaddr: "));
	print_dword(vga_buffer, index, progHeader->p_vaddr);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "ELF_PROGRAM_HEADER p_paddr: ", strlen("ELF_PROGRAM_HEADER p_paddr: "));
	print_dword(vga_buffer, index, progHeader->p_paddr);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "ELF_PROGRAM_HEADER p_filesz: ", strlen("ELF_PROGRAM_HEADER p_filesz: "));
	print_dword(vga_buffer, index, progHeader->p_filesz);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "ELF_PROGRAM_HEADER p_memsz: ", strlen("ELF_PROGRAM_HEADER p_memsz: "));
	print_dword(vga_buffer, index, progHeader->p_memsz);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "ELF_PROGRAM_HEADER p_flags: ", strlen("ELF_PROGRAM_HEADER p_flags: "));
	print_dword(vga_buffer, index, progHeader->p_flags);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "ELF_PROGRAM_HEADER p_align: ", strlen("ELF_PROGRAM_HEADER p_align: "));
	print_dword(vga_buffer, index, progHeader->p_align);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	return;

}

void print_ELF_HEADER_32_EXT(uint16_t* const vga_buffer, size_t* const index, const ELF_HEADER_32_EXT* const elf_header_ext) {

	size_t index_c = (*index) / VGA_WIDTH;

	print_string(vga_buffer, index, "ELF_HEADER_32_EXT progEntryOffset: ", strlen("ELF_HEADER_32_EXT progEntryOffset: "));
	print_dword(vga_buffer, index, elf_header_ext->progEntryOffset);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "ELF_HEADER_32_EXT progHeaderTableOffset: ", strlen("ELF_HEADER_32_EXT progHeaderTableOffset: "));
	print_dword(vga_buffer, index, elf_header_ext->progHeaderTableOffset);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "ELF_HEADER_32_EXT sectionHeaderTableOffset: ", strlen("ELF_HEADER_32_EXT sectionHeaderTableOffset: "));
	print_dword(vga_buffer, index, elf_header_ext->sectionHeaderTableOffset);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "ELF_HEADER_32_EXT flags: ", strlen("ELF_HEADER_32_EXT flags: "));
	print_dword(vga_buffer, index, elf_header_ext->flags);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "ELF_HEADER_32_EXT headerSize: ", strlen("ELF_HEADER_32_EXT headerSize: "));
	print_word(vga_buffer, index, elf_header_ext->headerSize);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "ELF_HEADER_32_EXT progHTEntrySize: ", strlen("ELF_HEADER_32_EXT progHTEntrySize: "));
	print_word(vga_buffer, index, elf_header_ext->progHTEntrySize);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "ELF_HEADER_32_EXT progHTEntryCount: ", strlen("ELF_HEADER_32_EXT progHTEntryCount: "));
	print_word(vga_buffer, index, elf_header_ext->progHTEntryCount);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "ELF_HEADER_32_EXT sectionHTEntrySize: ", strlen("ELF_HEADER_32_EXT sectionHTEntrySize: "));
	print_word(vga_buffer, index, elf_header_ext->sectionHTEntrySize);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "ELF_HEADER_32_EXT sectionHTEntryCount: ", strlen("ELF_HEADER_32_EXT sectionHTEntryCount: "));
	print_word(vga_buffer, index, elf_header_ext->sectionHTEntryCount);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "ELF_HEADER_32_EXT sectionIndexStringTable: ", strlen("ELF_HEADER_32_EXT sectionIndexStringTable: "));
	print_word(vga_buffer, index, elf_header_ext->sectionIndexStringTable);
	index_c++;
	*index = index_c * VGA_WIDTH;

	return;

}

void print_ELF_HEADER(uint16_t* const vga_buffer, size_t* const index, const ELF_HEADER* const elf_header) {

	size_t index_c = (*index) / VGA_WIDTH;

	print_string(vga_buffer, index, "ELF_HEADER magicNumber: ", strlen("ELF_HEADER magicNumber: "));
	print_byte(vga_buffer, index, elf_header->magicNumber);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "ELF_HEADER elf: ", strlen("ELF_HEADER elf: "));
	print_string(vga_buffer, index, (char*)&elf_header->elf[0], 0x3);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "ELF_HEADER bits_32_64: ", strlen("ELF_HEADER bits_32_64: "));
	print_byte(vga_buffer, index, elf_header->bits_32_64);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "ELF_HEADER endianness: ", strlen("ELF_HEADER endianness: "));
	print_byte(vga_buffer, index, elf_header->endianness);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "ELF_HEADER headerVersion: ", strlen("ELF_HEADER headerVersion: "));
	print_byte(vga_buffer, index, elf_header->headerVersion);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "ELF_HEADER osABI: ", strlen("ELF_HEADER osABI: "));
	print_byte(vga_buffer, index, elf_header->osABI);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "ELF_HEADER type: ", strlen("ELF_HEADER type: "));
	print_word(vga_buffer, index, elf_header->type);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "ELF_HEADER instructionSet: ", strlen("ELF_HEADER instructionSet: "));
	print_word(vga_buffer, index, elf_header->instructionSet);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "ELF_HEADER elfVersion: ", strlen("ELF_HEADER elfVersion: "));
	print_dword(vga_buffer, index, elf_header->elfVersion);
	index_c++;
	*index = index_c * VGA_WIDTH;

	return;

}