#ifndef _UEFI_EFI_SYSTEM_TABLE_H_
#define _UEFI_EFI_SYSTEM_TABLE_H_ 1
#include <uefi/efi/boot_services.h>
#include <uefi/efi/runtime_services.h>
#include <uefi/efi/simple_text_output.h>
#include <uefi/efi/types.h>



typedef struct _EFI_CONFIGURATION_TABLE{
	efi_guid_t VendorGuid;
	void* VendorTable;
} efi_configuration_table_t;



typedef struct _EFI_SYSTEM_TABLE{
	efi_table_header_t Hdr;
	efi_wchar_t* FirmwareVendor;
	u32 FirmwareRevision;
	void* ConsoleInHandle;
	void* ConIn;
	void* ConsoleOutHandle;
	efi_simple_text_output_protocol_t* ConOut;
	void* StandardErrorHandle;
	efi_simple_text_output_protocol_t* StdErr;
	efi_runtime_services_t* RuntimeServices;
	efi_boot_services_t* BootServices;
	u64 NumberOfTableEntries;
	efi_configuration_table_t* ConfigurationTable;
} efi_system_table_t;



#endif
