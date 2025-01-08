cmake_minimum_required(VERSION 3.20)

add_executable(
	kernel_stub
	kernel_start.s
	elf.c
	ehci.c
	fs.c
	kernel.c
	memory.c
	pci.c
	print.c
	string.c
	time.c
	interrupt.s
	io.s
	)

set_target_properties(kernel_stub PROPERTIES OUTPUT_NAME STUB)
set_target_properties(kernel_stub PROPERTIES SUFFIX .ELF)
target_compile_options(kernel_stub PRIVATE $<$<COMPILE_LANGUAGE:C>:-std=gnu99>)
target_compile_options(kernel_stub PRIVATE $<$<COMPILE_LANGUAGE:C>:-Wall>)
target_compile_options(kernel_stub PRIVATE $<$<COMPILE_LANGUAGE:C>:-Wextra>)
target_link_options(kernel_stub PRIVATE -nostdlib)
target_link_options(kernel_stub PRIVATE -lgcc)
target_link_options(kernel_stub PRIVATE -T${CMAKE_CURRENT_SOURCE_DIR}/linker.ld)

install(TARGETS kernel_stub DESTINATION ${CMAKE_SOURCE_DIR}/)