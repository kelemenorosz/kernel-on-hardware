#ifndef PCI_H
#define PCI_H

typedef struct __attribute__((__packed__)) {

	uint8_t class_code;
	uint8_t subclass;
	uint8_t progIF;
	uint8_t revisionID;

} PCI_CLASS;

typedef struct __attribute__((__packed__)) {

	uint8_t latency_timer;
	uint8_t subordinate_bus;
	uint8_t secondary_bus;
	uint8_t primary_bus;

} PCI_BRIDGE_BUS_IDS;

typedef struct __attribute__((__packed__)) {

	uint8_t max_latency;
	uint8_t min_grant;
	uint8_t interrupt_pin;
	uint8_t interrupt_line;

} PCI_INTERRUPT;

void 		set_PCI_offset(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t write_data);
uint32_t 	get_PCI_offset(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);

uint8_t 	get_PCI_header_type(uint8_t bus, uint8_t device, uint8_t function);
uint16_t 	get_PCI_vendorID(uint8_t bus, uint8_t device, uint8_t function);
uint32_t 	get_PCI_BAR0(uint8_t bus, uint8_t device, uint8_t function);

void 		get_PCI_class(uint8_t bus, uint8_t device, uint8_t function, PCI_CLASS* pci_class);
void 		get_PCI_bridge_bus_IDs(uint8_t bus, uint8_t device, uint8_t function, PCI_BRIDGE_BUS_IDS* pci_bridge_bus_ids);
void 		get_PCI_interrupt(uint8_t bus, uint8_t device, uint8_t function, PCI_INTERRUPT* pci_interrupt);

#endif /* PCI_H */