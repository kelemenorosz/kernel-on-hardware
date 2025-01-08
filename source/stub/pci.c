#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "pci.h"
#include "io.h"

//===========================================================================================================================================================
// Non-exported function declarations

uint32_t 	read_config_data(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
void 		write_config_data(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t write_data);


//===========================================================================================================================================================
// Exported functions

void set_PCI_offset(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t write_data) {

	write_config_data(bus, device, function, offset, write_data);
	return;

}

uint32_t get_PCI_offset(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {

	return (uint32_t)read_config_data(bus, device, function, offset);

}

uint8_t get_PCI_header_type(uint8_t bus, uint8_t device, uint8_t function) {

	return (uint8_t)(read_config_data(bus, device, function, 0xC) >> 16);

}

uint16_t get_PCI_vendorID(uint8_t bus, uint8_t device, uint8_t function) {

	return (uint16_t)read_config_data(bus, device, function, 0x0);

}

uint32_t get_PCI_BAR0(uint8_t bus, uint8_t device, uint8_t function) {

	return (uint32_t)read_config_data(bus, device, function, 0x10);

}

void get_PCI_class(uint8_t bus, uint8_t device, uint8_t function, PCI_CLASS* pci_class) {

	uint32_t pci_class_uint32 = read_config_data(bus, device, function, 0x8);
	pci_class->class_code = (uint8_t)(pci_class_uint32 >> 24);
	pci_class->subclass = (uint8_t)(pci_class_uint32 >> 16);
	pci_class->progIF = (uint8_t)(pci_class_uint32 >> 8);
	pci_class->revisionID = (uint8_t)pci_class_uint32;
	return;

}

void get_PCI_bridge_bus_IDs(uint8_t bus, uint8_t device, uint8_t function, PCI_BRIDGE_BUS_IDS* pci_bridge_bus_ids) {

	uint32_t pci_bridge_bus_ids_uint32 = read_config_data(bus, device, function, 0x18);
	pci_bridge_bus_ids->latency_timer = (uint8_t)(pci_bridge_bus_ids_uint32 >> 24);
	pci_bridge_bus_ids->subordinate_bus = (uint8_t)(pci_bridge_bus_ids_uint32 >> 16);
	pci_bridge_bus_ids->secondary_bus = (uint8_t)(pci_bridge_bus_ids_uint32 >> 8);
	pci_bridge_bus_ids->primary_bus = (uint8_t)pci_bridge_bus_ids_uint32;
	return;

}

void get_PCI_interrupt(uint8_t bus, uint8_t device, uint8_t function, PCI_INTERRUPT* pci_interrupt) {

	uint32_t pci_interrupt_uint32 = read_config_data(bus, device, function, 0x3C);
	pci_interrupt->max_latency = (uint8_t)(pci_interrupt_uint32 >> 24);
	pci_interrupt->min_grant = (uint8_t)(pci_interrupt_uint32 >> 16);
	pci_interrupt->interrupt_pin = (uint8_t)(pci_interrupt_uint32 >> 8);
	pci_interrupt->interrupt_line = (uint8_t)pci_interrupt_uint32;
	return;

}

//===========================================================================================================================================================
// Non-exported function definitions

uint32_t read_config_data(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {

	uint32_t bus_32b = (uint32_t) bus;
	uint32_t device_32b = (uint32_t) device;
	uint32_t function_32b = (uint32_t) function;
	uint32_t offset_32b = (uint32_t) offset;

	uint32_t config_address = (uint32_t)((uint32_t)0x80000000 | (bus_32b << 16) | (device_32b << 11) | (function_32b << 8) | offset_32b);

	iowrite(0xCF8, config_address);
	uint32_t return_value = ioread(0xCFC);

	return return_value;

}

void write_config_data(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t write_data) {

	uint32_t bus_32b = (uint32_t) bus;
	uint32_t device_32b = (uint32_t) device;
	uint32_t function_32b = (uint32_t) function;
	uint32_t offset_32b = (uint32_t) offset;

	uint32_t config_address = (uint32_t)((uint32_t)0x80000000 | (bus_32b << 16) | (device_32b << 11) | (function_32b << 8) | offset_32b);

	iowrite(0xCF8, config_address);
	iowrite(0xCFC, write_data);

	return;

}
