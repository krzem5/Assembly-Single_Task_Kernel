#ifndef _UEFI_EFI_BOOT_SERVICES_H_
#define _UEFI_EFI_BOOT_SERVICES_H_ 1
#include <uefi/efi/guid.h>
#include <uefi/efi/types.h>



typedef enum _EFI_LOCATE_SEARCH_TYPE{
	AllHandles,
	ByRegisterNotify,
	ByProtocol
} efi_locate_search_type_t;



typedef enum _EFI_ALLOCATE_TYPE{
	AllocateAnyPages,
	AllocateMaxAddress,
	AllocateAddress,
	MaxAllocateType
} efi_allocate_type_t;



typedef enum _EFI_MEMORY_TYPE{
	EfiReservedMemoryType,
	EfiLoaderCode,
	EfiLoaderData,
	EfiBootServicesCode,
	EfiBootServicesData,
	EfiRuntimeServicesCode,
	EfiRuntimeServicesData,
	EfiConventionalMemory,
	EfiUnusableMemory,
	EfiACPIReclaimMemory,
	EfiACPIMemoryNVS,
	EfiMemoryMappedIO,
	EfiMemoryMappedIOPortSpace,
	EfiPalCode,
	EfiMaxMemoryType
} efi_memory_type_t;



typedef struct _EFI_MEMORY_DESCRIPTOR{
	u32 Type;
	u32 Pad;
	efi_physical_address_t PhysicalStart;
	efi_virtual_address_t VirtualStart;
	u64 NumberOfPages;
	u64 Attribute;
} efi_memory_descriptor_t;



typedef struct _EFI_BOOT_SERVICES{
	efi_table_header_t Hdr;
	void* RaiseTPL;
	void* RestoreTPL;
	efi_status_t (EFI_API* AllocatePages)(efi_allocate_type_t Type,efi_memory_type_t MemoryType,u64 NoPages,efi_physical_address_t* Memory);
	efi_status_t (EFI_API* FreePages)(efi_physical_address_t Memory,u64 NoPages);
	efi_status_t (EFI_API* GetMemoryMap)(u64* MemoryMapSize,efi_memory_descriptor_t* MemoryMap,u64* MapKey,u64* DescriptorSize,u32* DescriptorVersion);
	efi_status_t (EFI_API* AllocatePool)(efi_memory_type_t PoolType,u64 Size,void** Buffer);
	efi_status_t (EFI_API* FreePool)(void* Buffer);
	void* CreateEvent;
	void* SetTimer;
	void* WaitForEvent;
	void* SignalEvent;
	void* CloseEvent;
	void* CheckEvent;
	void* InstallProtocolInterface;
	void* ReinstallProtocolInterface;
	void* UninstallProtocolInterface;
	efi_status_t (EFI_API* HandleProtocol)(void* Handle,efi_guid_t* Protocol,void* *Interface);
	void* PCHandleProtocol;
	void* RegisterProtocolNotify;
	efi_status_t (EFI_API* LocateHandle)(efi_locate_search_type_t SearchType,efi_guid_t* Protocol,void* SearchKey,u64* BufferSize,void** Buffer);
	void* LocateDevicePath;
	void* InstallConfigurationTable;
	void* LoadImage;
	void* StartImage;
	void* Exit;
	void* UnloadImage;
	efi_status_t (EFI_API* ExitBootServices)(void* ImageHandle,u64 MapKey);
	void* GetNextMonotonicCount;
	void* Stall;
	void* SetWatchdogTimer;
	void* ConnectController;
	void* DisconnectController;
	void* OpenProtocol;
	void* CloseProtocol;
	void* OpenProtocolInformation;
	void* ProtocolsPerHandle;
	void* LocateHandleBuffer;
	void* LocateProtocol;
	void* InstallMultipleProtocolInterfaces;
	void* UninstallMultipleProtocolInterfaces;
	void* CalculateCrc32;
	void (EFI_API* CopyMem)(void* Destination,void* Source,u64 Length);
	void (EFI_API* SetMem)(void* Buffer,u64 Size,u8 Value);
	void* CreateEventEx;
} efi_boot_services_t;



#endif
