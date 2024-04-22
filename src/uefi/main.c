#include <common/kfs2/api.h>
#include <common/kfs2/structures.h>
#include <efi.h>
#include <uefi/compression.h>
#include <uefi/kernel_data.h>
#include <uefi/relocator.h>
#include <uefi/tpm2.h>



#define VMM_HIGHER_HALF_ADDRESS_OFFSET 0xffff800000000000ull

#define PAGE_SIZE 4096
#define PAGE_SIZE_SHIFT 12



static EFI_GUID efi_block_io_protocol_guid=EFI_BLOCK_IO_PROTOCOL_GUID;
static EFI_GUID efi_acpi_20_table_guid=ACPI_20_TABLE_GUID;
static EFI_GUID efi_smbios_table_guid=SMBIOS_TABLE_GUID;

EFI_SYSTEM_TABLE* uefi_global_system_table=NULL;



static _Bool _equal_guid(EFI_GUID* a,EFI_GUID* b){
	if (a->Data1!=b->Data1||a->Data2!=b->Data2||a->Data3!=b->Data3){
		return 0;
	}
	for (uint8_t i=0;i<8;i++){
		if (a->Data4[i]!=b->Data4[i]){
			return 0;
		}
	}
	return 1;
}



static uint64_t _decompress_data(const uint8_t* data,uint32_t data_length,uint64_t address){
	uint32_t out_length=*((const uint32_t*)data);
	data+=sizeof(uint32_t);
	data_length-=sizeof(uint32_t);
	if (EFI_ERROR(uefi_global_system_table->BootServices->AllocatePages(AllocateAddress,0x80000000,(out_length+PAGE_SIZE)>>PAGE_SIZE_SHIFT,&address))){
		return 0;
	}
	_decompress_raw(data,data_length,(uint8_t*)address);
	return address+((out_length+PAGE_SIZE)&(-PAGE_SIZE));
}



static void _extend_tpm2_event(uint64_t data,uint64_t data_end){
	EFI_GUID efi_tpm2_guid=EFI_TCG2_PROTOCOL_GUID;
	UINTN buffer_size=0;
	uefi_global_system_table->BootServices->LocateHandle(ByProtocol,&efi_tpm2_guid,NULL,&buffer_size,NULL);
	if (!buffer_size){
		return;
	}
	EFI_HANDLE* buffer;
	uefi_global_system_table->BootServices->AllocatePool(0x80000000,buffer_size,(void**)(&buffer));
	uefi_global_system_table->BootServices->LocateHandle(ByProtocol,&efi_tpm2_guid,NULL,&buffer_size,buffer);
	EFI_TCG2* tcg2;
	_Bool is_error=EFI_ERROR(uefi_global_system_table->BootServices->HandleProtocol(buffer[0],&efi_tpm2_guid,(void**)(&tcg2)));
	uefi_global_system_table->BootServices->FreePool(buffer);
	if (is_error){
		return;
	}
	EFI_TCG2_BOOT_SERVICE_CAPABILITY capability;
	capability.Size=sizeof(EFI_TCG2_BOOT_SERVICE_CAPABILITY);
	if (EFI_ERROR(tcg2->GetCapability(tcg2,&capability))||!capability.TPMPresentFlag){
		return;
	}
	EFI_TCG2_EVENT* tcg2_event;
	uefi_global_system_table->BootServices->AllocatePool(0x80000000,sizeof(EFI_TCG2_EVENT),(void**)(&tcg2_event));
	uefi_global_system_table->BootServices->SetMem(tcg2_event,sizeof(EFI_TCG2_EVENT),0);
	tcg2_event->Size=sizeof(EFI_TCG2_EVENT);
	tcg2_event->Header.HeaderSize=sizeof(EFI_TCG2_EVENT_HEADER);
	tcg2_event->Header.HeaderVersion=EFI_TCG2_EVENT_HEADER_VERSION;
	tcg2_event->Header.PCRIndex=8;
	tcg2_event->Header.EventType=13;
	tcg2->HashLogExtendEvent(tcg2,0,data,data_end-data,tcg2_event);
	uefi_global_system_table->BootServices->FreePool(tcg2_event);
}



static uint64_t _kfs2_decompress_node(kfs2_filesystem_t* fs,kfs2_node_t* node,uint64_t address){
	if (!node->size||(node->flags&KFS2_INODE_TYPE_MASK)!=KFS2_INODE_TYPE_FILE){
		return 0;
	}
	uint64_t buffer_page_count=(node->size+PAGE_SIZE-1)>>PAGE_SIZE_SHIFT;
	void* buffer=NULL;
	if (EFI_ERROR(uefi_global_system_table->BootServices->AllocatePages(AllocateAnyPages,0x80000000,buffer_page_count,(EFI_PHYSICAL_ADDRESS*)(&buffer)))){
		return 0;
	}
	if (kfs2_node_read(fs,node,0,buffer,node->size)!=node->size){
		goto _cleanup;
	}
	uint64_t out=_decompress_data((const uint8_t*)buffer,node->size,address);
	uefi_global_system_table->BootServices->FreePages((uint64_t)buffer,buffer_page_count);
	_extend_tpm2_event(address,out);
	return out;
_cleanup:
	uefi_global_system_table->BootServices->FreePages((uint64_t)buffer,buffer_page_count);
	return 0;
}



static _Bool _kfs2_lookup_path(kfs2_filesystem_t* fs,const char* path,kfs2_node_t* out){
	kfs2_filesystem_get_root(fs,out);
	while (path[0]){
		if (path[0]=='/'){
			path++;
			continue;
		}
		uint64_t i=0;
		for (;path[i]&&path[i]!='/';i++){
			if (i>255){
				return 0;
			}
		}
		if (i==1&&path[0]=='.'){
			path+=1;
			continue;
		}
		if (i==2&&path[0]=='.'&&path[1]=='.'){
			uefi_global_system_table->ConOut->OutputString(uefi_global_system_table->ConOut,L"Backtracking is not supported\r\n");
			return 0;
		}
		kfs2_node_t child;
		if (!kfs2_node_lookup(fs,out,path,i,&child)){
			return 0;
		}
		*out=child;
		path+=i;
	}
	return 1;
}



static void* _alloc_callback(uint64_t count){
	void* out=NULL;
	if (EFI_ERROR(uefi_global_system_table->BootServices->AllocatePages(AllocateAnyPages,0x80000000,count,(EFI_PHYSICAL_ADDRESS*)(&out)))){
		return 0;
	}
	return out;
}



static void _dealloc_callback(void* ptr,uint64_t count){
	uefi_global_system_table->BootServices->FreePages((uint64_t)ptr,count);
}



static uint64_t _read_callback(void* ctx,uint64_t offset,void* buffer,uint64_t count){
	EFI_BLOCK_IO_PROTOCOL* block_io_protocol=ctx;
	uefi_global_system_table->ConOut->OutputString(uefi_global_system_table->ConOut,L"\r\n");
	return (EFI_ERROR(block_io_protocol->ReadBlocks(block_io_protocol,block_io_protocol->Media->MediaId,offset,count*block_io_protocol->Media->BlockSize,buffer))?0:count);
}



static uint64_t _write_callback(void* ctx,uint64_t offset,const void* buffer,uint64_t count){
	return 0;
}



EFI_STATUS efi_main(EFI_HANDLE image,EFI_SYSTEM_TABLE* system_table){
	relocate_executable();
	uefi_global_system_table=system_table;
	system_table->ConOut->Reset(system_table->ConOut,0);
	UINTN buffer_size=0;
	system_table->BootServices->LocateHandle(ByProtocol,&efi_block_io_protocol_guid,NULL,&buffer_size,NULL);
	EFI_HANDLE* buffer;
	system_table->BootServices->AllocatePool(0x80000000,buffer_size,(void**)(&buffer));
	system_table->BootServices->LocateHandle(ByProtocol,&efi_block_io_protocol_guid,NULL,&buffer_size,buffer);
	uint64_t first_free_address=0;
	uint64_t initramfs_address=0;
	uint64_t initramfs_size=0;
	uint8_t boot_fs_guid[16];
	uint8_t master_key[64];
	for (UINTN i=0;i<buffer_size/sizeof(EFI_HANDLE);i++){
		EFI_BLOCK_IO_PROTOCOL* block_io_protocol;
		if (EFI_ERROR(system_table->BootServices->HandleProtocol(buffer[i],&efi_block_io_protocol_guid,(void**)(&block_io_protocol)))||!block_io_protocol->Media->LastBlock||block_io_protocol->Media->BlockSize&(block_io_protocol->Media->BlockSize-1)){
			continue;
		}
		const kfs2_filesystem_config_t fs_config={
			block_io_protocol,
			(kfs2_filesystem_block_read_callback_t)_read_callback,
			(kfs2_filesystem_block_write_callback_t)_write_callback,
			(kfs2_filesystem_page_alloc_callback_t)_alloc_callback,
			(kfs2_filesystem_page_dealloc_callback_t)_dealloc_callback,
			block_io_protocol->Media->BlockSize,
			0,
			block_io_protocol->Media->LastBlock
		};
		kfs2_filesystem_t fs;
		if (!kfs2_filesystem_init(&fs_config,&fs)){
			continue;
		}
		kfs2_node_t kernel_kfs2_node;
		kfs2_node_t initramfs_kfs2_node;
		if (!_kfs2_lookup_path(&fs,"/boot/kernel.compressed",&kernel_kfs2_node)||!_kfs2_lookup_path(&fs,"/boot/initramfs.compressed",&initramfs_kfs2_node)){
			kfs2_filesystem_deinit(&fs);
			continue;
		}
		first_free_address=_kfs2_decompress_node(&fs,&kernel_kfs2_node,KERNEL_MEMORY_ADDRESS);
		if (!first_free_address){
			kfs2_filesystem_deinit(&fs);
			continue;
		}
		initramfs_address=first_free_address;
		first_free_address=_kfs2_decompress_node(&fs,&initramfs_kfs2_node,first_free_address);
		if (!first_free_address){
			kfs2_filesystem_deinit(&fs);
			continue;
		}
		initramfs_size=first_free_address-initramfs_address;
		for (uint8_t j=0;j<16;j++){
			boot_fs_guid[j]=fs.root_block.uuid[j];
		}
		for (uint8_t j=0;j<64;j++){
			master_key[j]=fs.root_block.master_key[j];
		}
		kfs2_filesystem_deinit(&fs);
		break;
	}
	system_table->BootServices->FreePool(buffer);
	if (!first_free_address){
		return EFI_SUCCESS;
	}
	kernel_data_t* kernel_data=(void*)first_free_address;
	if (EFI_ERROR(system_table->BootServices->AllocatePages(AllocateAddress,0x80000000,1,(EFI_PHYSICAL_ADDRESS*)(&kernel_data)))){
		kernel_data=NULL;
		return EFI_SUCCESS;
	}
	first_free_address+=PAGE_SIZE;
	EFI_PHYSICAL_ADDRESS kernel_stack=first_free_address;
	if (EFI_ERROR(system_table->BootServices->AllocatePages(AllocateAddress,0x80000000,KERNEL_STACK_PAGE_COUNT,&kernel_stack))){
		return EFI_SUCCESS;
	}
	first_free_address+=KERNEL_STACK_PAGE_COUNT<<PAGE_SIZE_SHIFT;
	EFI_PHYSICAL_ADDRESS kernel_pagemap=first_free_address;
	if (EFI_ERROR(system_table->BootServices->AllocatePages(AllocateAddress,0x80000000,4,&kernel_pagemap))){
		return EFI_SUCCESS;
	}
	first_free_address+=4*PAGE_SIZE;
	*((uint64_t*)(kernel_pagemap+0x0000))=0x00000003|(kernel_pagemap+0x1000);
	*((uint64_t*)(kernel_pagemap+0x0800))=0x00000003|(kernel_pagemap+0x2000);
	*((uint64_t*)(kernel_pagemap+0x0ff8))=0x00000003|(kernel_pagemap+0x3000);
	*((uint64_t*)(kernel_pagemap+0x1000))=0x00000083;
	*((uint64_t*)(kernel_pagemap+0x2000))=0x00000083;
	*((uint64_t*)(kernel_pagemap+0x3ff8))=0x00000083;
	kernel_data->mmap_size=0;
	kernel_data->first_free_address=first_free_address;
	kernel_data->rsdp_address=0;
	kernel_data->smbios_address=0;
	for (uint64_t i=0;i<system_table->NumberOfTableEntries;i++){
		if (!kernel_data->rsdp_address&&_equal_guid(&(system_table->ConfigurationTable+i)->VendorGuid,&efi_acpi_20_table_guid)){
			kernel_data->rsdp_address=(uint64_t)((system_table->ConfigurationTable+i)->VendorTable);
		}
		else if (!kernel_data->smbios_address&&_equal_guid(&(system_table->ConfigurationTable+i)->VendorGuid,&efi_smbios_table_guid)){
			kernel_data->smbios_address=(uint64_t)((system_table->ConfigurationTable+i)->VendorTable);
		}
	}
	kernel_data->initramfs_address=initramfs_address;
	kernel_data->initramfs_size=initramfs_size;
	for (uint8_t i=0;i<16;i++){
		kernel_data->boot_fs_guid[i]=boot_fs_guid[i];
	}
	for (uint8_t i=0;i<64;i++){
		kernel_data->master_key[i]=master_key[i];
	}
	EFI_TIME time;
	if (EFI_ERROR(system_table->RuntimeServices->GetTime(&time,NULL))){
		return EFI_SUCCESS;
	}
	uint32_t measurement_offset_low;
	uint32_t measurement_offset_high;
	asm volatile("rdtsc":"=a"(measurement_offset_low),"=d"(measurement_offset_high));
	kernel_data->date.year=time.Year;
	kernel_data->date.month=time.Month;
	kernel_data->date.day=time.Day;
	kernel_data->date.hour=time.Hour;
	kernel_data->date.minute=time.Minute;
	kernel_data->date.second=time.Second;
	kernel_data->date.nanosecond=time.Nanosecond;
	kernel_data->date.measurement_offset=(((uint64_t)measurement_offset_high)<<32)|measurement_offset_low;
	UINTN memory_map_size=0;
	void* memory_map=NULL;
	UINTN memory_map_key=0;
	UINTN memory_descriptor_size=0;
	UINT32 memory_descriptor_version=0;
	while (system_table->BootServices->GetMemoryMap(&memory_map_size,memory_map,&memory_map_key,&memory_descriptor_size,&memory_descriptor_version)==EFI_BUFFER_TOO_SMALL){
		memory_map_size+=memory_descriptor_size<<4;
		if (memory_map){
			system_table->BootServices->FreePool(memory_map);
		}
		system_table->BootServices->AllocatePool(0x80000000,memory_map_size,&memory_map);
	}
	for (UINTN i=0;i<memory_map_size;i+=memory_descriptor_size){
		EFI_MEMORY_DESCRIPTOR* entry=(EFI_MEMORY_DESCRIPTOR*)(memory_map+i);
		entry->VirtualStart=entry->PhysicalStart+VMM_HIGHER_HALF_ADDRESS_OFFSET;
		if (entry->Type!=EfiLoaderCode&&entry->Type!=EfiLoaderData&&entry->Type!=EfiBootServicesCode&&entry->Type!=EfiBootServicesData&&entry->Type!=EfiConventionalMemory){
			continue;
		}
		uint64_t entry_start=entry->PhysicalStart;
		uint64_t entry_end=entry->PhysicalStart+(entry->NumberOfPages<<PAGE_SIZE_SHIFT);
		if (entry_start<KERNEL_MEMORY_ADDRESS){
			entry_start=KERNEL_MEMORY_ADDRESS;
		}
		if (entry_start>=entry_end){
			continue;
		}
		for (uint16_t j=0;j<kernel_data->mmap_size;j++){
			uint64_t end=kernel_data->mmap[j].base+kernel_data->mmap[j].length;
			if (end<entry_start||entry_end<kernel_data->mmap[j].base){
				continue;
			}
			if (entry_end>end){
				end=entry_end;
			}
			if (kernel_data->mmap[j].base>entry_start){
				kernel_data->mmap[j].base=entry_start;
			}
			kernel_data->mmap[j].length=end-kernel_data->mmap[j].base;
			goto _entry_added;
		}
		kernel_data->mmap[kernel_data->mmap_size].base=entry_start;
		kernel_data->mmap[kernel_data->mmap_size].length=entry_end-entry_start;
		kernel_data->mmap_size++;
_entry_added:
	}
	while (system_table->BootServices->ExitBootServices(image,memory_map_key)==EFI_INVALID_PARAMETER){
		system_table->BootServices->GetMemoryMap(&memory_map_size,memory_map,&memory_map_key,&memory_descriptor_size,&memory_descriptor_version);
	}
	system_table->RuntimeServices->SetVirtualAddressMap(memory_map_size,memory_descriptor_size,memory_descriptor_version,memory_map);
	((void (*)(void*,void*,void*))KERNEL_MEMORY_ADDRESS)(kernel_data,(void*)kernel_pagemap,(void*)(kernel_stack+(KERNEL_STACK_PAGE_COUNT<<PAGE_SIZE_SHIFT)));
	for (;;);
}
