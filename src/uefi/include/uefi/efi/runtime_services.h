#ifndef _UEFI_EFI_RUNTIME_TABLE_H_
#define _UEFI_EFI_RUNTIME_TABLE_H_ 1
#include <uefi/efi/time.h>
#include <uefi/efi/types.h>



typedef struct _EFI_RUNTIME_SERVICES{
	efi_table_header_t Hdr;
	efi_status_t (EFI_API* GetTime)(efi_time_t* Time,void* Capabilities);
	void* SetTime;
	void* GetWakeupTime;
	void* SetWakeupTime;
	efi_status_t (EFI_API* SetVirtualAddressMap)(u64 MemoryMapSize,u64 DescriptorSize,u32 DescriptorVersion,efi_memory_descriptor_t* VirtualMap);
	void* ConvertPointer;
	void* GetVariable;
	void* GetNextVariableName;
	void* SetVariable;
	void* GetNextHighMonotonicCount;
	void* ResetSystem;
	void* UpdateCapsule;
	void* QueryCapsuleCapabilities;
	void* QueryVariableInfo;
} efi_runtime_services_t;



#endif
