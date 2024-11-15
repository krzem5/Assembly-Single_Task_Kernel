#include <common/compressor/compressor.h>
#include <common/kernel/kernel.h>
#include <common/kfs2/api.h>
#include <common/types.h>
#include <uefi/efi/block_io.h>
#include <uefi/efi/error.h>
#include <uefi/efi/guid.h>
#include <uefi/efi/system_table.h>
#include <uefi/efi/tcg2.h>
#include <uefi/relocator.h>



#define KERNEL_LOAD_ADDRESS 0x100000

#define STACK_PAGE_COUNT 3

#define VIRTUAL_IDENTITY_MAP_OFFSET 0xffff800000000000ull

#define PAGE_SIZE 4096
#define PAGE_SIZE_SHIFT 12



static efi_guid_t efi_block_io_protocol_uuid=EFI_BLOCK_IO_PROTOCOL_GUID;
static efi_guid_t efi_acpi_20_table_uuid={0x8868e871,0xe4f1,0x11d3,{0xbc,0x22,0x0,0x80,0xc7,0x3c,0x88,0x81}};
static efi_guid_t efi_smbios_table_uuid={0xeb9d2d31,0x2d88,0x11d3,{0x9a,0x16,0x0,0x90,0x27,0x3f,0xc1,0x4d}};

efi_system_table_t* uefi_global_system_table=NULL;



static bool _equal_guid(efi_guid_t* a,efi_guid_t* b){
	if (a->Data1!=b->Data1||a->Data2!=b->Data2||a->Data3!=b->Data3){
		return 0;
	}
	for (u32 i=0;i<8;i++){
		if (a->Data4[i]!=b->Data4[i]){
			return 0;
		}
	}
	return 1;
}



static u64 _decompress_data(const u8* data,u32 data_length,u64 address){
	u32 out_length=*((const u32*)data);
	data+=sizeof(u32);
	data_length-=sizeof(u32);
	if (EFI_IS_ERROR(uefi_global_system_table->BootServices->AllocatePages(AllocateAddress,0x80000000,(out_length+PAGE_SIZE)>>PAGE_SIZE_SHIFT,(efi_physical_address_t*)(&address)))){
		return 0;
	}
	compressor_decompress(data,data_length,(void*)address);
	return address+((out_length+PAGE_SIZE)&(-PAGE_SIZE));
}



static efi_tcg2_protocol_t* _get_tpm2_handle(void){
	efi_guid_t efi_tpm2_uuid=EFI_TCG2_PROTOCOL_GUID;
	u64 buffer_size=0;
	uefi_global_system_table->BootServices->LocateHandle(ByProtocol,&efi_tpm2_uuid,NULL,&buffer_size,NULL);
	if (!buffer_size){
		return NULL;
	}
	void** buffer=NULL;
	uefi_global_system_table->BootServices->AllocatePool(0x80000000,buffer_size,(void**)(&buffer));
	uefi_global_system_table->BootServices->LocateHandle(ByProtocol,&efi_tpm2_uuid,NULL,&buffer_size,buffer);
	efi_tcg2_protocol_t* out;
	bool is_error=EFI_IS_ERROR(uefi_global_system_table->BootServices->HandleProtocol(buffer[0],&efi_tpm2_uuid,(void**)(&out)));
	uefi_global_system_table->BootServices->FreePool(buffer);
	if (is_error){
		return NULL;
	}
	efi_tcg2_boot_service_capability_t capability;
	capability.Size=sizeof(efi_tcg2_boot_service_capability_t);
	if (EFI_IS_ERROR(out->GetCapability(out,&capability))||!capability.TPMPresentFlag){
		return NULL;
	}
	return out;
}



static void _extend_tpm2_event(u64 data,u64 data_end){
	efi_tcg2_protocol_t* tcg2=_get_tpm2_handle();
	if (!tcg2){
		return;
	}
	efi_tcg2_event_t* tcg2_event;
	uefi_global_system_table->BootServices->AllocatePool(0x80000000,sizeof(efi_tcg2_event_t),(void**)(&tcg2_event));
	uefi_global_system_table->BootServices->SetMem(tcg2_event,sizeof(efi_tcg2_event_t),0);
	tcg2_event->Size=sizeof(efi_tcg2_event_t);
	tcg2_event->Header.HeaderSize=sizeof(efi_tcg2_event_header_t);
	tcg2_event->Header.HeaderVersion=EFI_TCG2_EVENT_HEADER_VERSION;
	tcg2_event->Header.PCRIndex=8;
	tcg2_event->Header.EventType=13;
	tcg2->HashLogExtendEvent(tcg2,0,data,data_end-data,tcg2_event);
	uefi_global_system_table->BootServices->FreePool(tcg2_event);
}



static u64 _kfs2_decompress_node(kfs2_filesystem_t* fs,kfs2_node_t* node,u64 address){
	if (!node->size||(node->flags&KFS2_INODE_TYPE_MASK)!=KFS2_INODE_TYPE_FILE){
		return 0;
	}
	u64 buffer_page_count=(node->size+PAGE_SIZE-1)>>PAGE_SIZE_SHIFT;
	void* buffer=NULL;
	if (EFI_IS_ERROR(uefi_global_system_table->BootServices->AllocatePages(AllocateAnyPages,0x80000000,buffer_page_count,(efi_physical_address_t*)(&buffer)))){
		return 0;
	}
	if (kfs2_node_read(fs,node,0,buffer,node->size)!=node->size){
		goto _cleanup;
	}
	u64 out=_decompress_data((const void*)buffer,node->size,address);
	uefi_global_system_table->BootServices->FreePages((u64)buffer,buffer_page_count);
	_extend_tpm2_event(address,out);
	return out;
_cleanup:
	uefi_global_system_table->BootServices->FreePages((u64)buffer,buffer_page_count);
	return 0;
}



static bool _kfs2_lookup_path(kfs2_filesystem_t* fs,const char* path,kfs2_node_t* out){
	kfs2_filesystem_get_root(fs,out);
	while (path[0]){
		if (path[0]=='/'){
			path++;
			continue;
		}
		u64 i=0;
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



static void* _alloc_callback(u64 count){
	void* out=NULL;
	if (EFI_IS_ERROR(uefi_global_system_table->BootServices->AllocatePages(AllocateAnyPages,0x80000000,count,(efi_physical_address_t*)(&out)))){
		return 0;
	}
	return out;
}



static void _dealloc_callback(void* ptr,u64 count){
	uefi_global_system_table->BootServices->FreePages((u64)ptr,count);
}



static u64 _read_callback(void* ctx,u64 offset,void* buffer,u64 count){
	efi_block_io_protocol_t* block_io_protocol=ctx;
	uefi_global_system_table->ConOut->OutputString(uefi_global_system_table->ConOut,L"\r\n");
	return (EFI_IS_ERROR(block_io_protocol->ReadBlocks(block_io_protocol,block_io_protocol->Media->MediaId,offset,count*block_io_protocol->Media->BlockSize,buffer))?0:count);
}



static u64 _write_callback(void* ctx,u64 offset,const void* buffer,u64 count){
	efi_block_io_protocol_t* block_io_protocol=ctx;
	uefi_global_system_table->ConOut->OutputString(uefi_global_system_table->ConOut,L"\r\n");
	return (EFI_IS_ERROR(block_io_protocol->WriteBlocks(block_io_protocol,block_io_protocol->Media->MediaId,offset,count*block_io_protocol->Media->BlockSize,(void*)buffer))?0:count);
}



efi_status_t efi_main(void* image,efi_system_table_t* system_table){
	relocate_executable();
	uefi_global_system_table=system_table;
	system_table->ConOut->Reset(system_table->ConOut,0);
	u64 buffer_size=0;
	system_table->BootServices->LocateHandle(ByProtocol,&efi_block_io_protocol_uuid,NULL,&buffer_size,NULL);
	void** buffer;
	system_table->BootServices->AllocatePool(0x80000000,buffer_size,(void**)(&buffer));
	system_table->BootServices->LocateHandle(ByProtocol,&efi_block_io_protocol_uuid,NULL,&buffer_size,buffer);
	u64 first_free_address=0;
	u64 initramfs_address=0;
	u64 initramfs_size=0;
	u8 boot_fs_uuid[16];
	u8 master_key[64];
	for (u64 i=0;i<buffer_size/sizeof(void*);i++){
		efi_block_io_protocol_t* block_io_protocol;
		if (EFI_IS_ERROR(system_table->BootServices->HandleProtocol(buffer[i],&efi_block_io_protocol_uuid,(void**)(&block_io_protocol)))||!block_io_protocol->Media->LastBlock||block_io_protocol->Media->BlockSize&(block_io_protocol->Media->BlockSize-1)){
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
		if (!_kfs2_lookup_path(&fs,"/boot/kernel",&kernel_kfs2_node)||!_kfs2_lookup_path(&fs,"/boot/initramfs",&initramfs_kfs2_node)){
			kfs2_filesystem_deinit(&fs);
			continue;
		}
		first_free_address=_kfs2_decompress_node(&fs,&kernel_kfs2_node,KERNEL_LOAD_ADDRESS);
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
		for (u32 j=0;j<16;j++){
			boot_fs_uuid[j]=fs.root_block.uuid[j];
		}
		for (u32 j=0;j<64;j++){
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
	if (EFI_IS_ERROR(system_table->BootServices->AllocatePages(AllocateAddress,0x80000000,1,(efi_physical_address_t*)(&kernel_data)))){
		kernel_data=NULL;
		return EFI_SUCCESS;
	}
	first_free_address+=PAGE_SIZE;
	efi_physical_address_t kernel_stack=first_free_address;
	if (EFI_IS_ERROR(system_table->BootServices->AllocatePages(AllocateAddress,0x80000000,STACK_PAGE_COUNT,&kernel_stack))){
		return EFI_SUCCESS;
	}
	first_free_address+=STACK_PAGE_COUNT<<PAGE_SIZE_SHIFT;
	efi_physical_address_t kernel_pagemap=first_free_address;
	if (EFI_IS_ERROR(system_table->BootServices->AllocatePages(AllocateAddress,0x80000000,4,&kernel_pagemap))){
		return EFI_SUCCESS;
	}
	first_free_address+=4*PAGE_SIZE;
	*((u64*)(kernel_pagemap+0x0000))=0x00000003|(kernel_pagemap+0x1000);
	*((u64*)(kernel_pagemap+0x0800))=0x00000003|(kernel_pagemap+0x2000);
	*((u64*)(kernel_pagemap+0x0ff8))=0x00000003|(kernel_pagemap+0x3000);
	*((u64*)(kernel_pagemap+0x1000))=0x00000083;
	*((u64*)(kernel_pagemap+0x2000))=0x00000083;
	*((u64*)(kernel_pagemap+0x3ff8))=0x00000083;
	kernel_data->mmap_size=0;
	kernel_data->first_free_address=first_free_address;
	kernel_data->rsdp_address=0;
	kernel_data->smbios_address=0;
	for (u64 i=0;i<system_table->NumberOfTableEntries;i++){
		if (!kernel_data->rsdp_address&&_equal_guid(&(system_table->ConfigurationTable+i)->VendorGuid,&efi_acpi_20_table_uuid)){
			kernel_data->rsdp_address=(u64)((system_table->ConfigurationTable+i)->VendorTable);
		}
		else if (!kernel_data->smbios_address&&_equal_guid(&(system_table->ConfigurationTable+i)->VendorGuid,&efi_smbios_table_uuid)){
			kernel_data->smbios_address=(u64)((system_table->ConfigurationTable+i)->VendorTable);
		}
	}
	kernel_data->initramfs_address=initramfs_address;
	kernel_data->initramfs_size=initramfs_size;
	for (u32 i=0;i<16;i++){
		kernel_data->boot_fs_uuid[i]=boot_fs_uuid[i];
	}
	for (u32 i=0;i<64;i++){
		kernel_data->master_key[i]=master_key[i];
	}
	EFI_TIME time;
	if (EFI_IS_ERROR(system_table->RuntimeServices->GetTime(&time,NULL))){
		return EFI_SUCCESS;
	}
	u32 measurement_offset_low;
	u32 measurement_offset_high;
	asm volatile("rdtsc":"=a"(measurement_offset_low),"=d"(measurement_offset_high));
	kernel_data->date.year=time.Year;
	kernel_data->date.month=time.Month;
	kernel_data->date.day=time.Day;
	kernel_data->date.hour=time.Hour;
	kernel_data->date.minute=time.Minute;
	kernel_data->date.second=time.Second;
	kernel_data->date.nanosecond=time.Nanosecond;
	kernel_data->date.measurement_offset=(((u64)measurement_offset_high)<<32)|measurement_offset_low;
	u64 memory_map_size=0;
	void* memory_map=NULL;
	u64 memory_map_key=0;
	u64 memory_descriptor_size=0;
	u32 memory_descriptor_version=0;
	while (system_table->BootServices->GetMemoryMap(&memory_map_size,memory_map,&memory_map_key,&memory_descriptor_size,&memory_descriptor_version)==EFI_BUFFER_TOO_SMALL){
		memory_map_size+=memory_descriptor_size<<4;
		if (memory_map){
			system_table->BootServices->FreePool(memory_map);
		}
		system_table->BootServices->AllocatePool(0x80000000,memory_map_size,&memory_map);
	}
	for (u64 i=0;i<memory_map_size;i+=memory_descriptor_size){
		efi_memory_descriptor_t* entry=(efi_memory_descriptor_t*)(memory_map+i);
		entry->VirtualStart=entry->PhysicalStart+VIRTUAL_IDENTITY_MAP_OFFSET;
		if (entry->Type!=EfiLoaderCode&&entry->Type!=EfiLoaderData&&entry->Type!=EfiBootServicesCode&&entry->Type!=EfiBootServicesData&&entry->Type!=EfiConventionalMemory){
			continue;
		}
		u64 entry_start=entry->PhysicalStart;
		u64 entry_end=entry->PhysicalStart+(entry->NumberOfPages<<PAGE_SIZE_SHIFT);
		if (entry_start<KERNEL_LOAD_ADDRESS){
			entry_start=KERNEL_LOAD_ADDRESS;
		}
		if (entry_start>=entry_end){
			continue;
		}
		for (u32 j=0;j<kernel_data->mmap_size;j++){
			u64 end=kernel_data->mmap[j].base+kernel_data->mmap[j].length;
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
	((void (*)(void*,void*,void*))KERNEL_LOAD_ADDRESS)(kernel_data,(void*)kernel_pagemap,(void*)(kernel_stack+(STACK_PAGE_COUNT<<PAGE_SIZE_SHIFT)));
	for (;;);
}
