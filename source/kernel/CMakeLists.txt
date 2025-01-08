cmake_minimum_required(VERSION 3.20)

add_executable(
	kernel_core
	kernel_start.s
	ahci.c
	ahci.s
	cpuid.s
	interrupt.c
	interrupt.s
	io.s
	kernel.c
	keyboard.c
	keyboard.s
	memory.c
	msr.c
	msr.s
	mtrr.c
	pci.c
	print.c
	string.c
	time.c
	time.s
	)

set_target_properties(kernel_core PROPERTIES OUTPUT_NAME KERNEL)
set_target_properties(kernel_core PROPERTIES SUFFIX .ELF)
target_compile_options(kernel_core PRIVATE $<$<COMPILE_LANGUAGE:C>:-std=gnu99>)
target_compile_options(kernel_core PRIVATE $<$<COMPILE_LANGUAGE:C>:-Wall>)
target_compile_options(kernel_core PRIVATE $<$<COMPILE_LANGUAGE:C>:-Wextra>)
target_link_options(kernel_core PRIVATE -nostdlib)
target_link_options(kernel_core PRIVATE -lgcc)
target_link_options(kernel_core PRIVATE -T${CMAKE_CURRENT_SOURCE_DIR}/linker.ld)

install(TARGETS kernel_core DESTINATION ${CMAKE_SOURCE_DIR}/)