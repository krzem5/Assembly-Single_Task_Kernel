#include <common/aes/aes.h>
#include <common/compressor/compressor.h>
#include <common/hash/sha256.h>
#include <common/kernel/kernel_data.h>
#include <common/kfs2/api.h>
#include <common/kfs2/structures.h>
#include <common/tpm/commands.h>
#include <common/types.h>
#include <common/update/update.h>
#include <efi.h>
#include <uefi/relocator.h>
#include <uefi/tpm2.h>



#define KERNEL_LOAD_ADDRESS 0x100000

#define STACK_PAGE_COUNT 3

#define VIRTUAL_IDENTITY_MAP_OFFSET 0xffff800000000000ull

#define PAGE_SIZE 4096
#define PAGE_SIZE_SHIFT 12



typedef struct _TPM_PLATFORM_KEY_STATE{
	hash_sha256_state_t combined_hash;
	u8 last_hash[32];
	u8 new_last_hash[32];
} tpm_platform_key_state_t;



static EFI_GUID efi_block_io_protocol_guid=EFI_BLOCK_IO_PROTOCOL_GUID;
static EFI_GUID efi_acpi_20_table_guid=ACPI_20_TABLE_GUID;
static EFI_GUID efi_smbios_table_guid=SMBIOS_TABLE_GUID;

EFI_SYSTEM_TABLE* uefi_global_system_table=NULL;



static bool _equal_guid(EFI_GUID* a,EFI_GUID* b){
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
	if (EFI_ERROR(uefi_global_system_table->BootServices->AllocatePages(AllocateAddress,0x80000000,(out_length+PAGE_SIZE)>>PAGE_SIZE_SHIFT,(EFI_PHYSICAL_ADDRESS*)(&address)))){
		return 0;
	}
	compressor_decompress(data,data_length,(void*)address);
	return address+((out_length+PAGE_SIZE)&(-PAGE_SIZE));
}



static EFI_TCG2* _get_tpm2_handle(void){
	EFI_GUID efi_tpm2_guid=EFI_TCG2_PROTOCOL_GUID;
	UINTN buffer_size=0;
	uefi_global_system_table->BootServices->LocateHandle(ByProtocol,&efi_tpm2_guid,NULL,&buffer_size,NULL);
	if (!buffer_size){
		return NULL;
	}
	EFI_HANDLE* buffer;
	uefi_global_system_table->BootServices->AllocatePool(0x80000000,buffer_size,(void**)(&buffer));
	uefi_global_system_table->BootServices->LocateHandle(ByProtocol,&efi_tpm2_guid,NULL,&buffer_size,buffer);
	EFI_TCG2* out;
	bool is_error=EFI_ERROR(uefi_global_system_table->BootServices->HandleProtocol(buffer[0],&efi_tpm2_guid,(void**)(&out)));
	uefi_global_system_table->BootServices->FreePool(buffer);
	if (is_error){
		return NULL;
	}
	EFI_TCG2_BOOT_SERVICE_CAPABILITY capability;
	capability.Size=sizeof(EFI_TCG2_BOOT_SERVICE_CAPABILITY);
	if (EFI_ERROR(out->GetCapability(out,&capability))||!capability.TPMPresentFlag){
		return NULL;
	}
	return out;
}



static void _extend_tpm2_event(u64 data,u64 data_end){
	EFI_TCG2* tcg2=_get_tpm2_handle();
	if (!tcg2){
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



static u64 _kfs2_decompress_node(kfs2_filesystem_t* fs,kfs2_node_t* node,u64 address,tpm_platform_key_state_t* platform_key_state){
	if (!node->size||(node->flags&KFS2_INODE_TYPE_MASK)!=KFS2_INODE_TYPE_FILE){
		return 0;
	}
	u64 buffer_page_count=(node->size+PAGE_SIZE-1)>>PAGE_SIZE_SHIFT;
	void* buffer=NULL;
	if (EFI_ERROR(uefi_global_system_table->BootServices->AllocatePages(AllocateAnyPages,0x80000000,buffer_page_count,(EFI_PHYSICAL_ADDRESS*)(&buffer)))){
		return 0;
	}
	if (kfs2_node_read(fs,node,0,buffer,node->size)!=node->size){
		goto _cleanup;
	}
	u64 out=_decompress_data((const void*)buffer,node->size,address);
	uefi_global_system_table->BootServices->FreePages((u64)buffer,buffer_page_count);
	_extend_tpm2_event(address,out);
	hash_sha256_state_t hash_state;
	hash_sha256_init(&hash_state);
	hash_sha256_process_chunk(&hash_state,(void*)address,out-address);
	hash_sha256_finalize(&hash_state);
	hash_sha256_state_t last_pcr_hash_state;
	hash_sha256_init(&last_pcr_hash_state);
	hash_sha256_process_chunk(&last_pcr_hash_state,platform_key_state->last_hash,32);
	hash_sha256_process_chunk(&last_pcr_hash_state,hash_state.result,32);
	hash_sha256_finalize(&last_pcr_hash_state);
	for (u32 i=0;i<32;i++){
		platform_key_state->last_hash[i]=last_pcr_hash_state.result[i];
	}
	uefi_global_system_table->BootServices->SetMem(&hash_state,sizeof(hash_state),0);
	uefi_global_system_table->BootServices->SetMem(&last_pcr_hash_state,sizeof(last_pcr_hash_state),0);
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
	if (EFI_ERROR(uefi_global_system_table->BootServices->AllocatePages(AllocateAnyPages,0x80000000,count,(EFI_PHYSICAL_ADDRESS*)(&out)))){
		return 0;
	}
	return out;
}



static void _dealloc_callback(void* ptr,u64 count){
	uefi_global_system_table->BootServices->FreePages((u64)ptr,count);
}



static u64 _read_callback(void* ctx,u64 offset,void* buffer,u64 count){
	EFI_BLOCK_IO_PROTOCOL* block_io_protocol=ctx;
	uefi_global_system_table->ConOut->OutputString(uefi_global_system_table->ConOut,L"\r\n");
	return (EFI_ERROR(block_io_protocol->ReadBlocks(block_io_protocol,block_io_protocol->Media->MediaId,offset,count*block_io_protocol->Media->BlockSize,buffer))?0:count);
}



static u64 _write_callback(void* ctx,u64 offset,const void* buffer,u64 count){
	EFI_BLOCK_IO_PROTOCOL* block_io_protocol=ctx;
	uefi_global_system_table->ConOut->OutputString(uefi_global_system_table->ConOut,L"\r\n");
	return (EFI_ERROR(block_io_protocol->WriteBlocks(block_io_protocol,block_io_protocol->Media->MediaId,offset,count*block_io_protocol->Media->BlockSize,(void*)buffer))?0:count);
}



static inline char _int_to_hex(uint8_t v){
	v&=15;
	return v+(v>9?87:48);
}



static inline void _output_int_hex(uint64_t value){
	uint16_t buffer[17];
	for (uint8_t i=0;i<16;i++){
		uint8_t j=(value>>((15-i)<<2))&0xf;
		buffer[i]=j+(j>9?87:48);
	}
	buffer[16]=0;
	uefi_global_system_table->ConOut->OutputString(uefi_global_system_table->ConOut,buffer);
}



static void _fetch_platform_key(tpm_platform_key_state_t* out){
	EFI_TCG2* tcg2=_get_tpm2_handle();
	if (!tcg2){
		return;
	}
	hash_sha256_init(&(out->combined_hash));
	tpm_command_t* command;
	uefi_global_system_table->BootServices->AllocatePages(AllocateAnyPages,0x80000000,1,(EFI_PHYSICAL_ADDRESS*)(&command));
	for (u32 pcr_index=TPM_KEY_PCR_MIN;pcr_index<=TPM_KEY_PCR_MAX;pcr_index++){
		command->header.tag=__builtin_bswap16(TPM2_ST_NO_SESSIONS);
		command->header.length=__builtin_bswap32(sizeof(tpm_command_header_t)+sizeof(command->pcr_read));
		command->header.command_code=__builtin_bswap32(TPM2_CC_PCR_READ);
		command->pcr_read.selection_count=__builtin_bswap32(1);
		command->pcr_read.selection_hash_alg=__builtin_bswap16(TPM_KEY_PCR_HASH);
		command->pcr_read.selection_size=sizeof(command->pcr_read.selection_data);
		for (u32 i=0;i<command->pcr_read.selection_size;i++){
			command->pcr_read.selection_data[i]=0;
		}
		command->pcr_read.selection_data[pcr_index>>3]|=1<<(pcr_index&7);
		if (EFI_ERROR(tcg2->SubmitCommand(tcg2,sizeof(tpm_command_header_t)+sizeof(command->pcr_read),(void*)command,PAGE_SIZE,(void*)command))||__builtin_bswap32(command->header.return_code)!=TPM2_RC_SUCCESS||__builtin_bswap32(command->pcr_read_resp.digest_count)!=1||__builtin_bswap16(command->pcr_read_resp.digest_size)!=sizeof(out->last_hash)){
			goto _cleanup;
		}
		if (pcr_index<TPM_KEY_PCR_MAX){
			hash_sha256_process_chunk(&(out->combined_hash),command->pcr_read_resp.data,sizeof(out->last_hash));
		}
		else{
			for (u32 i=0;i<sizeof(out->last_hash);i++){
				out->last_hash[i]=command->pcr_read_resp.data[i];
				out->new_last_hash[i]=command->pcr_read_resp.data[i];
			}
		}
	}
_cleanup:
	uefi_global_system_table->BootServices->SetMem(command,PAGE_SIZE,0);
	uefi_global_system_table->BootServices->FreePages((u64)command,1);
}



static void _generate_new_platform_key(kfs2_filesystem_t* fs,tpm_platform_key_state_t* platform_key_state){
	kfs2_node_t update_ticket_kfs2_node;
	update_ticket_t update_ticket;
	if (!_kfs2_lookup_path(fs,"/boot/update_ticket",&update_ticket_kfs2_node)||update_ticket_kfs2_node.size!=sizeof(update_ticket_t)||kfs2_node_read(fs,&update_ticket_kfs2_node,0,&update_ticket,sizeof(update_ticket_t))!=sizeof(update_ticket_t)){
		goto _cleanup;
	}
	hash_sha256_state_t new_combined_hash=platform_key_state->combined_hash;
	hash_sha256_process_chunk(&(platform_key_state->combined_hash),platform_key_state->last_hash,sizeof(platform_key_state->last_hash));
	hash_sha256_finalize(&(platform_key_state->combined_hash));
	/*
	 * current platform key: platform_key_state->combined_hash.result (256 bits)
	 * new platform key: new_combined_hash.result (needs to be updated with the decrypted hash of new kernel and initramfs) (256 bits)
	 */
	// _output_int_hex(*((const u64*)(platform_key_state->combined_hash.result)));
	// _output_int_hex(*((const u64*)(platform_key_state->combined_hash.result+8)));
	// _output_int_hex(*((const u64*)(platform_key_state->combined_hash.result+16)));
	// _output_int_hex(*((const u64*)(platform_key_state->combined_hash.result+24)));
	// uefi_global_system_table->ConOut->OutputString(uefi_global_system_table->ConOut,L"\r\n");
	// // _fetch_platform_key(platform_key_state);
	// // hash_sha256_process_chunk(&(platform_key_state->combined_hash),platform_key_state->last_hash,sizeof(platform_key_state->last_hash));
	// // hash_sha256_finalize(&(platform_key_state->combined_hash));
	// // _output_int_hex(*((const u64*)(platform_key_state->combined_hash.result)));
	// // _output_int_hex(*((const u64*)(platform_key_state->combined_hash.result+8)));
	// // _output_int_hex(*((const u64*)(platform_key_state->combined_hash.result+16)));
	// // _output_int_hex(*((const u64*)(platform_key_state->combined_hash.result+24)));
	// for (;;);
_cleanup:
	uefi_global_system_table->BootServices->SetMem(&new_combined_hash,sizeof(new_combined_hash),0);
	uefi_global_system_table->BootServices->SetMem(platform_key_state,sizeof(*platform_key_state),0);
	uefi_global_system_table->BootServices->SetMem(&update_ticket,sizeof(update_ticket),0);
	/*
	 * Update sequence:
	 * (kernel)
	 * 1.  [ ] use AES with platform key to encrypt new kernel hash, and new initramfs hash
	 * 2.  [ ] kernel and initramfs are stored in a new file on the drive, alongside their old counterparts
	 * 3.  [ ] store the update ticket on the boot drive
	 * 4.  [ ] restart
	 * (boot loader)
	 * 5.  [x] if no update ticket is found, return to normal boot
	 * 6.  [x] store a copy of the PCR hash before the registers are extended
	 * 7.  [x] extend the PCR registers
	 * 8.  [x] decrypt the encrypted hash
	 * 9.  [ ] if the hmac of decrypted data does not match, return to normal boot
	 * 10. [ ] compute the new PCR values based on new kernel and initramfs hashes
	 * 11. [ ] return to normal boot, and pass new PCR platform key to the old kernel
	 * (kernel)
	 * 12. [ ] decrypt the master key and re-encrypt it with the new platform key
	 * 13. [ ] update files on drive and delete old kernel and old initramfs
	 * 14. [ ] restart
	 */
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
	u64 first_free_address=0;
	u64 initramfs_address=0;
	u64 initramfs_size=0;
	u8 boot_fs_guid[16];
	u8 master_key[64];
	tpm_platform_key_state_t platform_key_state;
	_fetch_platform_key(&platform_key_state);
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
		if (!_kfs2_lookup_path(&fs,"/boot/kernel",&kernel_kfs2_node)||!_kfs2_lookup_path(&fs,"/boot/initramfs",&initramfs_kfs2_node)){
			kfs2_filesystem_deinit(&fs);
			continue;
		}
		first_free_address=_kfs2_decompress_node(&fs,&kernel_kfs2_node,KERNEL_LOAD_ADDRESS,&platform_key_state);
		if (!first_free_address){
			kfs2_filesystem_deinit(&fs);
			continue;
		}
		initramfs_address=first_free_address;
		first_free_address=_kfs2_decompress_node(&fs,&initramfs_kfs2_node,first_free_address,&platform_key_state);
		if (!first_free_address){
			kfs2_filesystem_deinit(&fs);
			continue;
		}
		initramfs_size=first_free_address-initramfs_address;
		_generate_new_platform_key(&fs,&platform_key_state);
		for (u32 j=0;j<16;j++){
			boot_fs_guid[j]=fs.root_block.uuid[j];
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
	if (EFI_ERROR(system_table->BootServices->AllocatePages(AllocateAddress,0x80000000,1,(EFI_PHYSICAL_ADDRESS*)(&kernel_data)))){
		kernel_data=NULL;
		return EFI_SUCCESS;
	}
	first_free_address+=PAGE_SIZE;
	EFI_PHYSICAL_ADDRESS kernel_stack=first_free_address;
	if (EFI_ERROR(system_table->BootServices->AllocatePages(AllocateAddress,0x80000000,STACK_PAGE_COUNT,&kernel_stack))){
		return EFI_SUCCESS;
	}
	first_free_address+=STACK_PAGE_COUNT<<PAGE_SIZE_SHIFT;
	EFI_PHYSICAL_ADDRESS kernel_pagemap=first_free_address;
	if (EFI_ERROR(system_table->BootServices->AllocatePages(AllocateAddress,0x80000000,4,&kernel_pagemap))){
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
		if (!kernel_data->rsdp_address&&_equal_guid(&(system_table->ConfigurationTable+i)->VendorGuid,&efi_acpi_20_table_guid)){
			kernel_data->rsdp_address=(u64)((system_table->ConfigurationTable+i)->VendorTable);
		}
		else if (!kernel_data->smbios_address&&_equal_guid(&(system_table->ConfigurationTable+i)->VendorGuid,&efi_smbios_table_guid)){
			kernel_data->smbios_address=(u64)((system_table->ConfigurationTable+i)->VendorTable);
		}
	}
	kernel_data->initramfs_address=initramfs_address;
	kernel_data->initramfs_size=initramfs_size;
	for (u32 i=0;i<16;i++){
		kernel_data->boot_fs_guid[i]=boot_fs_guid[i];
	}
	for (u32 i=0;i<64;i++){
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
	kernel_data->date.measurement_offset=(((u64)measurement_offset_high)<<32)|measurement_offset_low;
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
