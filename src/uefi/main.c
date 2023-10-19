#include <efi.h>
#include <uefi/relocator.h>



#define KFS2_BLOCK_SIZE 4096

#define KFS2_ROOT_BLOCK_SIGNATURE 0x544f4f523253464b
#define KFS2_BITMAP_LEVEL_COUNT 5

#define KFS2_INODE_GET_BLOCK_INDEX(inode) ((inode)/(KFS2_BLOCK_SIZE/sizeof(kfs2_node_t)))
#define KFS2_INODE_GET_NODE_INDEX(inode) ((inode)%(KFS2_BLOCK_SIZE/sizeof(kfs2_node_t)))

#define KFS2_INODE_TYPE_FILE 0x0000
#define KFS2_INODE_TYPE_DIRECTORY 0x0001
#define KFS2_INODE_TYPE_MASK 0x0001

#define KFS2_INODE_STORAGE_MASK 0x000e
#define KFS2_INODE_STORAGE_TYPE_INLINE 0x0000
#define KFS2_INODE_STORAGE_TYPE_SINGLE 0x0002
#define KFS2_INODE_STORAGE_TYPE_DOUBLE 0x0004
#define KFS2_INODE_STORAGE_TYPE_TRIPLE 0x0006
#define KFS2_INODE_STORAGE_TYPE_QUADRUPLE 0x0008

#define VMM_HIGHER_HALF_ADDRESS_OFFSET 0xffff800000000000ull

#define KERNEL_MEMORY_ADDRESS 0x100000
#define KERNEL_STACK_PAGE_COUNT 3

#define PAGE_SIZE 4096
#define PAGE_SIZE_SHIFT 12



typedef struct __attribute__((packed)) _KFS2_ROOT_BLOCK{
	uint64_t signature;
	uint64_t block_count;
	uint64_t inode_count;
	uint64_t data_block_count;
	uint64_t first_inode_block;
	uint64_t first_data_block;
	uint64_t first_bitmap_block;
	uint64_t inode_allocation_bitmap_offsets[KFS2_BITMAP_LEVEL_COUNT];
	uint64_t data_block_allocation_bitmap_offsets[KFS2_BITMAP_LEVEL_COUNT];
	uint16_t inode_allocation_bitmap_highest_level_length;
	uint16_t data_block_allocation_bitmap_highest_level_length;
	uint32_t kernel_inode;
	uint32_t initramfs_inode;
	uint32_t crc;
} kfs2_root_block_t;



typedef struct __attribute__((packed)) _KFS2_NODE{
	uint64_t size;
	union{
		uint8_t inline_[48];
		uint64_t single[6];
		uint64_t double_;
		uint64_t triple;
		uint64_t quadruple;
	} data;
	uint16_t hard_link_count;
	uint16_t flags;
	uint32_t crc;
} kfs2_node_t;



typedef struct __attribute__((packed)) _KERNEL_DATA{
	uint16_t mmap_size;
	uint8_t _padding[6];
	struct{
		uint64_t base;
		uint64_t length;
		uint32_t type;
		uint8_t _padding[4];
	} mmap[42];
	uint64_t first_free_address;
	uint64_t rsdp_address;
	uint64_t smbios_address;
	uint64_t initramfs_address;
	uint64_t initramfs_size;
} kernel_data_t;



_Static_assert(sizeof(kfs2_node_t)==64);



static const uint32_t _kfs2_crc_table[256]={
	0x00000000,0x77073096,0xee0e612c,0x990951ba,0x076dc419,0x706af48f,0xe963a535,0x9e6495a3,
	0x0edb8832,0x79dcb8a4,0xe0d5e91e,0x97d2d988,0x09b64c2b,0x7eb17cbd,0xe7b82d07,0x90bf1d91,
	0x1db71064,0x6ab020f2,0xf3b97148,0x84be41de,0x1adad47d,0x6ddde4eb,0xf4d4b551,0x83d385c7,
	0x136c9856,0x646ba8c0,0xfd62f97a,0x8a65c9ec,0x14015c4f,0x63066cd9,0xfa0f3d63,0x8d080df5,
	0x3b6e20c8,0x4c69105e,0xd56041e4,0xa2677172,0x3c03e4d1,0x4b04d447,0xd20d85fd,0xa50ab56b,
	0x35b5a8fa,0x42b2986c,0xdbbbc9d6,0xacbcf940,0x32d86ce3,0x45df5c75,0xdcd60dcf,0xabd13d59,
	0x26d930ac,0x51de003a,0xc8d75180,0xbfd06116,0x21b4f4b5,0x56b3c423,0xcfba9599,0xb8bda50f,
	0x2802b89e,0x5f058808,0xc60cd9b2,0xb10be924,0x2f6f7c87,0x58684c11,0xc1611dab,0xb6662d3d,
	0x76dc4190,0x01db7106,0x98d220bc,0xefd5102a,0x71b18589,0x06b6b51f,0x9fbfe4a5,0xe8b8d433,
	0x7807c9a2,0x0f00f934,0x9609a88e,0xe10e9818,0x7f6a0dbb,0x086d3d2d,0x91646c97,0xe6635c01,
	0x6b6b51f4,0x1c6c6162,0x856530d8,0xf262004e,0x6c0695ed,0x1b01a57b,0x8208f4c1,0xf50fc457,
	0x65b0d9c6,0x12b7e950,0x8bbeb8ea,0xfcb9887c,0x62dd1ddf,0x15da2d49,0x8cd37cf3,0xfbd44c65,
	0x4db26158,0x3ab551ce,0xa3bc0074,0xd4bb30e2,0x4adfa541,0x3dd895d7,0xa4d1c46d,0xd3d6f4fb,
	0x4369e96a,0x346ed9fc,0xad678846,0xda60b8d0,0x44042d73,0x33031de5,0xaa0a4c5f,0xdd0d7cc9,
	0x5005713c,0x270241aa,0xbe0b1010,0xc90c2086,0x5768b525,0x206f85b3,0xb966d409,0xce61e49f,
	0x5edef90e,0x29d9c998,0xb0d09822,0xc7d7a8b4,0x59b33d17,0x2eb40d81,0xb7bd5c3b,0xc0ba6cad,
	0xedb88320,0x9abfb3b6,0x03b6e20c,0x74b1d29a,0xead54739,0x9dd277af,0x04db2615,0x73dc1683,
	0xe3630b12,0x94643b84,0x0d6d6a3e,0x7a6a5aa8,0xe40ecf0b,0x9309ff9d,0x0a00ae27,0x7d079eb1,
	0xf00f9344,0x8708a3d2,0x1e01f268,0x6906c2fe,0xf762575d,0x806567cb,0x196c3671,0x6e6b06e7,
	0xfed41b76,0x89d32be0,0x10da7a5a,0x67dd4acc,0xf9b9df6f,0x8ebeeff9,0x17b7be43,0x60b08ed5,
	0xd6d6a3e8,0xa1d1937e,0x38d8c2c4,0x4fdff252,0xd1bb67f1,0xa6bc5767,0x3fb506dd,0x48b2364b,
	0xd80d2bda,0xaf0a1b4c,0x36034af6,0x41047a60,0xdf60efc3,0xa867df55,0x316e8eef,0x4669be79,
	0xcb61b38c,0xbc66831a,0x256fd2a0,0x5268e236,0xcc0c7795,0xbb0b4703,0x220216b9,0x5505262f,
	0xc5ba3bbe,0xb2bd0b28,0x2bb45a92,0x5cb36a04,0xc2d7ffa7,0xb5d0cf31,0x2cd99e8b,0x5bdeae1d,
	0x9b64c2b0,0xec63f226,0x756aa39c,0x026d930a,0x9c0906a9,0xeb0e363f,0x72076785,0x05005713,
	0x95bf4a82,0xe2b87a14,0x7bb12bae,0x0cb61b38,0x92d28e9b,0xe5d5be0d,0x7cdcefb7,0x0bdbdf21,
	0x86d3d2d4,0xf1d4e242,0x68ddb3f8,0x1fda836e,0x81be16cd,0xf6b9265b,0x6fb077e1,0x18b74777,
	0x88085ae6,0xff0f6a70,0x66063bca,0x11010b5c,0x8f659eff,0xf862ae69,0x616bffd3,0x166ccf45,
	0xa00ae278,0xd70dd2ee,0x4e048354,0x3903b3c2,0xa7672661,0xd06016f7,0x4969474d,0x3e6e77db,
	0xaed16a4a,0xd9d65adc,0x40df0b66,0x37d83bf0,0xa9bcae53,0xdebb9ec5,0x47b2cf7f,0x30b5ffe9,
	0xbdbdf21c,0xcabac28a,0x53b39330,0x24b4a3a6,0xbad03605,0xcdd70693,0x54de5729,0x23d967bf,
	0xb3667a2e,0xc4614ab8,0x5d681b02,0x2a6f2b94,0xb40bbe37,0xc30c8ea1,0x5a05df1b,0x2d02ef8d,
};

static EFI_GUID efi_block_io_protocol_guid=EFI_BLOCK_IO_PROTOCOL_GUID;
static EFI_GUID efi_acpi_20_table_guid=ACPI_20_TABLE_GUID;
static EFI_GUID efi_smbios_table_guid=SMBIOS_TABLE_GUID;



static uint32_t _calculate_crc(const void* data,uint32_t length){
	const uint8_t* ptr=data;
	uint32_t out=0xffffffff;
	for (uint32_t i=0;i<length;i++){
		out=_kfs2_crc_table[(out&0xff)^ptr[i]]^(out>>8);
	}
	return ~out;
}



static _Bool kfs_verify_crc(const void* data,uint32_t length){
	return _calculate_crc(data,length-4)==*((uint32_t*)(data+length-4));
}



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



static inline void _output_int(EFI_SYSTEM_TABLE* system_table,uint64_t value){
	uint16_t buffer[21];
	buffer[20]=0;
	uint8_t i=20;
	do{
		i--;
		buffer[i]=(value%10)+48;
		value/=10;
	} while (value);
	system_table->ConOut->OutputString(system_table->ConOut,buffer+i);
}



static inline char _int_to_hex(uint8_t v){
	v&=15;
	return v+(v>9?87:48);
}



static inline void _output_int_hex(EFI_SYSTEM_TABLE* system_table,uint64_t value){
	uint16_t buffer[17];
	for (uint8_t i=0;i<16;i++){
		uint8_t j=(value>>((15-i)<<2))&0xf;
		buffer[i]=j+(j>9?87:48);
	}
	buffer[16]=0;
	system_table->ConOut->OutputString(system_table->ConOut,buffer);
}



static inline void _output_int_hex32(EFI_SYSTEM_TABLE* system_table,uint32_t value){
	uint16_t buffer[9];
	for (uint8_t i=0;i<8;i++){
		uint8_t j=(value>>((7-i)<<2))&0xf;
		buffer[i]=j+(j>9?87:48);
	}
	buffer[8]=0;
	system_table->ConOut->OutputString(system_table->ConOut,buffer);
}



static uint64_t _kfs2_load_node_into_memory(EFI_SYSTEM_TABLE* system_table,EFI_BLOCK_IO_PROTOCOL* block_io_protocol,const kfs2_root_block_t* kfs2_root_block,uint32_t block_size_shift,uint32_t inode,uint64_t address){
	uint8_t disk_buffer[KFS2_BLOCK_SIZE];
	if (EFI_ERROR(block_io_protocol->ReadBlocks(block_io_protocol,block_io_protocol->Media->MediaId,(kfs2_root_block->first_inode_block+KFS2_INODE_GET_BLOCK_INDEX(inode))<<block_size_shift,KFS2_BLOCK_SIZE,disk_buffer))){
		return 0;
	}
	kfs2_node_t node=*((const kfs2_node_t*)(disk_buffer+KFS2_INODE_GET_NODE_INDEX(inode)*sizeof(kfs2_node_t)));
	if (!node.size||!kfs_verify_crc(&node,sizeof(kfs2_node_t))||(node.flags&KFS2_INODE_TYPE_MASK)!=KFS2_INODE_TYPE_FILE){
		return 0;
	}
	uint64_t page_count=(node.size+PAGE_SIZE-1)>>PAGE_SIZE_SHIFT;
	if (EFI_ERROR(system_table->BootServices->AllocatePages(AllocateAddress,0x80000000,page_count,&address))){
		return 0;
	}
	switch (node.flags&KFS2_INODE_STORAGE_MASK){
		case KFS2_INODE_STORAGE_TYPE_INLINE:
			system_table->ConOut->OutputString(system_table->ConOut,L"Unimplemented: KFS2_INODE_STORAGE_TYPE_INLINE\r\n");
			system_table->RuntimeServices->ResetSystem(EfiResetShutdown,EFI_SUCCESS,0,NULL);
			break;
		case KFS2_INODE_STORAGE_TYPE_SINGLE:
			system_table->ConOut->OutputString(system_table->ConOut,L"Unimplemented: KFS2_INODE_STORAGE_TYPE_SINGLE\r\n");
			system_table->RuntimeServices->ResetSystem(EfiResetShutdown,EFI_SUCCESS,0,NULL);
			break;
		case KFS2_INODE_STORAGE_TYPE_DOUBLE:
			if (EFI_ERROR(block_io_protocol->ReadBlocks(block_io_protocol,block_io_protocol->Media->MediaId,(kfs2_root_block->first_data_block+node.data.double_)<<block_size_shift,KFS2_BLOCK_SIZE,disk_buffer))){
				goto _cleanup;
			}
			void* buffer_ptr=(void*)address;
			for (uint16_t i=0;i<(node.size+KFS2_BLOCK_SIZE-1)/KFS2_BLOCK_SIZE;i++){
				if (EFI_ERROR(block_io_protocol->ReadBlocks(block_io_protocol,block_io_protocol->Media->MediaId,(kfs2_root_block->first_data_block+(*((const uint64_t*)(disk_buffer+i*sizeof(uint64_t)))))<<block_size_shift,KFS2_BLOCK_SIZE,buffer_ptr))){
					goto _cleanup;
				}
				buffer_ptr+=KFS2_BLOCK_SIZE;
			}
			break;
		case KFS2_INODE_STORAGE_TYPE_TRIPLE:
			system_table->ConOut->OutputString(system_table->ConOut,L"Unimplemented: KFS2_INODE_STORAGE_TYPE_TRIPLE\r\n");
			system_table->RuntimeServices->ResetSystem(EfiResetShutdown,EFI_SUCCESS,0,NULL);
			break;
		case KFS2_INODE_STORAGE_TYPE_QUADRUPLE:
			system_table->ConOut->OutputString(system_table->ConOut,L"Unimplemented: KFS2_INODE_STORAGE_TYPE_QUADRUPLE\r\n");
			system_table->RuntimeServices->ResetSystem(EfiResetShutdown,EFI_SUCCESS,0,NULL);
			break;
	}
	return address+(page_count<<PAGE_SIZE_SHIFT);
_cleanup:
	system_table->BootServices->FreePages(address,page_count);
	return 0;
}



EFI_STATUS efi_main(EFI_HANDLE image,EFI_SYSTEM_TABLE* system_table){
	relocate_executable();
	system_table->ConOut->Reset(system_table->ConOut,0);
	UINTN buffer_size=0;
	system_table->BootServices->LocateHandle(ByProtocol,&efi_block_io_protocol_guid,NULL,&buffer_size,NULL);
	EFI_HANDLE* buffer;
	system_table->BootServices->AllocatePool(0x80000000,buffer_size,(void**)(&buffer));
	system_table->BootServices->LocateHandle(ByProtocol,&efi_block_io_protocol_guid,NULL,&buffer_size,buffer);
	uint64_t first_free_address=0;
	uint64_t initramfs_address=0;
	uint64_t initramfs_size=0;
	for (UINTN i=0;i<buffer_size/sizeof(EFI_HANDLE);i++){
		EFI_BLOCK_IO_PROTOCOL* block_io_protocol;
		if (EFI_ERROR(system_table->BootServices->HandleProtocol(buffer[i],&efi_block_io_protocol_guid,(void**)(&block_io_protocol)))||!block_io_protocol->Media->LastBlock||block_io_protocol->Media->BlockSize>KFS2_BLOCK_SIZE||block_io_protocol->Media->BlockSize&(block_io_protocol->Media->BlockSize-1)){
			continue;
		}
		uint8_t disk_buffer[KFS2_BLOCK_SIZE];
		if (EFI_ERROR(block_io_protocol->ReadBlocks(block_io_protocol,block_io_protocol->Media->MediaId,0,KFS2_BLOCK_SIZE,disk_buffer))){
			continue;
		}
		kfs2_root_block_t kfs2_root_block=*((const kfs2_root_block_t*)disk_buffer);
		if (kfs2_root_block.signature!=KFS2_ROOT_BLOCK_SIGNATURE||!kfs2_root_block.kernel_inode||!kfs2_root_block.initramfs_inode||!kfs_verify_crc(&kfs2_root_block,sizeof(kfs2_root_block_t))){
			continue;
		}
		uint32_t block_size_shift=63-__builtin_clzll(KFS2_BLOCK_SIZE/block_io_protocol->Media->BlockSize);
		first_free_address=_kfs2_load_node_into_memory(system_table,block_io_protocol,&kfs2_root_block,block_size_shift,kfs2_root_block.kernel_inode,KERNEL_MEMORY_ADDRESS);
		if (!first_free_address){
			continue;
		}
		initramfs_address=first_free_address;
		first_free_address=_kfs2_load_node_into_memory(system_table,block_io_protocol,&kfs2_root_block,block_size_shift,kfs2_root_block.initramfs_inode,first_free_address);
		if (!first_free_address){
			continue;
		}
		initramfs_size=first_free_address-initramfs_address;
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
	if (EFI_ERROR(system_table->BootServices->AllocatePages(AllocateAddress,0x80000000,3,&kernel_pagemap))){
		return EFI_SUCCESS;
	}
	first_free_address+=3*PAGE_SIZE;
	*((uint64_t*)(kernel_pagemap+0x0000))=0x00000003|(kernel_pagemap+0x1000);
	*((uint64_t*)(kernel_pagemap+0x0ff8))=0x00000003|(kernel_pagemap+0x2000);
	*((uint64_t*)(kernel_pagemap+0x1000))=0x00000083;
	*((uint64_t*)(kernel_pagemap+0x2ff8))=0x00000083;
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
		kernel_data->mmap[kernel_data->mmap_size].type=1;
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
