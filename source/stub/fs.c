#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "fs.h"
#include "memory.h"
#include "string.h"
#include "print.h"

extern const size_t VGA_WIDTH;

void parse_MBR(void* mbr, MBR_DATA* mbr_data) {

	mbr_data->uniqueDiskID = *((uint32_t*)mbr + 0x6E);
	memcpy(&mbr_data->partitions[0], mbr + 0x1BE, 0x40);

	return;

}

void parse_BPB_EBR(void* bpb, BPB_DATA* bpb_data, EBR_DATA* ebr_data) {

	memcpy(bpb_data, bpb, sizeof(BPB_DATA));
	memcpy(ebr_data, bpb + 0x24, sizeof(EBR_DATA));

	return;

}

void parse_FSINFO(void* fsinfo, FSINFO_DATA* fsinfo_data) {

	fsinfo_data->leadSignature 				= *((uint32_t*)fsinfo);
	fsinfo_data->anotherSignature 			= *((uint32_t*)(fsinfo + 0x1E4));
	fsinfo_data->lastKnownFreeClusterCount 	= *((uint32_t*)(fsinfo + 0x1E8));
	fsinfo_data->availableClustersStart 	= *((uint32_t*)(fsinfo + 0x1EC));
	fsinfo_data->trailSignature 			= *((uint32_t*)(fsinfo + 0x1FC));

	return;

}

void parse_directory_findA(void* directory, FILE_DATA* file_data, FILE_QUERY_DESCRIPTOR* file_query) {

	uint8_t* directory_uint8 = (uint8_t*)directory;
	uint8_t previous_entry_is_ext = 0x0;
	while (*(directory_uint8 + 0xB) != 0x0) {

		if (*(directory_uint8 + 0xB) == 0xF) {
			// Extended file entry
			directory_uint8 += sizeof(FAT32_DIR_ENTRY_EXT);
			previous_entry_is_ext = 0x1;
		}
		else {
			// Normal file entry
			if (!previous_entry_is_ext){
				FAT32_DIR_ENTRY* dir_entry = (FAT32_DIR_ENTRY*)directory_uint8;
				if (strncmp((char*)&dir_entry->filename, (char*)&file_query->file_name, file_query->file_name_size) == 0x0 &&
				 strncmp((char*)&dir_entry->filename[8], (char*)&file_query->file_ext, file_query->file_ext_size) == 0x0) {
					file_data->startCluster = (dir_entry->firstClusterHighBits << 0x10) + (dir_entry->firstClusterLowBits);
					memcpy((void*)&file_data->filename, (void*)&dir_entry->filename, 0xB);
				}
			}

			directory_uint8 += sizeof(FAT32_DIR_ENTRY);
			previous_entry_is_ext = 0x0;
		}

	} 

	return;

}

void read_fileA(USB_EHCI* usb_ehci, USB_EHCI_MSB_QUEUE_HEADS* ehci_msb_qhs, void* buf, FILE_DATA* file_data, uint32_t fat_lba, uint32_t data_start_lba, uint32_t sectorsPerCluster) {

	// Read 32 sectors of FAT
	volatile uint8_t fat_buf[0x4000];
	uint32_t file_cluster = file_data->startCluster;
	uint32_t file_offset = 0x0;

	while (file_cluster) {

		uint32_t start_sector_offset = file_cluster;
		start_sector_offset = start_sector_offset >> 0xC;
		start_sector_offset = start_sector_offset << 0x5;
		uint32_t fat_offset = fat_lba + start_sector_offset;
		EHCI_Read(usb_ehci, ehci_msb_qhs, (void*)&fat_buf, fat_offset, 0x20); 

		uint32_t cluster_lba = file_cluster;
		cluster_lba -= 2;
		cluster_lba *= sectorsPerCluster;
		cluster_lba += data_start_lba;
		EHCI_Read(usb_ehci, ehci_msb_qhs, buf + file_offset, cluster_lba, sectorsPerCluster);
		file_offset += sectorsPerCluster * 0x200; // Should be replaced by bytesPerSector

		uint32_t next_cluster_in_fat = file_cluster;
		next_cluster_in_fat &= 0xFFF;
		next_cluster_in_fat *= 0x4;
		uint32_t next_cluster_entry_value = *(uint32_t*)(fat_buf + next_cluster_in_fat);
		next_cluster_entry_value &= 0x0FFFFFFF;
		
		if (next_cluster_entry_value < 0x0FFFFFF8) {
			file_cluster = next_cluster_entry_value;
		}
		else {
			file_cluster = 0x0;
		}

	}

	return;

}

void print_FSINFO_DATA(uint16_t* const vga_buffer, size_t* const index, const FSINFO_DATA* const fsinfo_data) {

	size_t index_c = (*index) / VGA_WIDTH;

	print_string(vga_buffer, index, "FSINFO_DATA leadSignature: ", strlen("FSINFO_DATA leadSignature: "));
	print_dword(vga_buffer, index, fsinfo_data->leadSignature);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "FSINFO_DATA anotherSignature: ", strlen("FSINFO_DATA anotherSignature: "));
	print_dword(vga_buffer, index, fsinfo_data->anotherSignature);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "FSINFO_DATA lastKnownFreeClusterCount: ", strlen("FSINFO_DATA lastKnownFreeClusterCount: "));
	print_dword(vga_buffer, index, fsinfo_data->lastKnownFreeClusterCount);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "FSINFO_DATA availableClustersStart: ", strlen("FSINFO_DATA availableClustersStart: "));
	print_dword(vga_buffer, index, fsinfo_data->availableClustersStart);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "FSINFO_DATA trailSignature: ", strlen("FSINFO_DATA trailSignature: "));
	print_dword(vga_buffer, index, fsinfo_data->trailSignature);
	index_c++;
	*index = index_c * VGA_WIDTH;

	return;

}

void print_EBR_DATA(uint16_t* const vga_buffer, size_t* const index, const EBR_DATA* const ebr_data) {

	size_t index_c = (*index) / VGA_WIDTH;

	print_string(vga_buffer, index, "EBR_DATA sectorsPerFAT: ", strlen("EBR_DATA sectorsPerFAT: "));
	print_dword(vga_buffer, index, ebr_data->sectorsPerFAT);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "EBR_DATA flags: ", strlen("EBR_DATA flags: "));
	print_word(vga_buffer, index, ebr_data->flags);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "EBR_DATA fatVersionNumber: ", strlen("EBR_DATA fatVersionNumber: "));
	print_byte(vga_buffer, index, ebr_data->fatVersionNumber >> 0x8);
	print_string(vga_buffer, index, ".", strlen("."));
	print_byte(vga_buffer, index, ebr_data->fatVersionNumber);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "EBR_DATA rootDirectoryClusterNumber: ", strlen("EBR_DATA rootDirectoryClusterNumber: "));
	print_dword(vga_buffer, index, ebr_data->rootDirectoryClusterNumber);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "EBR_DATA FSInfoSectorNumber: ", strlen("EBR_DATA FSInfoSectorNumber: "));
	print_word(vga_buffer, index, ebr_data->FSInfoSectorNumber);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "EBR_DATA backupBootSectorSectorNumber: ", strlen("EBR_DATA backupBootSectorSectorNumber: "));
	print_word(vga_buffer, index, ebr_data->backupBootSectorSectorNumber);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "EBR_DATA driveNumber: ", strlen("EBR_DATA driveNumber: "));
	print_byte(vga_buffer, index, ebr_data->driveNumber);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "EBR_DATA windowsNTFlags: ", strlen("EBR_DATA windowsNTFlags: "));
	print_byte(vga_buffer, index, ebr_data->windowsNTFlags);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "EBR_DATA signature: ", strlen("EBR_DATA signature: "));
	print_byte(vga_buffer, index, ebr_data->signature);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "EBR_DATA volumeID: ", strlen("EBR_DATA volumeID: "));
	print_dword(vga_buffer, index, ebr_data->volumeID);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "EBR_DATA volumeLabel: ", strlen("EBR_DATA volumeLabel: "));
	print_string(vga_buffer, index, (char*)&ebr_data->volumeLabel, 0xB);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "EBR_DATA systemIdentifier: ", strlen("EBR_DATA systemIdentifier: "));
	print_string(vga_buffer, index, (char*)&ebr_data->systemIdentifier, 0x8);
	index_c++;
	*index = index_c * VGA_WIDTH;

	return;

}

void print_BPB_DATA(uint16_t* const vga_buffer, size_t* const index, const BPB_DATA* const bpb_data) {

	size_t index_c = (*index) / VGA_WIDTH;

	print_string(vga_buffer, index, "BPB_DATA jumpOpcode: ", strlen("BPB_DATA jumpOpcode: "));
	for (uint8_t i = 0; i < 3; ++i) print_byte(vga_buffer, index, bpb_data->jumpOpcode[i]);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "BPB_DATA oemIdentifier: ", strlen("BPB_DATA oemIdentifier: "));
	print_string(vga_buffer, index, (char*)&bpb_data->oemIdentifier, 0x8);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "BPB_DATA bytesPerSector: ", strlen("BPB_DATA bytesPerSector: "));
	print_word(vga_buffer, index, bpb_data->bytesPerSector);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "BPB_DATA sectorsPerCluster: ", strlen("BPB_DATA sectorsPerCluster: "));
	print_byte(vga_buffer, index, bpb_data->sectorsPerCluster);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "BPB_DATA reservedSectors: ", strlen("BPB_DATA reservedSectors: "));
	print_word(vga_buffer, index, bpb_data->reservedSectors);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "BPB_DATA numberOfFATs: ", strlen("BPB_DATA numberOfFATs: "));
	print_byte(vga_buffer, index, bpb_data->numberOfFATs);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "BPB_DATA numberOfRootDirectories: ", strlen("BPB_DATA numberOfRootDirectories: "));
	print_word(vga_buffer, index, bpb_data->numberOfRootDirectories);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "BPB_DATA smallSectorCount: ", strlen("BPB_DATA smallSectorCount: "));
	print_word(vga_buffer, index, bpb_data->smallSectorCount);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "BPB_DATA mediaDescriptorType: ", strlen("BPB_DATA mediaDescriptorType: "));
	print_byte(vga_buffer, index, bpb_data->mediaDescriptorType);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "BPB_DATA sectorsPerFAT: ", strlen("BPB_DATA sectorsPerFAT: "));
	print_word(vga_buffer, index, bpb_data->sectorsPerFAT);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "BPB_DATA sectorsPerTrack: ", strlen("BPB_DATA sectorsPerTrack: "));
	print_word(vga_buffer, index, bpb_data->sectorsPerTrack);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "BPB_DATA numberOfHeadsOnStorageMedia: ", strlen("BPB_DATA numberOfHeadsOnStorageMedia: "));
	print_word(vga_buffer, index, bpb_data->numberOfHeadsOnStorageMedia);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "BPB_DATA hiddenSectors: ", strlen("BPB_DATA hiddenSectors: "));
	print_dword(vga_buffer, index, bpb_data->hiddenSectors);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	print_string(vga_buffer, index, "BPB_DATA largeSectorCount: ", strlen("BPB_DATA largeSectorCount: "));
	print_dword(vga_buffer, index, bpb_data->largeSectorCount);
	index_c++;
	*index = index_c * VGA_WIDTH;
	
	return;

}

void print_MBR_PARITION_DATA(uint16_t* const vga_buffer, size_t* const index, const MBR_PARTITION_DATA* const mbr_partition_data) {

	size_t index_c = (*index) / VGA_WIDTH;

	print_string(vga_buffer, index, "MBR_PARTITION_DATA bootFlag: ", strlen("MBR_PARTITION_DATA bootFlag: "));
	print_byte(vga_buffer, index, mbr_partition_data->bootFlag);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "MBR_PARTITION_DATA partitionType: ", strlen("MBR_PARTITION_DATA partitionType: "));
	print_byte(vga_buffer, index, mbr_partition_data->partitionType);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "MBR_PARTITION_DATA lbaPartitionStart: ", strlen("MBR_PARTITION_DATA lbaPartitionStart: "));
	print_dword(vga_buffer, index, mbr_partition_data->lbaPartitionStart);
	index_c++;
	*index = index_c * VGA_WIDTH;

	print_string(vga_buffer, index, "MBR_PARTITION_DATA sectorsInPartition: ", strlen("MBR_PARTITION_DATA sectorsInPartition: "));
	print_dword(vga_buffer, index, mbr_partition_data->sectorsInPartition);
	index_c++;
	*index = index_c * VGA_WIDTH;

	return;

}