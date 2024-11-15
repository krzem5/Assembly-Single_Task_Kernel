#ifndef _UEFI_EFI_TCG2_H_
#define _UEFI_EFI_TCG2_H_ 1
#include <stdint.h>
#include <uefi/efi/guid.h>



#define EFI_TCG2_PROTOCOL_GUID {0x607f766c,0x7455,0x42be,{0x93,0x0b,0xe4,0xd7,0x6d,0xb2,0x72,0x0f}}

#define EFI_TCG2_EVENT_HEADER_VERSION  1



typedef struct tdEFI_TCG2_PROTOCOL EFI_TCG2_PROTOCOL;



typedef struct tdEFI_TCG2_VERSION{
	UINT8 Major;
	UINT8 Minor;
} EFI_TCG2_VERSION;



typedef UINT32 EFI_TCG2_EVENT_LOG_BITMAP;



typedef UINT32 EFI_TCG2_EVENT_ALGORITHM_BITMAP;



typedef struct tdEFI_TCG2_BOOT_SERVICE_CAPABILITY{
	UINT8 Size;
	EFI_TCG2_VERSION StructureVersion;
	EFI_TCG2_VERSION ProtocolVersion;
	EFI_TCG2_EVENT_ALGORITHM_BITMAP HashAlgorithmBitmap;
	EFI_TCG2_EVENT_LOG_BITMAP SupportedEventLogs;
	BOOLEAN TPMPresentFlag;
	UINT16 MaxCommandSize;
	UINT16 MaxResponseSize;
	UINT32 ManufacturerID;
	UINT32 NumberOfPCRBanks;
	EFI_TCG2_EVENT_ALGORITHM_BITMAP ActivePcrBanks;
} EFI_TCG2_BOOT_SERVICE_CAPABILITY;



typedef struct __attribute__((packed)){
	UINT32 HeaderSize;
	UINT16 HeaderVersion;
	UINT32 PCRIndex;
	UINT32 EventType;
} EFI_TCG2_EVENT_HEADER;



typedef struct __attribute__((packed)) tdEFI_TCG2_EVENT{
	UINT32 Size;
	EFI_TCG2_EVENT_HEADER Header;
	UINT8 Event[1];
} EFI_TCG2_EVENT;



typedef EFI_STATUS (EFIAPI *EFI_TCG2_GET_CAPABILITY)(EFI_TCG2_PROTOCOL* This,EFI_TCG2_BOOT_SERVICE_CAPABILITY* ProtocolCapability);



typedef EFI_STATUS (EFIAPI *EFI_TCG2_HASH_LOG_EXTEND_EVENT)(EFI_TCG2_PROTOCOL* This,UINT64 Flags,EFI_PHYSICAL_ADDRESS DataToHash,UINT64 DataToHashLen,EFI_TCG2_EVENT* EfiTcgEvent);



typedef struct tdEFI_TCG2_PROTOCOL{
	EFI_TCG2_GET_CAPABILITY GetCapability;
	void* GetEventLog;
	EFI_TCG2_HASH_LOG_EXTEND_EVENT HashLogExtendEvent;
	void* SubmitCommand;
	void* GetActivePcrBanks;
	void* SetActivePcrBanks;
	void* GetResultOfSetActivePcrBanks;
} EFI_TCG2;



#endif
