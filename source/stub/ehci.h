#ifndef EHCI_H
#define EHCI_H

typedef struct __attribute__((__packed__)) USB_EHCI_MSB_QUEUE_HEADS {

	uint32_t ehci_bulk_out_qh;
	uint32_t ehci_bulk_in_qh;
	uint32_t ehci_bulk_out_token;
	uint32_t ehci_bulk_in_token;

} USB_EHCI_MSB_QUEUE_HEADS;

typedef struct __attribute__((__packed__)) USB_EHCI_MSB_INTERNALS {

	uint32_t ehci_addr;
	uint32_t ehci_bulk_endp_addr_out;
	uint32_t ehci_bulk_endp_addr_in;
	uint32_t ehci_bulk_endp_max_s_out;
	uint32_t ehci_bulk_endp_max_s_in;
	uint32_t ehci_bulk_interface_nr;

} USB_EHCI_MSB_INTERNALS;

typedef struct __attribute__((__packed__)) USB_EHCI {

	uint32_t ehci_capreg;
	uint32_t ehci_opreg_core;
	uint32_t ehci_opreg_aux;	

} USB_EHCI;

void EHCI_Init(uint8_t bus, uint8_t device, uint8_t function, USB_EHCI_MSB_INTERNALS* ehci_msb_internals, USB_EHCI* usb_ehci);
void EHCI_Reset(uint8_t bus, uint8_t device, uint8_t function, uint16_t* const vga_buffer, size_t* const index);

void EHCI_BulkOnly_Setup(USB_EHCI_MSB_INTERNALS* ehci_msb_internals, USB_EHCI* usb_ehci, USB_EHCI_MSB_QUEUE_HEADS* ehci_msb_qhs);
uint8_t EHCI_Read(USB_EHCI* usb_ehci, USB_EHCI_MSB_QUEUE_HEADS* ehci_msb_qhs, void* data, uint32_t lba_start, uint16_t lba_count);

#endif /* EHCI_H */