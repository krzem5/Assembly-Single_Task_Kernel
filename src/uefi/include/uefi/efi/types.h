#ifndef _UEFI_EFI_TYPES_H_
#define _UEFI_EFI_TYPES_H_ 1
#include <common/types.h>



#define EFI_API __attribute__((ms_abi))



typedef u8 efi_bool_t;



typedef u16 efi_wchar_t;



typedef u64 efi_status_t;



typedef u64 efi_physical_address_t;



typedef u64 efi_virtual_address_t;



typedef struct _EFI_TABLE_HEADER{
	u64 Signature;
	u32 Revision;
	u32 HeaderSize;
	u32 CRC32;
	u32 Reserved;
} efi_table_header_t;



#endif
