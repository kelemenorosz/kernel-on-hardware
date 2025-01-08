#ifndef FS_H
#define FS_H

#include "ehci.h"

typedef struct __attribute__((__packed__)) MBR_PARTITION_DATA {

	uint8_t bootFlag;
	uint8_t chsPartitionStart[3];
	uint8_t partitionType;
	uint8_t chsPartitionEnd[3];
	uint32_t lbaPartitionStart;
	uint32_t sectorsInPartition;

} MBR_PARTITION_DATA;

typedef struct __attribute__((__packed__)) MBR_DATA {

	uint32_t uniqueDiskID;
	MBR_PARTITION_DATA partitions[4];

} MBR_DATA;

typedef struct __attribute__((__packed__)) BPB_DATA {

	uint8_t jumpOpcode[3];
	uint8_t oemIdentifier[8];
	uint16_t bytesPerSector;
	uint8_t sectorsPerCluster;
	uint16_t reservedSectors;
	uint8_t numberOfFATs;
	uint16_t numberOfRootDirectories;
	uint16_t smallSectorCount;
	uint8_t mediaDescriptorType;
	uint16_t sectorsPerFAT;
	uint16_t sectorsPerTrack;
	uint16_t numberOfHeadsOnStorageMedia;
	uint32_t hiddenSectors;
	uint32_t largeSectorCount;

} BPB_DATA;

typedef struct __attribute__((__packed__)) EBR_DATA {

	uint32_t sectorsPerFAT;
	uint16_t flags;
	uint16_t fatVersionNumber;
	uint32_t rootDirectoryClusterNumber;
	uint16_t FSInfoSectorNumber;
	uint16_t backupBootSectorSectorNumber;
	uint8_t reserved[12];
	uint8_t driveNumber;
	uint8_t windowsNTFlags;
	uint8_t signature;
	uint32_t volumeID;
	uint8_t volumeLabel[11];
	uint8_t systemIdentifier[8];

} EBR_DATA;

typedef struct __attribute__((__packed__)) FSINFO_DATA {

	uint32_t leadSignature;
	uint32_t anotherSignature;
	uint32_t lastKnownFreeClusterCount;
	uint32_t availableClustersStart;
	uint32_t trailSignature;

} FSINFO_DATA;


typedef struct __attribute__((__packed__)) FAT32_DIR_ENTRY {

	uint8_t filename[11];
	uint8_t attributes;
	uint8_t reserved;
	uint8_t creationTime;
	uint16_t fileTime;
	uint16_t fileDate;
	uint16_t lastAccessDate;
	uint16_t firstClusterHighBits;
	uint16_t lastModificationTime;
	uint16_t lastModificationDate;
	uint16_t firstClusterLowBits;
	uint32_t fileSizeInBytes;

} FAT32_DIR_ENTRY;

typedef struct __attribute__((__packed__)) FAT32_DIR_ENTRY_EXT {

	uint8_t orderOfEntry;
	uint8_t firstString[10];
	uint8_t attributes;
	uint8_t longEntryType;
	uint8_t checksum;
	uint8_t middleString[12];
	uint8_t reserved[2];
	uint8_t finalString[4]; 

} FAT32_DIR_ENTRY_EXT;

typedef struct __attribute__((__packed__)) FILE_DATA {

	uint32_t startCluster;
	uint8_t filename[11];

} FILE_DATA;

typedef struct __attribute__((__packed__)) FILE_QUERY_DESCRIPTOR {

	char file_name[8];
	char file_ext[3];
	size_t file_name_size;
	size_t file_ext_size;

} FILE_QUERY_DESCRIPTOR;

void read_fileA(USB_EHCI* usb_ehci, USB_EHCI_MSB_QUEUE_HEADS* ehci_msb_qhs, void* buf, FILE_DATA* file_data, uint32_t fat_lba, uint32_t data_start_lba, uint32_t sectorsPerCluster);
void parse_directory_findA(void* directory, FILE_DATA* file_data, FILE_QUERY_DESCRIPTOR* file_query);
void parse_FSINFO(void* fsinfo, FSINFO_DATA* fsinfo_data);
void parse_BPB_EBR(void* bpb, BPB_DATA* bpb_data, EBR_DATA* ebr_data);
void parse_MBR(void* mbr, MBR_DATA* mbr_data);

void print_FSINFO_DATA(uint16_t* const vga_buffer, size_t* const index, const FSINFO_DATA* const fsinfo_data);
void print_EBR_DATA(uint16_t* const vga_buffer, size_t* const index, const EBR_DATA* const ebr_data);
void print_BPB_DATA(uint16_t* const vga_buffer, size_t* const index, const BPB_DATA* const bpb_data);
void print_MBR_PARITION_DATA(uint16_t* const vga_buffer, size_t* const index, const MBR_PARTITION_DATA* const mbr_partition_data);

#endif /* FS_H */