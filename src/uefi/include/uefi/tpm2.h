#ifndef _UEFI_TPM2_H_
#define _UEFI_TPM2_H_ 1
#include <stdint.h>



#define EFI_TCG2_PROTOCOL_GUID {0x607f766c, 0x7455, 0x42be, { 0x93, 0x0b, 0xe4, 0xd7, 0x6d, 0xb2, 0x72, 0x0f }}

typedef struct tdEFI_TCG2_PROTOCOL EFI_TCG2_PROTOCOL;

typedef struct tdEFI_TCG2_VERSION {
        UINT8 Major;
        UINT8 Minor;
} EFI_TCG2_VERSION;

typedef UINT32 EFI_TCG2_EVENT_LOG_BITMAP;
typedef UINT32 EFI_TCG2_EVENT_LOG_FORMAT;
typedef UINT32 EFI_TCG2_EVENT_ALGORITHM_BITMAP;

#define EFI_TCG2_EVENT_LOG_FORMAT_TCG_1_2       0x00000001
#define EFI_TCG2_EVENT_LOG_FORMAT_TCG_2         0x00000002

typedef struct tdEFI_TCG2_BOOT_SERVICE_CAPABILITY {
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

#define EFI_TCG2_EVENT_HEADER_VERSION  1

typedef struct __attribute__((packed)){
        UINT32 HeaderSize;
        UINT16 HeaderVersion;
        UINT32 PCRIndex;
        UINT32 EventType;
} EFI_TCG2_EVENT_HEADER;

typedef struct __attribute__((packed)) tdEFI_TCG2_EVENT {
        UINT32 Size;
        EFI_TCG2_EVENT_HEADER Header;
        UINT8 Event[1];
} EFI_TCG2_EVENT;

typedef EFI_STATUS(EFIAPI * EFI_TCG2_GET_CAPABILITY) (IN EFI_TCG2_PROTOCOL * This,
                                                      IN OUT EFI_TCG2_BOOT_SERVICE_CAPABILITY * ProtocolCapability);

typedef EFI_STATUS(EFIAPI * EFI_TCG2_GET_EVENT_LOG) (IN EFI_TCG2_PROTOCOL * This,
                                                     IN EFI_TCG2_EVENT_LOG_FORMAT EventLogFormat,
                                                     OUT EFI_PHYSICAL_ADDRESS * EventLogLocation,
                                                     OUT EFI_PHYSICAL_ADDRESS * EventLogLastEntry,
                                                     OUT BOOLEAN * EventLogTruncated);

typedef EFI_STATUS(EFIAPI * EFI_TCG2_HASH_LOG_EXTEND_EVENT) (IN EFI_TCG2_PROTOCOL * This,
                                                             IN UINT64 Flags,
                                                             IN EFI_PHYSICAL_ADDRESS DataToHash,
                                                             IN UINT64 DataToHashLen, IN EFI_TCG2_EVENT * EfiTcgEvent);

typedef EFI_STATUS(EFIAPI * EFI_TCG2_SUBMIT_COMMAND) (IN EFI_TCG2_PROTOCOL * This,
                                                      IN UINT32 InputParameterBlockSize,
                                                      IN UINT8 * InputParameterBlock,
                                                      IN UINT32 OutputParameterBlockSize, IN UINT8 * OutputParameterBlock);

typedef EFI_STATUS(EFIAPI * EFI_TCG2_GET_ACTIVE_PCR_BANKS) (IN EFI_TCG2_PROTOCOL * This, OUT UINT32 * ActivePcrBanks);

typedef EFI_STATUS(EFIAPI * EFI_TCG2_SET_ACTIVE_PCR_BANKS) (IN EFI_TCG2_PROTOCOL * This, IN UINT32 ActivePcrBanks);

typedef EFI_STATUS(EFIAPI * EFI_TCG2_GET_RESULT_OF_SET_ACTIVE_PCR_BANKS) (IN EFI_TCG2_PROTOCOL * This,
                                                                          OUT UINT32 * OperationPresent, OUT UINT32 * Response);

typedef struct tdEFI_TCG2_PROTOCOL {
        EFI_TCG2_GET_CAPABILITY GetCapability;
        EFI_TCG2_GET_EVENT_LOG GetEventLog;
        EFI_TCG2_HASH_LOG_EXTEND_EVENT HashLogExtendEvent;
        EFI_TCG2_SUBMIT_COMMAND SubmitCommand;
        EFI_TCG2_GET_ACTIVE_PCR_BANKS GetActivePcrBanks;
        EFI_TCG2_SET_ACTIVE_PCR_BANKS SetActivePcrBanks;
        EFI_TCG2_GET_RESULT_OF_SET_ACTIVE_PCR_BANKS GetResultOfSetActivePcrBanks;
} EFI_TCG2;



#endif
