#ifndef PRINT_H
#define PRINT_H

void print_dword(uint16_t* const vga_buffer, size_t* const index, const uint32_t to_print);
void print_word(uint16_t* const vga_buffer, size_t* const index, const uint16_t to_print);
void print_byte(uint16_t* const vga_buffer, size_t* const index, const uint8_t to_print);
void print_space(uint16_t* const vga_buffer, size_t* const index, const uint32_t space_len);
void print_string(uint16_t* const vga_buffer, size_t* const index, const char* const string, const uint32_t string_len);

#endif /* PRINT_H */