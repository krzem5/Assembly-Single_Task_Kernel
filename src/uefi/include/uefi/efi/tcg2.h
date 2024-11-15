#ifndef _UEFI_EFI_TCG2_H_
#define _UEFI_EFI_TCG2_H_ 1
#include <uefi/efi/types.h>



#define EFI_TCG2_PROTOCOL_GUID {0x607f766c,0x7455,0x42be,{0x93,0x0b,0xe4,0xd7,0x6d,0xb2,0x72,0x0f}}

#define EFI_TCG2_EVENT_HEADER_VERSION  1



typedef struct _EFI_TCG2_VERSION{
	u8 Major;
	u8 Minor;
} efi_tcg2_version_t;



typedef struct _EFI_TCG2_BOOT_SERVICE_CAPABILITY{
	u8 Size;
	efi_tcg2_version_t StructureVersion;
	efi_tcg2_version_t ProtocolVersion;
	u32 HashAlgorithmBitmap;
	u32 SupportedEventLogs;
	efi_bool_t TPMPresentFlag;
	u16 MaxCommandSize;
	u16 MaxResponseSize;
	u32 ManufacturerID;
	u32 NumberOfPCRBanks;
	u32 ActivePcrBanks;
} efi_tcg2_boot_service_capability_t;



typedef struct __attribute__((packed)) _EFI_TCG2_EVENT_HEADER{
	u32 HeaderSize;
	u16 HeaderVersion;
	u32 PCRIndex;
	u32 EventType;
} efi_tcg2_event_header_t;



typedef struct __attribute__((packed)) _EFI_TCG2_EVENT{
	u32 Size;
	efi_tcg2_event_header_t Header;
	u8 Event[1];
} efi_tcg2_event_t;



typedef struct _EFI_TCG2_PROTOCOL{
	efi_status_t (EFI_API *GetCapability)(struct _EFI_TCG2_PROTOCOL* This,efi_tcg2_boot_service_capability_t* ProtocolCapability);
	void* GetEventLog;
	efi_status_t (EFI_API *HashLogExtendEvent)(struct _EFI_TCG2_PROTOCOL* This,u64 Flags,efi_physical_address_t DataToHash,u64 DataToHashLen,efi_tcg2_event_t* EfiTcgEvent);
	void* SubmitCommand;
	void* GetActivePcrBanks;
	void* SetActivePcrBanks;
	void* GetResultOfSetActivePcrBanks;
} efi_tcg2_protocol_t;



#endif
