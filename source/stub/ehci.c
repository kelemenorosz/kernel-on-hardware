#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "time.h"
#include "pci.h"
#include "print.h"
#include "string.h"
#include "ehci.h"

//================================================================================
// Gobal variables

extern const size_t VGA_WIDTH;

const uint32_t QH_BASE_ADDR 	= 0x02000000;
const uint32_t QTD_BASE_ADDR 	= 0x03000000;

//================================================================================
// EHCI PCI REGISTERS

typedef struct __attribute__((__packed__)) EHCI_PCIREG_USBLEGSUP {

	volatile uint8_t CAPABILITY_ID;
	volatile uint8_t EHCI_EXT_CAP_POINTER;
	volatile uint8_t HC_BIOS_OWNED_SEMAPHORE;
	volatile uint8_t HC_OS_OWNED_SEMAPHORE;

} EHCI_PCIREG_USBLEGSUP; 

//================================================================================
// EHCI CAPABILITY REGISTERS

typedef struct __attribute__((__packed__)) EHCI_CAPREG {

	volatile uint8_t CAPLENGTH;
	volatile uint8_t reserved;
	volatile uint16_t HCIVERSION;
	volatile uint32_t HCSPARAMS;
	volatile uint32_t HCCPARAMS;	

} EHCI_CAPREG;

typedef struct __attribute__((__packed__)) EHCI_CAPREG_HCSPARAMS_F {

	uint8_t debug_port_number;
	uint8_t P_INDICATOR;
	uint8_t N_CC;
	uint8_t N_PCC;
	uint8_t port_routing_rules;
	uint8_t PPC;
	uint8_t N_PORTS;

} EHCI_CAPREG_HCSPARAMS_F;

typedef struct __attribute__((__packed__)) EHCI_CAPREG_HCCPARAMS_F {

	uint8_t EECP;
	uint8_t isochronous_scheduling_treshold;
	uint8_t async_schedule_park_capability;
	uint8_t programmable_frame_list_flag;
	uint8_t addressing_capability_64_bit;

} EHCI_CAPREG_HCCPARAMS_F;

//================================================================================
// EHCI OPERATIONAL REGISTERS

typedef struct __attribute__((__packed__)) EHCI_OPREG_CORE {

	volatile uint32_t USBCMD;
	volatile uint32_t USBSTS;
	volatile uint32_t USBINTR;
	volatile uint32_t FRINDEX;
	volatile uint32_t CTRLDSSEGMENT;
	volatile uint32_t PERIODICLISTBASE;
	volatile uint32_t ASYNCLISTADDR;

} EHCI_OPREG_CORE;

#define USBCMD_RS								(0x01 << 0x00)
#define USBCMD_HCRESET							(0x01 << 0x01)
#define USBCMD_FRAME_LIST_SIZE_MASK				(0x03 << 0x02)
#define USBCMD_FRAME_LIST_SIZE_SHIFT			(0x02)
#define USBCMD_PERIODIC_SCH_ENABLE				(0x01 << 0x04)
#define USBCMD_ASYNC_SCH_ENABLE					(0x01 << 0x05)
#define USBCMD_INTR_ASYNC_ADVANCE_DOORBELL		(0x01 << 0x06)
#define USBCMD_LIGHT_HOST_CONTROLLER_RESET		(0x01 << 0x07)
#define USBCMD_ASYNC_SCH_PARK_MODE_COUNT_MASK	(0x03 << 0x08)
#define USBCMD_ASYNC_SCH_PARK_MODE_COUNT_SHIFT 	(0x08)
#define USBCMD_ASYNC_SCH_PARK_MODE_ENABLE		(0x01 << 0x0B)
#define USBCMD_ITC_MASK							(0xFF << 0x10)
#define USBCMD_ITC_SHIFT 						(0x10)

#define USBSTS_USBINT 							(0x01 << 0x00)
#define USBSTS_USBERRINT						(0x01 << 0x01)
#define USBSTS_PORT_CHANGE_DETECT				(0x01 << 0x02)
#define USBSTS_FRAME_LIST_ROLLOVER				(0x01 << 0x03)
#define USBSTS_HOST_SYSTEM_ERROR				(0x01 << 0x04)
#define USBSTS_INTERRUPT_ON_ASYNC_ADVANCE		(0x01 << 0x05)
#define USBSTS_HCHALTED							(0x01 << 0x0B)
#define USBSTS_RECLAMATION						(0x01 << 0x0C)
#define USBSTS_PERIODIC_SCH_STATUS				(0x01 << 0x0E)
#define USBSTS_ASYNC_SCH_STATUS					(0x01 << 0x0F)

typedef struct __attribute__((__packed__)) EHCI_OPREG_AUX {

	volatile uint32_t CONFIGFLAG;
	volatile uint32_t PORTSC;

} EHCI_OPREG_AUX;

#define PORTSC_CURRENT_CONNECT_STATUS 	(0x1 << 0x0) 
#define PORTSC_CONNECT_STATUS_CHANGE 	(0x1 << 0x1) 
#define PORTSC_ENABLED 					(0x1 << 0x2) 
#define PORTSC_ENABLED_CHANGE			(0x1 << 0x3) 
#define PORTSC_OVER_CURRENT_ACTIVE		(0x1 << 0x4) 
#define PORTSC_OVER_CURRENT_CHANGE		(0x1 << 0x5) 
#define PORTSC_FORCE_RESUME				(0x1 << 0x6) 
#define PORTSC_SUSPEND					(0x1 << 0x7) 
#define PORTSC_RESET					(0x1 << 0x8) 
#define PORTSC_LINE_STATUS 				(0x3 << 0xA) 
#define PORTSC_PORT_POWER 				(0x1 << 0xC) 
#define PORTSC_PORT_OWNER 				(0x1 << 0xD) 
#define PORTSC_INDICATOR_CONTROL 		(0x3 << 0xE) 
#define PORTSC_TEST_CONTROL 			(0xF << 0x10) 
#define PORTSC_WKCNNT_E 				(0x1 << 0x14) 
#define PORTSC_WKDSCNNT_E 				(0x1 << 0x15) 
#define PORTSC_WKOC_E 					(0x1 << 0x16) 

//================================================================================
// EHCI DATA STRUCTURES

// 64-bytes long (Per the specification has to be 32-byte aligned)
typedef struct __attribute__((__packed__)) EHCI_QUEUE_HEAD {

	volatile uint32_t link;
	volatile uint32_t endp_ch;
	volatile uint32_t endp_cap;
	volatile uint32_t qtd_curr;
	volatile uint32_t qtd_next;
	volatile uint32_t qtd_alt;
	volatile uint32_t token;
	volatile uint32_t buf[5];
	volatile uint32_t ext_buf[5];

	volatile uint8_t padding[28];

} EHCI_QUEUE_HEAD;

#define QH_ENDP_CH_NAK_COUNT_RELOAD_SHIFT		(0x1C)
#define QH_ENDP_CH_MAX_PACKET_LENGTH_SHIFT		(0x10)
#define QH_ENDP_CH_HEAD_OF_RECLAMATION_SHIFT	(0x0F)
#define QH_ENDP_CH_DATA_TOGGLE_CONTROL_SHIFT	(0x0E)
#define QH_ENDP_CH_ENDPOINT_SPEED_SHIFT			(0x0C)
#define QH_ENDP_CH_ENDPOINT_NUMBER_SHIFT		(0x08)
#define QH_ENDP_CH_DEVICE_ADDRESS_SHIFT			(0x00)

#define QH_ENDP_CAP_PIPE_MULT_SHIFT				(0x1E)

#define QH_PTR_QUEUE_HEAD						(0x02)

typedef struct __attribute__((__packed__)) EHCI_QUEUE_TD {

	volatile uint32_t next;
	volatile uint32_t alt;
	volatile uint32_t token;
	volatile uint32_t buf[5];
	volatile uint32_t ext_buf[5];

	volatile uint8_t padding[12];

} EHCI_QUEUE_TD;

#define QTD_TOKEN_TOGGLE_SHIFT					(0x1F)
#define QTD_BYTES_TO_TRANSFER_SHIFT				(0x10)
#define QTD_CERR_SHIFT							(0x0A)
#define QTD_PID_SHIFT							(0x08)
#define QTD_STATUS_SHIFT						(0x07)

#define QTD_PID_OUT								(0x00)
#define QTD_PID_IN								(0x01)
#define QTD_PID_SETUP							(0x02)

#define QTD_PTR_TERMINATE						(0x01)

//================================================================================
// USB CONTROL TRANSFER DATA STRUCTURES

typedef struct __attribute__((__packed__)) USB_DEVICE_DESCRIPTOR {

	volatile uint8_t bLength;
	volatile uint8_t bDescriptorType;
	volatile uint16_t bcdUSB;
	volatile uint8_t bDeviceClass;
	volatile uint8_t bDeviceSubClass;
	volatile uint8_t bDeviceProtocol;
	volatile uint8_t bMaxPacketSize0;
	volatile uint16_t idVendor;
	volatile uint16_t idProduct;
	volatile uint16_t bcdDevice;
	volatile uint8_t iManufacturer;
	volatile uint8_t iProduct;
	volatile uint8_t iSerialNumber;
	volatile uint8_t bNumConfigurations;

	volatile uint8_t padding[2];

} USB_DEVICE_DESCRIPTOR;

typedef struct __attribute__((__packed__)) USB_CONFIGURATION_DESCRIPTOR {

	volatile uint8_t bLength;
	volatile uint8_t bDescriptorType;
	volatile uint16_t wTotalLength;
	volatile uint8_t bNumInterfaces;
	volatile uint8_t bConfigurationValue;
	volatile uint8_t iConfiguration;
	volatile uint8_t bmAttributes;
	volatile uint8_t bMaxPower;

} USB_CONFIGURATION_DESCRIPTOR;

typedef struct __attribute__((__packed__)) USB_INTERFACE_DESCRIPTOR {

	volatile uint8_t bLength;
	volatile uint8_t bDescriptorType;
	volatile uint8_t bInterfaceNumber;
	volatile uint8_t bAlternateSetting;
	volatile uint8_t bNumEndpoints;
	volatile uint8_t bInterfaceClass;
	volatile uint8_t bInterfaceSubClass;
	volatile uint8_t bInterfaceProtocol;
	volatile uint8_t iInterface;

} USB_INTERFACE_DESCRIPTOR;

typedef struct __attribute__((__packed__)) USB_ENDPOINT_DESCRIPTOR {

	volatile uint8_t bLength;
	volatile uint8_t bDescriptorType;
	volatile uint8_t bEndpointAddress;
	volatile uint8_t bmAttributes;
	volatile uint16_t wMaxPacketSize;
	volatile uint8_t bInterval;

} USB_ENDPOINT_DESCRIPTOR;

typedef struct __attribute__((__packed__)) USB_INTERFACE_ASSOCIATION_DESCRIPTOR {

	volatile uint8_t bLength;
	volatile uint8_t bDescriptorType;
	volatile uint8_t bFirstInterface;
	volatile uint8_t bInterfaceCount;
	volatile uint8_t bFunctionClass;
	volatile uint8_t bFunctionSubClass;
	volatile uint8_t bFunctionProtocol;
	volatile uint8_t iFunction;

} USB_INTERFACE_ASSOCIATION_DESCRIPTOR;

typedef struct __attribute__((__packed__)) USB_CDC_CS_FUNCTIONAL_DESCRIPTOR_HEADER {

	volatile uint8_t bFunctionLength;
	volatile uint8_t bDescriptorType;
	volatile uint8_t bDescriptorSubtype;
	volatile uint16_t bcdCDC;

} USB_CDC_CS_FUNCTIONAL_DESCRIPTOR_HEADER;

typedef struct __attribute__((__packed__)) USB_UVC_CS_VC_INTERFACE_DESCRIPTOR_HEADER {

	volatile uint8_t bLength;
	volatile uint8_t bDescriptorType;
	volatile uint8_t bDescriptorSubtype;
	volatile uint16_t bcdUVC;
	volatile uint16_t wTotalLength;
	volatile uint32_t dwClockFrequency;
	volatile uint8_t bInCollection;

} USB_UVC_CS_VC_INTERFACE_DESCRIPTOR_HEADER;

typedef struct __attribute__((__packed__)) USB_UVC_CS_VC_INTERRUPUT_ENDPOINT_DESCRIPTOR {

	volatile uint8_t bLength;
	volatile uint8_t bDescriptorType;
	volatile uint8_t bDescriptorSubtype;
	volatile uint16_t wMaxTransferSize;

} USB_UVC_CS_VC_INTERRUPUT_ENDPOINT_DESCRIPTOR;

typedef struct __attribute__((__packed__)) USB_DEVICE_REQUEST {

	volatile uint8_t bmRequestType;
	volatile uint8_t bRequest;
	volatile uint16_t wValue;
	volatile uint16_t wIndex;
	volatile uint16_t wLength;

} USB_DEVICE_REQUEST;

#define USB_DR_REQUEST_GET_DESCRIPTOR		(0x06)
#define USB_DR_REQUEST_SET_ADDRESS			(0x05)
#define USB_DR_REQUEST_SET_CONFIGURATION	(0x09)

#define USB_DR_REQUEST_MSD_GET_MAX_LUN		(0xFE)
#define USB_DR_REQUEST_MSD_RESET			(0xFF)

#define USB_DR_DESC_TYPE_DEVICE				(0x01)
#define USB_DR_DESC_TYPE_CONFIGURATION		(0x02)			

#define USB_CTRL_DEV_TO_HOST				(0x00)
#define USB_CTRL_HOST_TO_DEV				(0x01)

//================================================================================
// USB MASS STORAGE STRUCTURES

typedef struct __attribute__((__packed__)) BULK_ONLY_COMMAND_BLOCK {

	volatile uint8_t bits[16];

} BULK_ONLY_COMMAND_BLOCK;

typedef struct __attribute__((__packed__)) BULK_ONLY_CBW {

	volatile uint32_t dCBWSignature;
	volatile uint32_t dCBWTag;
	volatile uint32_t dCBWDataTransferLength;
	volatile uint8_t bmCBWFlags;
	volatile uint8_t bCBWLUN;
	volatile uint8_t bCBWCBLength;
	volatile BULK_ONLY_COMMAND_BLOCK CBWCB;

} BULK_ONLY_CBW;

typedef struct __attribute__((__packed__)) BULK_ONLY_CSW {

	volatile uint32_t dCSWSignature;
	volatile uint32_t dCSWTag;
	volatile uint32_t dCSWDataResidue;
	volatile uint8_t bCSWStatus;

} BULK_ONLY_CSW;

typedef struct __attribute__((__packed__)) SCSI_READ_CAPACITY {

	volatile uint8_t returnedLogicalBlockAddress[4];
	volatile uint8_t blockLengthInBytes[4];

} SCSI_READ_CAPACITY;

//================================================================================
// INTERNAL DATA STRUCTURES

typedef struct EHCI_INTERNALS {

	uint32_t qh_count;
	uint32_t qtd_count;
	uint32_t first_qh;

} EHCI_INTERNALS;

//================================================================================
// Non-exported function declarations

uint8_t usb_bulk_transaction(EHCI_INTERNALS* ehci_internals, USB_EHCI_MSB_QUEUE_HEADS* ehci_msb_qhs, BULK_ONLY_CBW* cbw, void* data, uint32_t max_packet_size, uint32_t transaction_direction);
EHCI_QUEUE_TD* usb_control_transaction(EHCI_INTERNALS* ehci_internals, USB_DEVICE_REQUEST* req, void* data, uint32_t max_packet_size, uint32_t transaction_direction);
void init_qh(EHCI_QUEUE_HEAD* qh, uint32_t max_packet_size, uint32_t endp, uint32_t addr); 
void init_qtd(EHCI_QUEUE_TD* qtd, EHCI_QUEUE_TD* prev, uint32_t toggle, uint32_t len, uint32_t pid, void* data);
EHCI_QUEUE_HEAD* alloc_qh(EHCI_INTERNALS* ehci_internals);
EHCI_QUEUE_TD* alloc_qtd(EHCI_INTERNALS* ehci_internals);
void insert_qh(EHCI_INTERNALS* ehci_internals, EHCI_QUEUE_HEAD* qh);
void wait_for_qh(EHCI_QUEUE_HEAD* qh);

void reset_ports(EHCI_OPREG_AUX* opreg_aux, uint8_t n_ports);

void set_EHCI_portsc(EHCI_OPREG_AUX* opreg_aux, uint32_t port, uint32_t value);
void init_EHCI_QUEUE_TD(EHCI_QUEUE_TD* qTD);

void print_SCSI_READ_CAPACITY(uint16_t* const vga_buffer, size_t* const index, const SCSI_READ_CAPACITY* const scsi_readCap);
void print_BULK_ONLY_CSW(uint16_t* const vga_buffer, size_t* const index, const BULK_ONLY_CSW* const csw);
void print_USB_ENDPOINT_DESCRIPTOR(uint16_t* const vga_buffer, size_t* const index, const USB_ENDPOINT_DESCRIPTOR* const endpDesc);
void print_USB_INTERFACE_DESCRIPTOR(uint16_t* const vga_buffer, size_t* const index, const USB_INTERFACE_DESCRIPTOR* const interfaceDesc);
void print_USB_CONFIGURATION_DESCRIPTOR(uint16_t* const vga_buffer, size_t* const index, const USB_CONFIGURATION_DESCRIPTOR* const confDesc);
void print_USB_DEVICE_DESCRIPTOR(uint16_t* const vga_buffer, size_t* const index, const USB_DEVICE_DESCRIPTOR* const devDesc);
void print_EHCI_QUEUE_TD(uint16_t* const vga_buffer, size_t* const index, const EHCI_QUEUE_TD* const qtd);
void print_EHCI_QUEUE_HEAD(uint16_t* const vga_buffer, size_t* const index, const EHCI_QUEUE_HEAD* const qh);
void print_EHCI_CAPREG_HCCPARAMS_F(uint16_t* const vga_buffer, size_t* const index, const EHCI_CAPREG_HCCPARAMS_F* const hccparams);
void print_EHCI_CAPREG_HCSPARAMS_F(uint16_t* const vga_buffer, size_t* const index, const EHCI_CAPREG_HCSPARAMS_F* const hcsparams);
void print_EHCI_PCIREG_USBLEGSUP(uint16_t* const vga_buffer, size_t* const index, const EHCI_PCIREG_USBLEGSUP* const usblegsup);

//================================================================================
// Exported functions

void EHCI_Reset(uint8_t bus, uint8_t device, uint8_t function, uint16_t* const vga_buffer, size_t* const index) {

	size_t index_c = (*index) / VGA_WIDTH;

	// Get USBBASE value from PCI port

	uint32_t usbbase = get_PCI_BAR0(bus, device, function); 
	uint32_t base_address = usbbase & 0xFFFFFF00;
	
	// Get CAPABILITY REGISTERS

	EHCI_CAPREG* capreg = NULL;
	capreg = (EHCI_CAPREG*) base_address;
	
	// Get OPERATIONAL REGISTERS

	EHCI_OPREG_CORE* opreg_core = NULL;
	EHCI_OPREG_AUX* opreg_aux = NULL;
	opreg_core = (EHCI_OPREG_CORE*)(((uint8_t*)base_address) + capreg->CAPLENGTH);
	opreg_aux = (EHCI_OPREG_AUX*)((uint8_t*)opreg_core + 0x40);

	// Format HCCPARAMS and HCSPARAMS CAPABILITY REGISTERS

	EHCI_CAPREG_HCCPARAMS_F hccparams = {};
	hccparams.EECP = (capreg->HCCPARAMS & 0x0000FF00) >> 0x8;
	hccparams.isochronous_scheduling_treshold = (capreg->HCCPARAMS & 0x000000F0) >> 0x4; 
	hccparams.async_schedule_park_capability = (capreg->HCCPARAMS & 0x00000004) >> 0x2;
	hccparams.programmable_frame_list_flag = (capreg->HCCPARAMS & 0x00000002) >> 0x1;
	hccparams.addressing_capability_64_bit = capreg->HCCPARAMS & 0x00000001;

	EHCI_CAPREG_HCSPARAMS_F hcsparams = {};
	hcsparams.debug_port_number = (capreg->HCSPARAMS & 0x00F00000) >> 0x14;
	hcsparams.P_INDICATOR = (capreg->HCSPARAMS & 0x00010000) >> 0x10;
	hcsparams.N_CC = (capreg->HCSPARAMS & 0x0000F000) >> 0xC;
	hcsparams.N_PCC = (capreg->HCSPARAMS & 0x00000F00) >> 0x8;
	hcsparams.port_routing_rules = (capreg->HCSPARAMS & 0x00000080) >> 0x7;
	hcsparams.PPC = (capreg->HCSPARAMS & 0x00000008) >> 0x3;
	hcsparams.N_PORTS = capreg->HCSPARAMS & 0x00000007;

	// Get USBLEGSUP value from PCI port

	EHCI_PCIREG_USBLEGSUP usblegsup = {};
	uint32_t* usblegsup_uint32_ptr = (uint32_t*)&usblegsup;
	*usblegsup_uint32_ptr = get_PCI_offset(bus, device, function, hccparams.EECP);

	// print_string(vga_buffer, index, "PCI ehci USBLEGSUP HC_BIOS_OWNED_SEMAPHORE: ", strlen("PCI ehci USBLEGSUP HC_BIOS_OWNED_SEMAPHORE: "));
	// print_byte(vga_buffer, index, usblegsup.HC_BIOS_OWNED_SEMAPHORE);

	// index_c++;
	// *index = index_c * VGA_WIDTH;

	// print_string(vga_buffer, index, "PCI ehci USBLEGSUP HC_OS_OWNED_SEMAPHORE: ", strlen("PCI ehci USBLEGSUP HC_OS_OWNED_SEMAPHORE: "));
	// print_byte(vga_buffer, index, usblegsup.HC_OS_OWNED_SEMAPHORE);

	// index_c++;
	// *index = index_c * VGA_WIDTH;
	
	// Set USBLEGSUP HC_OS_OWNED_SEMAPHORE to TRUE (get control of host controller from the BIOS) 

	usblegsup.HC_OS_OWNED_SEMAPHORE = 0x1;
	set_PCI_offset(bus, device, function, hccparams.EECP, *usblegsup_uint32_ptr);

	sleep(0x20);

	// Get USBLEGSUP value from PCI port for confirmation of setting semaphore

	*usblegsup_uint32_ptr = get_PCI_offset(bus, device, function, hccparams.EECP);

	// print_string(vga_buffer, index, "Modified PCI ehci USBLEGSUP HC_BIOS_OWNED_SEMAPHORE: ", strlen("Modified PCI ehci USBLEGSUP HC_BIOS_OWNED_SEMAPHORE: "));
	// print_byte(vga_buffer, index, usblegsup.HC_BIOS_OWNED_SEMAPHORE);

	// index_c++;
	// *index = index_c * VGA_WIDTH;

	// print_string(vga_buffer, index, "Modified PCI ehci USBLEGSUP HC_OS_OWNED_SEMAPHORE: ", strlen("Modified PCI ehci USBLEGSUP HC_OS_OWNED_SEMAPHORE: "));
	// print_byte(vga_buffer, index, usblegsup.HC_OS_OWNED_SEMAPHORE);

	// index_c++;
	// *index = index_c * VGA_WIDTH;

	return;

}

void EHCI_Init(uint8_t bus, uint8_t device, uint8_t function, USB_EHCI_MSB_INTERNALS* ehci_msb_internals, USB_EHCI* usb_ehci) {

	uint16_t* vga_buffer = (uint16_t*)0xB8000;
	size_t index_c = 0x2;
	size_t index = index_c * VGA_WIDTH;

	ehci_msb_internals->ehci_addr = 0x0;
	ehci_msb_internals->ehci_bulk_endp_addr_out = 0x0;
	ehci_msb_internals->ehci_bulk_endp_addr_in = 0x0;
	ehci_msb_internals->ehci_bulk_endp_max_s_out = 0x0;
	ehci_msb_internals->ehci_bulk_endp_max_s_in = 0x0;
	ehci_msb_internals->ehci_bulk_interface_nr = 0x0;

	// Get USBBASE value from PCI port

	uint32_t usbbase = get_PCI_BAR0(bus, device, function); 
	uint32_t base_address = usbbase & 0xFFFFFF00;
	
	// Get CAPABILITY REGISTERS

	EHCI_CAPREG* capreg = NULL;
	capreg = (EHCI_CAPREG*) base_address;
	
	// Get OPERATIONAL REGISTERS

	EHCI_OPREG_CORE* opreg_core = NULL;
	EHCI_OPREG_AUX* opreg_aux = NULL;
	opreg_core = (EHCI_OPREG_CORE*)(((uint8_t*)base_address) + capreg->CAPLENGTH);
	opreg_aux = (EHCI_OPREG_AUX*)((uint8_t*)opreg_core + 0x40);

	// Format HCCPARAMS and HCSPARAMS CAPABILITY REGISTERS

	EHCI_CAPREG_HCCPARAMS_F hccparams = {};
	hccparams.EECP = (capreg->HCCPARAMS & 0x0000FF00) >> 0x8;
	hccparams.isochronous_scheduling_treshold = (capreg->HCCPARAMS & 0x000000F0) >> 0x4; 
	hccparams.async_schedule_park_capability = (capreg->HCCPARAMS & 0x00000004) >> 0x2;
	hccparams.programmable_frame_list_flag = (capreg->HCCPARAMS & 0x00000002) >> 0x1;
	hccparams.addressing_capability_64_bit = capreg->HCCPARAMS & 0x00000001;

	EHCI_CAPREG_HCSPARAMS_F hcsparams = {};
	hcsparams.debug_port_number = (capreg->HCSPARAMS & 0x00F00000) >> 0x14;
	hcsparams.P_INDICATOR = (capreg->HCSPARAMS & 0x00010000) >> 0x10;
	hcsparams.N_CC = (capreg->HCSPARAMS & 0x0000F000) >> 0xC;
	hcsparams.N_PCC = (capreg->HCSPARAMS & 0x00000F00) >> 0x8;
	hcsparams.port_routing_rules = (capreg->HCSPARAMS & 0x00000080) >> 0x7;
	hcsparams.PPC = (capreg->HCSPARAMS & 0x00000008) >> 0x3;
	hcsparams.N_PORTS = capreg->HCSPARAMS & 0x00000007;

	// Initialization

	EHCI_INTERNALS ehci_internals = {};
	ehci_internals.qh_count = 0x0;
	ehci_internals.qtd_count = 0x0;
	ehci_internals.first_qh = 0x0;

	// Set CTRLDSSEGMENT - Not used in 32-bit addressing mode 
	if (hccparams.addressing_capability_64_bit != 0x0) opreg_core->CTRLDSSEGMENT = 0x0;
	
	// Set USBINTR - Disable interrupts
	opreg_core->USBINTR = 0x0;

	// Set FRINDEX
	opreg_core->FRINDEX = 0x0;

	// Set PERIODICLISTBASE - Periodic Frame List at 0x04000000 in memory; Periodic queue head at 0x03000000 in memory
	EHCI_QUEUE_HEAD* periodic_queue_head = (EHCI_QUEUE_HEAD*)(0x04000000);
	periodic_queue_head->link = 0x1; // TERMINATE
	periodic_queue_head->endp_ch = 0x0;
	periodic_queue_head->endp_cap = 0x0;
	periodic_queue_head->qtd_curr = 0x0;
	periodic_queue_head->qtd_next = 0x1; // TERMINATE
	periodic_queue_head->qtd_alt = 0x0;
	periodic_queue_head->token = 0x0;
	for (uint8_t i = 0; i < 5; ++i) {periodic_queue_head->buf[i] = 0x0; periodic_queue_head->ext_buf[i] = 0x0;}
	opreg_core->PERIODICLISTBASE = (uint32_t)(0x01000000);
	uint32_t* periodic_frame_list = (uint32_t*)opreg_core->PERIODICLISTBASE;
	for (size_t i = 0; i < 1024; ++i) {
		*periodic_frame_list = (uint32_t)(periodic_queue_head) | 0x11; // QUEUE HEAD and TERMINATE
		periodic_frame_list++;
	}

	// Set ASYNCLISTADDR - First queue head at 0x02000000 in memory
	EHCI_QUEUE_HEAD* async_queue_head = (EHCI_QUEUE_HEAD*)(0x02000000);
	async_queue_head->link = (uint32_t)(async_queue_head) | QH_PTR_QUEUE_HEAD; // QUEUE HEAD
	async_queue_head->endp_ch = 0x1 << 0xF;
	async_queue_head->endp_cap = 0x0;
	async_queue_head->qtd_curr = 0x0;
	async_queue_head->qtd_next = 0x1; // TERMINATE
	async_queue_head->qtd_alt = 0x0;
	async_queue_head->token = 0x0;
	for (uint8_t i = 0; i < 5; ++i) {async_queue_head->buf[i] = 0x0; async_queue_head->ext_buf[i] = 0x0;}
	opreg_core->ASYNCLISTADDR = (uint32_t)async_queue_head;
	ehci_internals.qh_count = 0x1;
	ehci_internals.first_qh = (uint32_t)async_queue_head;

	// Set USBSTS - Clear interrupt bits (write 0x1 to first 6 bits)
	opreg_core->USBSTS = 0x3F;

	// Set USBCMD - Enable async shedule, set microframe duration and start host controller
	opreg_core->USBCMD = (0x8 << USBCMD_ITC_SHIFT) | USBCMD_ASYNC_SCH_ENABLE | USBCMD_RS; 

	// Wait for start
	while (opreg_core->USBSTS & USBSTS_HCHALTED);

	// Set CONFIGFLAG
	opreg_aux->CONFIGFLAG = 0x1;

	sleep(20);	

	// Reset ports 

	uint8_t available_address = 0x1;
	volatile uint32_t* portsc = NULL;
	portsc = &opreg_aux->PORTSC;
	for (uint8_t i = 0; i < hcsparams.N_PORTS; ++i) {

		set_EHCI_portsc(opreg_aux, i, (*portsc | PORTSC_RESET) & ~PORTSC_ENABLED);
		sleep(10);
		set_EHCI_portsc(opreg_aux, i, (*portsc) & ~PORTSC_RESET);
		sleep(10);

		// print_dword(vga_buffer, &index, *portsc);
		// index_c++;
		// index = index_c * VGA_WIDTH;

		if ((*portsc) & PORTSC_ENABLED) {

			uint8_t transaction_incomplete = 0x0;

			// Set async QH
			uint32_t endp_ch = 		(0x05 			<< QH_ENDP_CH_NAK_COUNT_RELOAD_SHIFT) |
					   				(0x40			<< QH_ENDP_CH_MAX_PACKET_LENGTH_SHIFT) |
					   				(0x01			<< QH_ENDP_CH_HEAD_OF_RECLAMATION_SHIFT) |
								   	(0x01 			<< QH_ENDP_CH_DATA_TOGGLE_CONTROL_SHIFT) |
								   	(0x02			<< QH_ENDP_CH_ENDPOINT_SPEED_SHIFT) |
								   	(0x00 			<< QH_ENDP_CH_ENDPOINT_NUMBER_SHIFT) |
								   	(0x00 			<< QH_ENDP_CH_DEVICE_ADDRESS_SHIFT);

			uint32_t endp_cap = 0x01 << QH_ENDP_CAP_PIPE_MULT_SHIFT;

			async_queue_head->endp_ch = endp_ch;
			async_queue_head->endp_cap = endp_cap;
			async_queue_head->qtd_alt = QTD_PTR_TERMINATE;
			async_queue_head->qtd_curr = 0x0;
			async_queue_head->qtd_next = QTD_PTR_TERMINATE;
			async_queue_head->token = 0x0;
			for (uint8_t i = 0; i < 5; ++i) {async_queue_head->buf[i] = 0x0; async_queue_head->ext_buf[i] = 0x0;}

			// GET_DESCRIPTOR - Device descriptor
			USB_DEVICE_REQUEST devReq = {};
			devReq.bmRequestType = 0x80;
			devReq.bRequest = USB_DR_REQUEST_GET_DESCRIPTOR;
			devReq.wValue = USB_DR_DESC_TYPE_DEVICE << 0x8;
			devReq.wIndex = 0x0;
			devReq.wLength = 0x40;

			USB_DEVICE_DESCRIPTOR devDesc = {};

			EHCI_QUEUE_TD* first_qtd = usb_control_transaction(&ehci_internals, &devReq, (void*)&devDesc, 0x40, USB_CTRL_DEV_TO_HOST); 
			async_queue_head->qtd_next = (uint32_t)first_qtd;
			wait_for_qh(async_queue_head);
			//sleep(10); // Should use a while() and check for qh->token for active bit
			// transaction_incomplete = 0x1;
			// while (transaction_incomplete) {
			// 	if (async_queue_head->qtd_next == QTD_PTR_TERMINATE) {
			// 		if ((async_queue_head->token & (0x1 << QTD_STATUS_SHIFT)) == 0x0) transaction_incomplete = 0x0;
			// 	}
			// }

			// SET_ADDRESS - Set device address

			uint8_t dev_addr = available_address;
			available_address++;

			USB_DEVICE_REQUEST devReq_setAddr = {};
			devReq_setAddr.bmRequestType = 0x0;
			devReq_setAddr.bRequest = USB_DR_REQUEST_SET_ADDRESS;
			devReq_setAddr.wValue = dev_addr; 
			devReq_setAddr.wIndex = 0x0;
			devReq_setAddr.wLength = 0x0;

			first_qtd = usb_control_transaction(&ehci_internals, &devReq_setAddr, NULL, 0x40, USB_CTRL_HOST_TO_DEV);
			async_queue_head->qtd_next = (uint32_t)first_qtd;
			wait_for_qh(async_queue_head);
			//sleep(10);

			// Change queue head endpoint characteristics

			endp_ch = 	(0x05 			<< QH_ENDP_CH_NAK_COUNT_RELOAD_SHIFT) |
					   	(0x40			<< QH_ENDP_CH_MAX_PACKET_LENGTH_SHIFT) |
					   	(0x01			<< QH_ENDP_CH_HEAD_OF_RECLAMATION_SHIFT) |
						(0x01 			<< QH_ENDP_CH_DATA_TOGGLE_CONTROL_SHIFT) |
						(0x02			<< QH_ENDP_CH_ENDPOINT_SPEED_SHIFT) |
						(0x00 			<< QH_ENDP_CH_ENDPOINT_NUMBER_SHIFT) |
						(dev_addr		<< QH_ENDP_CH_DEVICE_ADDRESS_SHIFT);
			async_queue_head->endp_ch = endp_ch;

			// GET_DESCRIPTOR - Get first 4 bytes of configuration descriptor

			USB_DEVICE_REQUEST devReq_conf_desc = {};
			devReq_conf_desc.bmRequestType = 0x80;
			devReq_conf_desc.bRequest = USB_DR_REQUEST_GET_DESCRIPTOR;
			devReq_conf_desc.wValue = USB_DR_DESC_TYPE_CONFIGURATION << 0x8;
			devReq_conf_desc.wIndex = 0x0;
			devReq_conf_desc.wLength = 0x4; // Only the first 4 bytes of configuration descriptor

			USB_CONFIGURATION_DESCRIPTOR devDesc_conf = {};

			first_qtd = usb_control_transaction(&ehci_internals, &devReq_conf_desc, (void*)&devDesc_conf, 0x40, USB_CTRL_DEV_TO_HOST);
			async_queue_head->qtd_next = (uint32_t)first_qtd;
			wait_for_qh(async_queue_head);
			//sleep(10);

			// GET_DESCRIPTOR - Get configuration, interface and endpoint descriptors

			// print_string(vga_buffer, &index, "USB CONF DESC wTotalLength: ", strlen("USB CONF DESC wTotalLength: "));
			// print_word(vga_buffer, &index, devDesc_conf.wTotalLength);
			// index_c++;
			// index = index_c * VGA_WIDTH;

			// Allocate buffer of wTotalLength bytes for configuration, interface and endpoint descriptors
			volatile uint8_t confBuffer[devDesc_conf.wTotalLength];

			devReq_conf_desc.wLength = devDesc_conf.wTotalLength;
			first_qtd = usb_control_transaction(&ehci_internals, &devReq_conf_desc, (void*)&confBuffer, 0x40, USB_CTRL_DEV_TO_HOST); 
			async_queue_head->qtd_next = (uint32_t)first_qtd;
			wait_for_qh(async_queue_head);
			// sleep(10);

			uint8_t* confBuffer_end = ((uint8_t*)&confBuffer) + devDesc_conf.wTotalLength;
			uint8_t* confBuffer_ptr = (uint8_t*)&confBuffer;
			confBuffer_ptr += sizeof(USB_CONFIGURATION_DESCRIPTOR);
			USB_INTERFACE_DESCRIPTOR* last_interfaceDesc = NULL;

			while (confBuffer_ptr < confBuffer_end) {

				// print_string(vga_buffer, &index, "USB bDescriptorType: ", strlen("USB bDescriptorType: "));
				// print_byte(vga_buffer, &index, *(confBuffer_ptr + 1));
				// index_c++;
				// index = index_c * VGA_WIDTH;

				if (*(confBuffer_ptr + 1) == 0x4) { // Interface descriptor
					// print_USB_INTERFACE_DESCRIPTOR(vga_buffer, &index, (USB_INTERFACE_DESCRIPTOR*)confBuffer_ptr);
					// index_c = index / VGA_WIDTH;
					last_interfaceDesc = (USB_INTERFACE_DESCRIPTOR*)confBuffer_ptr;
					if (last_interfaceDesc->bInterfaceClass == 0x08 && last_interfaceDesc->bInterfaceSubClass == 0x06 && last_interfaceDesc->bInterfaceProtocol == 0x50) {
						ehci_msb_internals->ehci_bulk_interface_nr = last_interfaceDesc->bInterfaceNumber;
					}
					confBuffer_ptr += sizeof(USB_INTERFACE_DESCRIPTOR);
				}
				else if (*(confBuffer_ptr + 1) == 0x5) { // Endpoint descriptor
					if (last_interfaceDesc->bInterfaceClass == 0x08 && last_interfaceDesc->bInterfaceSubClass == 0x06 && last_interfaceDesc->bInterfaceProtocol == 0x50) {
						USB_ENDPOINT_DESCRIPTOR* endpDesc = (USB_ENDPOINT_DESCRIPTOR*)confBuffer_ptr;
						if (endpDesc->bEndpointAddress & 0x80) {
							ehci_msb_internals->ehci_bulk_endp_addr_in = endpDesc->bEndpointAddress & 0x0F;
							ehci_msb_internals->ehci_bulk_endp_max_s_in = endpDesc->wMaxPacketSize;
						}
						else {
							ehci_msb_internals->ehci_bulk_endp_addr_out = endpDesc->bEndpointAddress & 0x0F;
							ehci_msb_internals->ehci_bulk_endp_max_s_out = endpDesc->wMaxPacketSize;
						}
					}
					confBuffer_ptr += sizeof(USB_ENDPOINT_DESCRIPTOR);
				}
				else if (*(confBuffer_ptr + 1) == 0x0B) {
					confBuffer_ptr += sizeof(USB_INTERFACE_ASSOCIATION_DESCRIPTOR);
				}
				else if (*(confBuffer_ptr + 1) == 0x24) {
					
					// Video Interface Class Code
					if (last_interfaceDesc->bInterfaceClass == 0x0E) {
						USB_UVC_CS_VC_INTERFACE_DESCRIPTOR_HEADER* uvc_interfaceDescHeader = (USB_UVC_CS_VC_INTERFACE_DESCRIPTOR_HEADER*)confBuffer_ptr; 
						confBuffer_ptr += uvc_interfaceDescHeader->wTotalLength;
					}

				}
				else if (*(confBuffer_ptr + 1) == 0x25) {

					// Video Interface Class Code
					if (last_interfaceDesc->bInterfaceClass == 0x0E) {
						confBuffer_ptr += sizeof(USB_UVC_CS_VC_INTERRUPUT_ENDPOINT_DESCRIPTOR);	
					}

				}
			}

			// SET_CONFIGURATION

			if (last_interfaceDesc->bInterfaceClass == 0x08 && last_interfaceDesc->bInterfaceSubClass == 0x06 && last_interfaceDesc->bInterfaceProtocol == 0x50) {

				ehci_msb_internals->ehci_addr = dev_addr;

				USB_CONFIGURATION_DESCRIPTOR* confDesc = (USB_CONFIGURATION_DESCRIPTOR*)&confBuffer;
				USB_DEVICE_REQUEST devReq_setConf = {};
				devReq_setConf.bmRequestType = 0x0;
				devReq_setConf.bRequest = USB_DR_REQUEST_SET_CONFIGURATION;
				devReq_setConf.wValue = confDesc->bConfigurationValue;
				devReq_setConf.wIndex = 0x0;
				devReq_setConf.wLength = 0x0;

				first_qtd = usb_control_transaction(&ehci_internals, &devReq_setConf, NULL, 0x40, USB_CTRL_HOST_TO_DEV);
				async_queue_head->qtd_next = (uint32_t)first_qtd;
				wait_for_qh(async_queue_head);
				// sleep(10);

			}

			// print_string(vga_buffer, &index, "USBCMD post-port_reset: ", strlen("USBCMD post-port_reset: "));
			// print_dword(vga_buffer, &index, opreg_core->USBCMD);
			// index_c++;
			// index = index_c * VGA_WIDTH;

			// print_string(vga_buffer, &index, "USBSTS post-port_reset: ", strlen("USBSTS post-port_reset: "));
			// print_dword(vga_buffer, &index, opreg_core->USBSTS);
			// index_c++;
			// index = index_c * VGA_WIDTH;

		}

		portsc++;

	}

	usb_ehci->ehci_capreg = (uint32_t)capreg;
	usb_ehci->ehci_opreg_core = (uint32_t)opreg_core;
	usb_ehci->ehci_opreg_aux = (uint32_t)opreg_aux;

	return;

}

void EHCI_BulkOnly_Setup(USB_EHCI_MSB_INTERNALS* ehci_msb_internals, USB_EHCI* usb_ehci, USB_EHCI_MSB_QUEUE_HEADS* ehci_msb_qhs) {

	EHCI_CAPREG* capreg = (EHCI_CAPREG*)usb_ehci->ehci_capreg;
	EHCI_OPREG_CORE* opreg_core = (EHCI_OPREG_CORE*)usb_ehci->ehci_opreg_core;
	EHCI_OPREG_AUX* opreg_aux = (EHCI_OPREG_AUX*)usb_ehci->ehci_opreg_aux;

	ehci_msb_qhs->ehci_bulk_out_qh = 0x0;
	ehci_msb_qhs->ehci_bulk_in_qh = 0x0;
	ehci_msb_qhs->ehci_bulk_out_token = 0x0;
	ehci_msb_qhs->ehci_bulk_in_token = 0x0;

	uint16_t* vga_buffer = (uint16_t*)0xB8000;
	size_t index_c = 0x2;
	size_t index = index_c * VGA_WIDTH;

	EHCI_QUEUE_HEAD* qh_control = (EHCI_QUEUE_HEAD*)(0x02000000);

	// Set first qh to msb control endpoint
	uint32_t endp_ch = 	(0x05 							<< QH_ENDP_CH_NAK_COUNT_RELOAD_SHIFT) |
		   				(0x40							<< QH_ENDP_CH_MAX_PACKET_LENGTH_SHIFT) |
		   				(0x01							<< QH_ENDP_CH_HEAD_OF_RECLAMATION_SHIFT) |
					   	(0x01 							<< QH_ENDP_CH_DATA_TOGGLE_CONTROL_SHIFT) |
					   	(0x02							<< QH_ENDP_CH_ENDPOINT_SPEED_SHIFT) |
					   	(0x00 							<< QH_ENDP_CH_ENDPOINT_NUMBER_SHIFT) |
					   	(ehci_msb_internals->ehci_addr	<< QH_ENDP_CH_DEVICE_ADDRESS_SHIFT);

	uint32_t endp_cap = 0x01 << QH_ENDP_CAP_PIPE_MULT_SHIFT;

	qh_control->endp_ch = endp_ch;
	qh_control->endp_cap = endp_cap;
	qh_control->qtd_alt = QTD_PTR_TERMINATE;
	qh_control->qtd_curr = 0x0;
	qh_control->qtd_next = QTD_PTR_TERMINATE;
	qh_control->token = 0x0;
	for (uint8_t i = 0; i < 5; ++i) {qh_control->buf[i] = 0x0; qh_control->ext_buf[i] = 0x0;}

	// Set two more qh to bulk in and out 
	EHCI_INTERNALS ehci_internals = {};
	ehci_internals.qh_count = 0x1;
	ehci_internals.qtd_count = 0x0;
	ehci_internals.first_qh = (uint32_t)qh_control;

	EHCI_QUEUE_HEAD* qh_bulk_out = NULL;
	EHCI_QUEUE_HEAD* qh_bulk_in = NULL;

	qh_bulk_out = alloc_qh(&ehci_internals);
	qh_bulk_in 	= alloc_qh(&ehci_internals);
	init_qh(qh_bulk_out, ehci_msb_internals->ehci_bulk_endp_max_s_out, ehci_msb_internals->ehci_bulk_endp_addr_out, ehci_msb_internals->ehci_addr);
	init_qh(qh_bulk_in, ehci_msb_internals->ehci_bulk_endp_max_s_in, ehci_msb_internals->ehci_bulk_endp_addr_in, ehci_msb_internals->ehci_addr);

	insert_qh(&ehci_internals, qh_bulk_out);
	insert_qh(&ehci_internals, qh_bulk_in);

	ehci_msb_qhs->ehci_bulk_out_qh = (uint32_t)qh_bulk_out;
	ehci_msb_qhs->ehci_bulk_in_qh = (uint32_t)qh_bulk_in;

	// print_string(vga_buffer, &index, "USBCMD pre-bulk_setup: ", strlen("USBCMD pre-bulk_setup: "));
	// print_dword(vga_buffer, &index, opreg_core->USBCMD);
	// index_c++;
	// index = index_c * VGA_WIDTH;

	// print_string(vga_buffer, &index, "USBSTS pre-bulk_setup: ", strlen("USBSTS pre-bulk_setup: "));
	// print_dword(vga_buffer, &index, opreg_core->USBSTS);
	// index_c++;
	// index = index_c * VGA_WIDTH;

	// Issue LUN command on control endp

	USB_DEVICE_REQUEST devReq_lun = {};
	devReq_lun.bmRequestType = 0xA1;
	devReq_lun.bRequest = USB_DR_REQUEST_MSD_GET_MAX_LUN;
	devReq_lun.wValue = 0x0;
	devReq_lun.wIndex = ehci_msb_internals->ehci_bulk_interface_nr;
	devReq_lun.wLength = 0x1;

	volatile uint8_t max_lun = 0x0;

	EHCI_QUEUE_TD* lun_qtd = NULL;
	lun_qtd = usb_control_transaction(&ehci_internals, &devReq_lun, (void*)&max_lun, 0x40, USB_CTRL_DEV_TO_HOST);
	qh_control->qtd_next = (uint32_t)lun_qtd;
	sleep(10);	

	// print_string(vga_buffer, &index, "MAX LUN: ", strlen("MAX LUN: "));
	// print_byte(vga_buffer, &index, max_lun);
	// index_c++;
	// index = index_c * VGA_WIDTH;
	
	// Issue INQUIRY SCSI command on bulk-only endp
	BULK_ONLY_CBW scsiReq_inquiry = {};
	scsiReq_inquiry.dCBWSignature = 0x43425355;		// Always 0x43425355
	scsiReq_inquiry.dCBWTag = 0x110;				// For now set it to 1
	scsiReq_inquiry.dCBWDataTransferLength = 0x24; 	// Set to 36 for now - minimal inquiry length
	scsiReq_inquiry.bmCBWFlags = 0x80;				// Device to host -> set bit 7
	scsiReq_inquiry.bCBWLUN = 0x0;					// Set LUN to 0
	scsiReq_inquiry.bCBWCBLength = 0x6;				// Inuqiry command block is 6 bytes long
		// SCSI Command Block
	scsiReq_inquiry.CBWCB.bits[0] = 0x12;			// OPCODE 
	scsiReq_inquiry.CBWCB.bits[1] = 0x0;			// Set EVPD and CMDDT to 0
	scsiReq_inquiry.CBWCB.bits[2] = 0x0; 			// Set to 0 when EVPD is 0
	scsiReq_inquiry.CBWCB.bits[3] = 0x0;			// High bits for allocaiton length
	scsiReq_inquiry.CBWCB.bits[4] = 0x24;			// Low bits for allocation length
	scsiReq_inquiry.CBWCB.bits[5] = 0x0;			// Set NACA to 0
	for (uint8_t i = 6; i < 16; ++i) scsiReq_inquiry.CBWCB.bits[i] = 0x0; // Set last 10 bytes to 0

	volatile uint8_t standard_inquiry_buf[0x24];
	uint8_t transaction_error = usb_bulk_transaction(&ehci_internals, ehci_msb_qhs, &scsiReq_inquiry, (void*)&standard_inquiry_buf, 0x200, 0x0);

	// print_string(vga_buffer, &index, "INQURY ERROR: ", strlen("INQURY ERROR: "));
	// print_byte(vga_buffer, &index, transaction_error);
	// index_c++;
	// index = index_c * VGA_WIDTH;

	uint32_t page_aligned_data = 0x0;

	// Issue scsi TEST UNIT READY command on bulk out endp
	BULK_ONLY_CBW scsiReq_testUnitReady = {};
	scsiReq_testUnitReady.dCBWSignature = 0x43425355;		// Always 0x43425355
	scsiReq_testUnitReady.dCBWTag = 0x213;					// For now set it to 1
	scsiReq_testUnitReady.dCBWDataTransferLength = 0x0; 	// Set to 36 for now - minimal inquiry length
	scsiReq_testUnitReady.bmCBWFlags = 0x80;				// Device to host -> set bit 7
	scsiReq_testUnitReady.bCBWLUN = 0x0;					// Set LUN to 0
	scsiReq_testUnitReady.bCBWCBLength = 0x6;				// Inuqiry command block is 6 bytes long
		// SCSI Command Block
	scsiReq_testUnitReady.CBWCB.bits[0] = 0x0;			// OPCODE: 0x00
	scsiReq_testUnitReady.CBWCB.bits[1] = 0x0;			// Reserved
	scsiReq_testUnitReady.CBWCB.bits[2] = 0x0; 			// Reserved
	scsiReq_testUnitReady.CBWCB.bits[3] = 0x0;			// Reserved
	scsiReq_testUnitReady.CBWCB.bits[4] = 0x0;			// Reserved
	scsiReq_testUnitReady.CBWCB.bits[5] = 0x0;			// CONTROL: set NACA to 0
	for (uint8_t i = 6; i < 16; ++i) scsiReq_testUnitReady.CBWCB.bits[i] = 0x0; // Set last 10 bytes to 0

	transaction_error = usb_bulk_transaction(&ehci_internals, ehci_msb_qhs, &scsiReq_testUnitReady, NULL, 0x200, 0x0);

	// print_string(vga_buffer, &index, "TEST UNIT READY ERROR: ", strlen("TEST UNIT READY ERROR: "));
	// print_byte(vga_buffer, &index, transaction_error);
	// index_c++;
	// index = index_c * VGA_WIDTH;

	// Issue scsi READ CAPACITY command on bulk out endp
	BULK_ONLY_CBW scsiReq_readCap = {};
	scsiReq_readCap.dCBWSignature = 0x43425355;		// Always 0x43425355
	scsiReq_readCap.dCBWTag = 0x5;					// For now set it to 1
	scsiReq_readCap.dCBWDataTransferLength = 0x8; 	// Set to 36 for now - minimal inquiry length
	scsiReq_readCap.bmCBWFlags = 0x80;				// Device to host -> set bit 7
	scsiReq_readCap.bCBWLUN = 0x0;					// Set LUN to 0
	scsiReq_readCap.bCBWCBLength = 0xA;				// Inuqiry command block is 6 bytes long
		// SCSI Command Block
	scsiReq_readCap.CBWCB.bits[0] = 0x25;			// OPCODE: 0x25
	scsiReq_readCap.CBWCB.bits[1] = 0x0;			// Reserved
	scsiReq_readCap.CBWCB.bits[2] = 0x0; 			// Reserved
	scsiReq_readCap.CBWCB.bits[3] = 0x0;			// Reserved
	scsiReq_readCap.CBWCB.bits[4] = 0x0;			// Reserved
	scsiReq_readCap.CBWCB.bits[5] = 0x0;			// CONTROL: set NACA to 0
	for (uint8_t i = 6; i < 16; ++i) scsiReq_readCap.CBWCB.bits[i] = 0x0; // Set last 10 bytes to 0

	volatile uint8_t readCap_buf[0x8];
	
	transaction_error = usb_bulk_transaction(&ehci_internals, ehci_msb_qhs, &scsiReq_readCap, (void*)&readCap_buf, 0x200, 0x0);

	// print_string(vga_buffer, &index, "READ CAPACITY ERROR: ", strlen("READ CAPACITY ERROR: "));
	// print_byte(vga_buffer, &index, transaction_error);
	// index_c++;
	// index = index_c * VGA_WIDTH;

	// print_SCSI_READ_CAPACITY(vga_buffer, &index, (SCSI_READ_CAPACITY*)&readCap_buf);
	// index_c = index / VGA_WIDTH; 

	// Issue scsi READ command on bulk out endp
	volatile uint8_t read_buf[0x400];
	
	uint8_t readError = EHCI_Read(usb_ehci, ehci_msb_qhs, (void*)&read_buf, 0x0, 0x2);

	// print_string(vga_buffer, &index, "READ ERROR: ", strlen("READ ERROR: "));
	// print_byte(vga_buffer, &index, readError);
	// index_c++;
	// index = index_c * VGA_WIDTH;

	// print_string(vga_buffer, &index, "Bootsector signature: 0x", strlen("Bootsector signature: 0x"));
	// print_byte(vga_buffer, &index, read_buf[510]);
	// print_byte(vga_buffer, &index, read_buf[511]);
	// index_c++;
	// index = index_c * VGA_WIDTH;

	// print_string(vga_buffer, &index, "Second sector last word: 0x", strlen("Second sector last word: 0x"));
	// print_byte(vga_buffer, &index, read_buf[1022]);
	// print_byte(vga_buffer, &index, read_buf[1023]);
	// index_c++;
	// index = index_c * VGA_WIDTH;

	print_string(vga_buffer, &index, "USBCMD post-bulk_setup: ", strlen("USBCMD post-bulk_setup: "));
	print_dword(vga_buffer, &index, opreg_core->USBCMD);
	index_c++;
	index = index_c * VGA_WIDTH;

	print_string(vga_buffer, &index, "USBSTS post-bulk_setup: ", strlen("USBSTS post-bulk_setup: "));
	print_dword(vga_buffer, &index, opreg_core->USBSTS);
	index_c++;
	index = index_c * VGA_WIDTH;

	return;

}

uint8_t EHCI_Read(USB_EHCI* usb_ehci, USB_EHCI_MSB_QUEUE_HEADS* ehci_msb_qhs, void* data, uint32_t lba_start, uint16_t lba_count) {

	EHCI_INTERNALS ehci_internals = {};
	ehci_internals.qh_count = 0x0;
	ehci_internals.qtd_count = 0x0;
	ehci_internals.first_qh = 0x0;

	uint32_t dataTransferLength = lba_count * 0x200;

	BULK_ONLY_CBW cbw = {};
	cbw.dCBWSignature = 0x43425355;						// Always 0x43425355
	cbw.dCBWTag = 0x5;									// Tag
	cbw.dCBWDataTransferLength = dataTransferLength;	// Set to 512 for now
	cbw.bmCBWFlags = 0x80;								// Device to host -> set bit 7
	cbw.bCBWLUN = 0x0;									// Set LUN to 0
	cbw.bCBWCBLength = 0xA;								// Read command block is 10 bytes long
		// SCSI Command Block
	cbw.CBWCB.bits[0] = 0x28;				// OPCODE: 0x25
	cbw.CBWCB.bits[1] = 0x0;				// RDPROTECT; DPO; FUA; RARC
	cbw.CBWCB.bits[2] = lba_start >> 0x18;	// LBA MSB
	cbw.CBWCB.bits[3] = lba_start >> 0x10;	// LBA
	cbw.CBWCB.bits[4] = lba_start >> 0x8;	// LBA
	cbw.CBWCB.bits[5] = lba_start;			// LBA LSB
	cbw.CBWCB.bits[6] = 0x0;				// Reserved; GROUP NUMBER
	cbw.CBWCB.bits[7] = lba_count >> 0x8;	// Transfer length high bytes
	cbw.CBWCB.bits[8] = lba_count;			// Transfer length low bytes
	cbw.CBWCB.bits[9] = 0x0;				// CONTROL: set NACA to 0
	for (uint8_t i = 10; i < 16; ++i) cbw.CBWCB.bits[i] = 0x0; // Set last 6 bytes to 0

	uint8_t transaction_error = usb_bulk_transaction(&ehci_internals, ehci_msb_qhs, &cbw, data, 0x200, 0x0);

	return transaction_error;

}

uint8_t usb_bulk_transaction(EHCI_INTERNALS* ehci_internals, USB_EHCI_MSB_QUEUE_HEADS* ehci_msb_qhs, BULK_ONLY_CBW* cbw, void* data, uint32_t max_packet_size, uint32_t transaction_direction) {

	// Get queue heads
	EHCI_QUEUE_HEAD* bulk_out_qh 	= (EHCI_QUEUE_HEAD*)ehci_msb_qhs->ehci_bulk_out_qh;
	EHCI_QUEUE_HEAD* bulk_in_qh 	= (EHCI_QUEUE_HEAD*)ehci_msb_qhs->ehci_bulk_in_qh;

	// Allocate transfer descriptors
	EHCI_QUEUE_TD* cmd_qtd 		= alloc_qtd(ehci_internals);
	EHCI_QUEUE_TD* data_qtd 	= NULL;
	EHCI_QUEUE_TD* data_prev 	= NULL;
	EHCI_QUEUE_TD* data_head 	= NULL;
	EHCI_QUEUE_TD* status_qtd 	= alloc_qtd(ehci_internals);

	// Allocate CSW
	BULK_ONLY_CSW csw = {};

	// Bulk out CBW
	init_qtd(cmd_qtd, NULL, ehci_msb_qhs->ehci_bulk_out_token, sizeof(BULK_ONLY_CBW), QTD_PID_OUT, (void*)cbw);
	ehci_msb_qhs->ehci_bulk_out_token ^= 0x1;	

	// Bulk in/out DATA
	if (cbw->dCBWDataTransferLength != 0x0) {
		uint32_t packet_size = 0x0;
		uint8_t* data_uint8 = (uint8_t*)data;
		uint8_t* data_end_uint8 = (uint8_t*)data + cbw->dCBWDataTransferLength;

		while (data_uint8 < data_end_uint8) {

			packet_size = data_end_uint8 - data_uint8;
			if (packet_size > max_packet_size) packet_size = max_packet_size;

			data_qtd = alloc_qtd(ehci_internals);
			if (data_head == NULL) {
				data_head = data_qtd;	
			}

			init_qtd(data_qtd, data_prev, ehci_msb_qhs->ehci_bulk_in_token, packet_size, QTD_PID_IN, (void*)data_uint8);
			ehci_msb_qhs->ehci_bulk_in_token ^= 0x1;
			data_prev = data_qtd;

			data_uint8 += max_packet_size;

		}
	}

	// Bulk in CSW
	init_qtd(status_qtd, data_prev, ehci_msb_qhs->ehci_bulk_in_token, sizeof(BULK_ONLY_CSW), QTD_PID_IN, (void*)&csw);
	ehci_msb_qhs->ehci_bulk_in_token ^= 0x1;

	bulk_out_qh->qtd_next = (uint32_t)cmd_qtd;
	while (cmd_qtd->token & (0x1 << QTD_STATUS_SHIFT));
	
	if (data_qtd == NULL) {
		bulk_in_qh->qtd_next = (uint32_t)status_qtd;
	}
	else {
		bulk_in_qh->qtd_next = (uint32_t)data_head;
	}
	while (status_qtd->token & (0x1 << QTD_STATUS_SHIFT));

	if (csw.dCSWTag == cbw->dCBWTag) {
		return 0;
	}

	return 1;

}

EHCI_QUEUE_TD* usb_control_transaction(EHCI_INTERNALS* ehci_internals, USB_DEVICE_REQUEST* req, void* data, uint32_t max_packet_size, uint32_t transaction_direction) {

	EHCI_QUEUE_TD* qtd 		= NULL;
	EHCI_QUEUE_TD* qtd_prev = NULL;
	EHCI_QUEUE_TD* qtd_head = NULL;
	uint32_t toggle 		= 0x0;
	uint32_t packet_size 	= 0x0;
	uint32_t data_PID 		= 0x0;
	uint32_t status_PID 	= 0x0;

	if (transaction_direction == USB_CTRL_DEV_TO_HOST) { data_PID = QTD_PID_IN; status_PID = QTD_PID_OUT; } 
	else { data_PID = QTD_PID_OUT; status_PID = QTD_PID_IN; }

	// SETUP PACKET
	qtd = alloc_qtd(ehci_internals);
	qtd_head = qtd;
	toggle = 0x0;
	init_qtd(qtd, NULL, toggle, sizeof(USB_DEVICE_REQUEST), QTD_PID_SETUP, (void*)req);
	qtd_prev = qtd;

	// IN/OUT PACKETS

	if (data != NULL) {
		uint8_t* data_uint8 = (uint8_t*)data;
		uint8_t* data_end_uint8 = (uint8_t*)data + req->wLength; 

		while (data_uint8 < data_end_uint8) {

			packet_size = data_end_uint8 - data_uint8;
			if (packet_size > max_packet_size) packet_size = max_packet_size;

			qtd = alloc_qtd(ehci_internals);
			toggle = toggle ^ 0x1;
			init_qtd(qtd, qtd_prev, toggle, packet_size, data_PID, (void*)data_uint8);
			qtd_prev = qtd;

			data_uint8 += max_packet_size;

		}
	}
	
	// STATUS PACKET

	qtd = alloc_qtd(ehci_internals);
	toggle = 0x1;
	init_qtd(qtd, qtd_prev, toggle, 0x0, status_PID, NULL);

	return qtd_head;

}


void init_qh(EHCI_QUEUE_HEAD* qh, uint32_t max_packet_size, uint32_t endp, uint32_t addr) {

	uint32_t endp_ch = (0x05 			<< QH_ENDP_CH_NAK_COUNT_RELOAD_SHIFT) |
					   (max_packet_size << QH_ENDP_CH_MAX_PACKET_LENGTH_SHIFT) |
					   (0x01 			<< QH_ENDP_CH_DATA_TOGGLE_CONTROL_SHIFT) |
					   (0x02			<< QH_ENDP_CH_ENDPOINT_SPEED_SHIFT) |
					   (endp 			<< QH_ENDP_CH_ENDPOINT_NUMBER_SHIFT) |
					   (addr 			<< QH_ENDP_CH_DEVICE_ADDRESS_SHIFT);

	uint32_t endp_cap = 0x01 << QH_ENDP_CAP_PIPE_MULT_SHIFT;

	qh->endp_ch = endp_ch;
	qh->endp_cap = endp_cap;
	qh->qtd_alt = QTD_PTR_TERMINATE;
	qh->qtd_curr = 0x0;
	qh->qtd_next = QTD_PTR_TERMINATE;
	qh->token = 0x0;
	for (uint8_t i = 0; i < 5; ++i) { qh->buf[i] = 0x0; qh->ext_buf[i] = 0x0; }

	return;
}


void init_qtd(EHCI_QUEUE_TD* qtd, EHCI_QUEUE_TD* prev, uint32_t toggle, uint32_t len, uint32_t pid, void* data) {

	/* TODO: don't set alternative pointer for every descriptor */
	if (prev != NULL) {
		prev->next = (uint32_t)qtd;
		prev->alt = (uint32_t)qtd;
	}

	qtd->next 	= QTD_PTR_TERMINATE;
	qtd->alt 	= QTD_PTR_TERMINATE;

	qtd->token 	= (toggle 	<< QTD_TOKEN_TOGGLE_SHIFT) |
				  (len 		<< QTD_BYTES_TO_TRANSFER_SHIFT) |
				  (0x03 	<< QTD_CERR_SHIFT) |
				  (pid 		<< QTD_PID_SHIFT) |
				  (0x01		<< QTD_STATUS_SHIFT);

	qtd->buf[0] = (uint32_t)data;
	qtd->ext_buf[0] = 0x0;
	uint32_t page_aligned_data = ((uint32_t)data) & (~0xFFF);
	for (uint8_t i = 1; i < 5; ++i) {
		page_aligned_data += 0x1000;
		qtd->buf[i] = page_aligned_data;
		qtd->ext_buf[i] = 0x0;
	} 

	return;

}

EHCI_QUEUE_HEAD* alloc_qh(EHCI_INTERNALS* ehci_internals) {

	EHCI_QUEUE_HEAD* qh = (EHCI_QUEUE_HEAD*)QH_BASE_ADDR;
	qh += ehci_internals->qh_count;
	ehci_internals->qh_count++;

	return qh;

}

EHCI_QUEUE_TD* alloc_qtd(EHCI_INTERNALS* ehci_internals) {

	EHCI_QUEUE_TD* qtd = (EHCI_QUEUE_TD*)QTD_BASE_ADDR;
	qtd += ehci_internals->qtd_count;
	ehci_internals->qtd_count++;

	return qtd;	

}

void insert_qh(EHCI_INTERNALS* ehci_internals, EHCI_QUEUE_HEAD* qh) {

	EHCI_QUEUE_HEAD* first_qh = (EHCI_QUEUE_HEAD*)ehci_internals->first_qh;
	qh->link = first_qh->link;
	first_qh->link = (uint32_t)qh | QH_PTR_QUEUE_HEAD;

	return;

}
void wait_for_qh(EHCI_QUEUE_HEAD* qh) {

	uint32_t complete = 0x0;

	while (complete == 0x0) {

		if (qh->qtd_next == QTD_PTR_TERMINATE) {

			if ((qh->token & 0x80) == 0x0) {

				complete = 0x1;

			}

		}

	}

	return;

}

void reset_ports(EHCI_OPREG_AUX* opreg_aux, uint8_t n_ports) {

	volatile uint32_t* portsc = NULL;

	portsc = &opreg_aux->PORTSC;
	for (uint8_t i = 0; i < n_ports; ++i) {

		set_EHCI_portsc(opreg_aux, i, (*portsc | PORTSC_RESET) & ~PORTSC_ENABLED);
		portsc++;

	}

	sleep(50);

	portsc = &opreg_aux->PORTSC;
	for (uint8_t i = 0; i < n_ports; ++i) {

		set_EHCI_portsc(opreg_aux, i, (*portsc) & ~PORTSC_RESET);
		portsc++;

	}

	sleep(50);

	return;

}

void set_EHCI_portsc(EHCI_OPREG_AUX* opreg_aux, uint32_t port, uint32_t value) {

	volatile uint32_t* portsc = &opreg_aux->PORTSC + port;
	*portsc = value;

	return;

}

void print_SCSI_READ_CAPACITY(uint16_t* const vga_buffer, size_t* const index, const SCSI_READ_CAPACITY* const scsi_readCap) {

	size_t index_c = (*index) / VGA_WIDTH;

	print_string(vga_buffer, index, "SCSI READ CAPACITY returnedLogicalBlockAddress: ", strlen("SCSI READ CAPACITY returnedLogicalBlockAddress: "));
	for (uint8_t i = 0; i < 4; ++i) print_byte(vga_buffer, index, scsi_readCap->returnedLogicalBlockAddress[i]);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "SCSI READ CAPACITY blockLengthInBytes: ", strlen("SCSI READ CAPACITY blockLengthInBytes: "));
	for (uint8_t i = 0; i < 4; ++i) print_byte(vga_buffer, index, scsi_readCap->blockLengthInBytes[i]);
	index_c++;
	*index = index_c * VGA_WIDTH;

	return;

}

void print_BULK_ONLY_CSW(uint16_t* const vga_buffer, size_t* const index, const BULK_ONLY_CSW* const csw) {

	size_t index_c = (*index) / VGA_WIDTH;

	print_string(vga_buffer, index, "BULK ONLY CSW dCSWSignature: ", strlen("BULK ONLY CSW dCSWSignature: "));
	print_dword(vga_buffer, index, csw->dCSWSignature);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "BULK ONLY CSW dCSWTag: ", strlen("BULK ONLY CSW dCSWTag: "));
	print_dword(vga_buffer, index, csw->dCSWTag);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "BULK ONLY CSW dCSWDataResidue: ", strlen("BULK ONLY CSW dCSWDataResidue: "));
	print_dword(vga_buffer, index, csw->dCSWDataResidue);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "BULK ONLY CSW bCSWStatus: ", strlen("BULK ONLY CSW bCSWStatus: "));
	print_byte(vga_buffer, index, csw->bCSWStatus);
	index_c++;
	*index = index_c * VGA_WIDTH;

	return;
}

void print_USB_ENDPOINT_DESCRIPTOR(uint16_t* const vga_buffer, size_t* const index, const USB_ENDPOINT_DESCRIPTOR* const endpDesc) {

	size_t index_c = (*index) / VGA_WIDTH;

	print_string(vga_buffer, index, "USB ENDP DESC bLength: ", strlen("USB ENDP DESC bLength: "));
	print_byte(vga_buffer, index, endpDesc->bLength);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "USB ENDP DESC bDescriptorType: ", strlen("USB ENDP DESC bDescriptorType: "));
	print_byte(vga_buffer, index, endpDesc->bDescriptorType);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "USB ENDP DESC bEndpointAddress: ", strlen("USB ENDP DESC bEndpointAddress: "));
	print_byte(vga_buffer, index, endpDesc->bEndpointAddress);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "USB ENDP DESC bmAttributes: ", strlen("USB ENDP DESC bmAttributes: "));
	print_byte(vga_buffer, index, endpDesc->bmAttributes);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "USB ENDP DESC wMaxPacketSize: ", strlen("USB ENDP DESC wMaxPacketSize: "));
	print_word(vga_buffer, index, endpDesc->wMaxPacketSize);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "USB ENDP DESC bInterval: ", strlen("USB ENDP DESC bInterval: "));
	print_byte(vga_buffer, index, endpDesc->bInterval);
	index_c++;
	*index = index_c * VGA_WIDTH;

	return;

}

void print_USB_INTERFACE_DESCRIPTOR(uint16_t* const vga_buffer, size_t* const index, const USB_INTERFACE_DESCRIPTOR* const interfaceDesc) {

	size_t index_c = (*index) / VGA_WIDTH;

	print_string(vga_buffer, index, "USB INTERFACE DESC bLength: ", strlen("USB INTERFACE DESC bLength: "));
	print_byte(vga_buffer, index, interfaceDesc->bLength);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "USB INTERFACE DESC bDescriptorType: ", strlen("USB INTERFACE DESC bDescriptorType: "));
	print_byte(vga_buffer, index, interfaceDesc->bDescriptorType);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "USB INTERFACE DESC bInterfaceNumber: ", strlen("USB INTERFACE DESC bInterfaceNumber: "));
	print_byte(vga_buffer, index, interfaceDesc->bInterfaceNumber);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "USB INTERFACE DESC bAlternateSetting: ", strlen("USB INTERFACE DESC bAlternateSetting: "));
	print_byte(vga_buffer, index, interfaceDesc->bAlternateSetting);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "USB INTERFACE DESC bNumEndpoints: ", strlen("USB INTERFACE DESC bNumEndpoints: "));
	print_byte(vga_buffer, index, interfaceDesc->bNumEndpoints);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "USB INTERFACE DESC bInterfaceClass: ", strlen("USB INTERFACE DESC bInterfaceClass: "));
	print_byte(vga_buffer, index, interfaceDesc->bInterfaceClass);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "USB INTERFACE DESC bInterfaceSubClass: ", strlen("USB INTERFACE DESC bInterfaceSubClass: "));
	print_byte(vga_buffer, index, interfaceDesc->bInterfaceSubClass);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "USB INTERFACE DESC bInterfaceProtocol: ", strlen("USB INTERFACE DESC bInterfaceProtocol: "));
	print_byte(vga_buffer, index, interfaceDesc->bInterfaceProtocol);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "USB INTERFACE DESC iInterface: ", strlen("USB INTERFACE DESC iInterface: "));
	print_byte(vga_buffer, index, interfaceDesc->iInterface);
	index_c++;
	*index = index_c * VGA_WIDTH;

	return;

}

void print_USB_CONFIGURATION_DESCRIPTOR(uint16_t* const vga_buffer, size_t* const index, const USB_CONFIGURATION_DESCRIPTOR* const confDesc) {

	size_t index_c = (*index) / VGA_WIDTH;

	print_string(vga_buffer, index, "USB CONF DESC bLength: ", strlen("USB CONF DESC bLength: "));
	print_byte(vga_buffer, index, confDesc->bLength);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "USB CONF DESC bDescriptorType: ", strlen("USB CONF DESC bDescriptorType: "));
	print_byte(vga_buffer, index, confDesc->bDescriptorType);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "USB CONF DESC wTotalLength: ", strlen("USB CONF DESC wTotalLength: "));
	print_word(vga_buffer, index, confDesc->wTotalLength);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "USB CONF DESC bNumInterfaces: ", strlen("USB CONF DESC bNumInterfaces: "));
	print_byte(vga_buffer, index, confDesc->bNumInterfaces);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "USB CONF DESC bConfigurationValue: ", strlen("USB CONF DESC bConfigurationValue: "));
	print_byte(vga_buffer, index, confDesc->bConfigurationValue);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "USB CONF DESC iConfiguration: ", strlen("USB CONF DESC iConfiguration: "));
	print_byte(vga_buffer, index, confDesc->iConfiguration);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "USB CONF DESC bmAttributes: ", strlen("USB CONF DESC bmAttributes: "));
	print_byte(vga_buffer, index, confDesc->bmAttributes);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "USB CONF DESC bMaxPower: ", strlen("USB CONF DESC bMaxPower: "));
	print_byte(vga_buffer, index, confDesc->bMaxPower);
	index_c++;
	*index = index_c * VGA_WIDTH;

	return;

}

void print_USB_DEVICE_DESCRIPTOR(uint16_t* const vga_buffer, size_t* const index, const USB_DEVICE_DESCRIPTOR* const devDesc) {

	size_t index_c = (*index) / VGA_WIDTH;

	print_string(vga_buffer, index, "USB DEV_DESC bLength: ", strlen("USB DEV_DESC bLength: "));
	print_byte(vga_buffer, index, devDesc->bLength);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "USB DEV_DESC bDescriptorType: ", strlen("USB DEV_DESC bDescriptorType: "));
	print_byte(vga_buffer, index, devDesc->bDescriptorType);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "USB DEV_DESC bcdUSB: ", strlen("USB DEV_DESC bcdUSB: "));
	print_word(vga_buffer, index, devDesc->bcdUSB);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "USB DEV_DESC bDeviceClass: ", strlen("USB DEV_DESC bDeviceClass: "));
	print_byte(vga_buffer, index, devDesc->bDeviceClass);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "USB DEV_DESC bDeviceSubClass: ", strlen("USB DEV_DESC bDeviceSubClass: "));
	print_byte(vga_buffer, index, devDesc->bDeviceSubClass);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "USB DEV_DESC bDeviceProtocol: ", strlen("USB DEV_DESC bDeviceProtocol: "));
	print_byte(vga_buffer, index, devDesc->bDeviceProtocol);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "USB DEV_DESC bMaxPacketSize0: ", strlen("USB DEV_DESC bMaxPacketSize0: "));
	print_byte(vga_buffer, index, devDesc->bMaxPacketSize0);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "USB DEV_DESC idVendor: ", strlen("USB DEV_DESC idVendor: "));
	print_word(vga_buffer, index, devDesc->idVendor);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "USB DEV_DESC idProduct: ", strlen("USB DEV_DESC idProduct: "));
	print_word(vga_buffer, index, devDesc->idProduct);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "USB DEV_DESC bcdDevice: ", strlen("USB DEV_DESC bcdDevice: "));
	print_word(vga_buffer, index, devDesc->bcdDevice);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "USB DEV_DESC iManufacturer: ", strlen("USB DEV_DESC iManufacturer: "));
	print_byte(vga_buffer, index, devDesc->iManufacturer);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "USB DEV_DESC iProduct: ", strlen("USB DEV_DESC iProduct: "));
	print_byte(vga_buffer, index, devDesc->iProduct);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "USB DEV_DESC iSerialNumber: ", strlen("USB DEV_DESC iSerialNumber: "));
	print_byte(vga_buffer, index, devDesc->iSerialNumber);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "USB DEV_DESC bNumConfigurations: ", strlen("USB DEV_DESC bNumConfigurations: "));
	print_byte(vga_buffer, index, devDesc->bNumConfigurations);
	index_c++;
	*index = index_c * VGA_WIDTH;

	return;

}

void print_EHCI_QUEUE_TD(uint16_t* const vga_buffer, size_t* const index, const EHCI_QUEUE_TD* const qtd) {

	size_t index_c = (*index) / VGA_WIDTH;

	print_string(vga_buffer, index, "EHCI qTD qTD next: ", strlen("EHCI qTD qTD next: "));
	print_dword(vga_buffer, index, qtd->next);

	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "EHCI qTD qTD alternative: ", strlen("EHCI qTD qTD alternative: "));
	print_dword(vga_buffer, index, qtd->alt);

	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "EHCI qTD token: ", strlen("EHCI qTD token: "));
	print_dword(vga_buffer, index, qtd->token);

	index_c++;
	*index = index_c * VGA_WIDTH;

	for (uint8_t i = 0; i < 5; ++i) {

		print_string(vga_buffer, index, "EHCI qTD buffer: ", strlen("EHCI qTD buffer: "));
		print_dword(vga_buffer, index, qtd->buf[i]);

		index_c++;
		*index = index_c * VGA_WIDTH;

	}

	return;

}

void print_EHCI_QUEUE_HEAD(uint16_t* const vga_buffer, size_t* const index, const EHCI_QUEUE_HEAD* const qh) {

	size_t index_c = (*index) / VGA_WIDTH;

	print_string(vga_buffer, index, "EHCI QH horizontal link: ", strlen("EHCI QH horizontal link: "));
	print_dword(vga_buffer, index, qh->link);

	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "EHCI QH endp characteristics: ", strlen("EHCI QH endp characteristics: "));
	print_dword(vga_buffer, index, qh->endp_ch);

	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "EHCI QH endp capabilities: ", strlen("EHCI QH endp capabilities: "));
	print_dword(vga_buffer, index, qh->endp_cap);

	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "EHCI QH qTD current: ", strlen("EHCI QH qTD current: "));
	print_dword(vga_buffer, index, qh->qtd_curr);

	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "EHCI QH qTD next: ", strlen("EHCI QH qTD next: "));
	print_dword(vga_buffer, index, qh->qtd_next);

	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "EHCI QH qTD alternative: ", strlen("EHCI QH qTD alternative: "));
	print_dword(vga_buffer, index, qh->qtd_alt);

	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "EHCI QH token: ", strlen("EHCI QH token: "));
	print_dword(vga_buffer, index, qh->token);

	index_c++;
	*index = index_c * VGA_WIDTH;

	for (uint8_t i = 0; i < 5; ++i) {

		print_string(vga_buffer, index, "EHCI QH buffer: ", strlen("EHCI QH buffer: "));
		print_dword(vga_buffer, index, qh->buf[i]);

		index_c++;
		*index = index_c * VGA_WIDTH;

	}

	return;

}

void print_EHCI_CAPREG_HCCPARAMS_F(uint16_t* const vga_buffer, size_t* const index, const EHCI_CAPREG_HCCPARAMS_F* const hccparams) {

	size_t index_c = (*index) / VGA_WIDTH;

	print_string(vga_buffer, index, "ehci CAPREG HCCPARAMS EECP: ", strlen("ehci CAPREG HCCPARAMS EECP: "));
	print_byte(vga_buffer, index, hccparams->EECP);

	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "ehci CAPREG HCCPARAMS isochronous scheduling treshold: ", strlen("ehci CAPREG HCCPARAMS isochronous scheduling treshold: "));
	print_byte(vga_buffer, index, hccparams->isochronous_scheduling_treshold);

	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "ehci CAPREG HCCPARAMS asynchronous schedule park capability: ", strlen("ehci CAPREG HCCPARAMS asynchronous schedule park capability: "));
	print_byte(vga_buffer, index, hccparams->async_schedule_park_capability);

	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "ehci CAPREG HCCPARAMS programmable frame list flag: ", strlen("ehci CAPREG HCCPARAMS programmable frame list flag: "));
	print_byte(vga_buffer, index, hccparams->programmable_frame_list_flag);

	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "ehci CAPREG HCCPARAMS 64-bit addressing capability: ", strlen("ehci CAPREG HCCPARAMS 64-bit addressing capability: "));
	print_byte(vga_buffer, index, hccparams->addressing_capability_64_bit);	

	index_c++;
	*index = index_c * VGA_WIDTH;

	return;

}

void print_EHCI_CAPREG_HCSPARAMS_F(uint16_t* const vga_buffer, size_t* const index, const EHCI_CAPREG_HCSPARAMS_F* const hcsparams) {

	size_t index_c = (*index) / VGA_WIDTH;

	print_string(vga_buffer, index, "ehci CAPREG HCSPARAMS debug port number: ", strlen("ehci CAPREG HCSPARAMS debug port number: "));
	print_byte(vga_buffer, index, hcsparams->debug_port_number);

	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "ehci CAPREG HCSPARAMS P_INDICATOR: ", strlen("ehci CAPREG HCSPARAMS P_INDICATOR: "));
	print_byte(vga_buffer, index, hcsparams->P_INDICATOR);

	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "ehci CAPREG HCSPARAMS N_CC: ", strlen("ehci CAPREG HCSPARAMS N_CC: "));
	print_byte(vga_buffer, index, hcsparams->N_CC);

	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "ehci CAPREG HCSPARAMS N_PCC: ", strlen("ehci CAPREG HCSPARAMS N_PCC: "));
	print_byte(vga_buffer, index, hcsparams->N_PCC);

	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "ehci CAPREG HCSPARAMS port routing rules: ", strlen("ehci CAPREG HCSPARAMS port routing rules: "));
	print_byte(vga_buffer, index, hcsparams->port_routing_rules);

	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "ehci CAPREG HCSPARAMS PPC: ", strlen("ehci CAPREG HCSPARAMS PPC: "));
	print_byte(vga_buffer, index, hcsparams->PPC);	

	index_c++;
	*index = index_c * VGA_WIDTH;	

	print_string(vga_buffer, index, "ehci CAPREG HCSPARAMS N_PORTS: ", strlen("ehci CAPREG HCSPARAMS N_PORTS: "));
	print_byte(vga_buffer, index, hcsparams->N_PORTS);	

	index_c++;
	*index = index_c * VGA_WIDTH;	

	return;

}

void print_EHCI_PCIREG_USBLEGSUP(uint16_t* const vga_buffer, size_t* const index, const EHCI_PCIREG_USBLEGSUP* const usblegsup) {

	size_t index_c = (*index) / VGA_WIDTH;

	print_string(vga_buffer, index, "PCI ehci USBLEGSUP CAPABILITY_ID: ", strlen("PCI ehci USBLEGSUP CAPABILITY_ID: "));
	print_byte(vga_buffer, index, usblegsup->CAPABILITY_ID);

	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "PCI ehci USBLEGSUP EHCI_EXT_CAP_POINTER: ", strlen("PCI ehci USBLEGSUP EHCI_EXT_CAP_POINTER: "));
	print_byte(vga_buffer, index, usblegsup->EHCI_EXT_CAP_POINTER);

	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "PCI ehci USBLEGSUP HC_BIOS_OWNED_SEMAPHORE: ", strlen("PCI ehci USBLEGSUP HC_BIOS_OWNED_SEMAPHORE: "));
	print_byte(vga_buffer, index, usblegsup->HC_BIOS_OWNED_SEMAPHORE);

	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "PCI ehci USBLEGSUP HC_OS_OWNED_SEMAPHORE: ", strlen("PCI ehci USBLEGSUP HC_OS_OWNED_SEMAPHORE: "));
	print_byte(vga_buffer, index, usblegsup->HC_OS_OWNED_SEMAPHORE);

	index_c++;
	*index = index_c * VGA_WIDTH;

	return;

}



