#ifndef ELF_H
#define ELF_H

typedef struct __attribute__((__packed__)) ELF_HEADER {

	uint8_t magicNumber;
	uint8_t elf[3];
	uint8_t bits_32_64;
	uint8_t endianness;
	uint8_t headerVersion;
	uint8_t osABI;
	uint8_t padding[8];
	uint16_t type;
	uint16_t instructionSet;
	uint32_t elfVersion;

} ELF_HEADER;

typedef struct __attribute__((__packed__)) ELF_HEADER_32_EXT {

	uint32_t progEntryOffset;
	uint32_t progHeaderTableOffset;
	uint32_t sectionHeaderTableOffset;
	uint32_t flags;
	uint16_t headerSize;
	uint16_t progHTEntrySize;
	uint16_t progHTEntryCount;
	uint16_t sectionHTEntrySize;
	uint16_t sectionHTEntryCount;
	uint16_t sectionIndexStringTable;

} ELF_HEADER_32_EXT;

typedef struct __attribute__((__packed__)) ELF_HEADER_64_EXT {

	uint64_t progEntryOffset;
	uint64_t progHeaderTableOffset;
	uint64_t sectionHeaderTableOffset;
	uint32_t flags;
	uint16_t headerSize;
	uint16_t progHTEntrySize;
	uint16_t progHTEntryCount;
	uint16_t sectionHTEntrySize;
	uint16_t sectionHTEntryCount;
	uint16_t sectionIndexStringTable;

} ELF_HEADER_64_EXT;

typedef struct __attribute__((__packed__)) ELF_PROGRAM_HEADER {

	uint32_t p_type;
	uint32_t p_offset;
	uint32_t p_vaddr;
	uint32_t p_paddr;
	uint32_t p_filesz;
	uint32_t p_memsz;
	uint32_t p_flags;
	uint32_t p_align;

} ELF_PROGRAM_HEADER;

void parse_ELF_PROGRAM_HEADER32(void* buf, ELF_PROGRAM_HEADER* progHeader, ELF_HEADER_32_EXT* elf_header_ext);
void parse_ELF_HEADER_32_EXT(void* buf, ELF_HEADER_32_EXT* elf_header_ext);
void parse_ELF_HEADER(void* buf, ELF_HEADER* elf_header);

void print_ELF_PROGRAM_HEADER(uint16_t* const vga_buffer, size_t* const index, const ELF_PROGRAM_HEADER* const progHeader);
void print_ELF_HEADER_32_EXT(uint16_t* const vga_buffer, size_t* const index, const ELF_HEADER_32_EXT* const elf_header_ext);
void print_ELF_HEADER(uint16_t* const vga_buffer, size_t* const index, const ELF_HEADER* const elf_header);

#endif /* ELF_H */