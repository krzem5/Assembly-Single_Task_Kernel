#ifndef _UEFI_EFI_GUID_H_
#define _UEFI_EFI_GUID_H_ 1
#include <uefi/efi/types.h>



typedef struct _EFI_GUID{
	u32 Data1;
	u16 Data2;
	u16 Data3;
	u8 Data4[8];
} efi_guid_t;



#endif
