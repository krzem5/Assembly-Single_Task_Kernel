#ifndef _UEFI_EFI_GUID_H_
#define _UEFI_EFI_GUID_H_ 1
#include <stdint.h>

#define EFIAPI __attribute__((ms_abi))

typedef uint16_t   UINT16;
typedef int16_t    INT16;
typedef uint8_t    UINT8;
typedef int8_t     INT8;
typedef uint64_t   UINT64;
typedef int64_t    INT64;
typedef uint32_t    UINT32;
typedef int32_t     INT32;
typedef UINT8           BOOLEAN;
typedef int64_t    INTN;
typedef uint64_t   UINTN;
typedef void   VOID;
typedef uint16_t   CHAR16;
typedef struct {
    UINT16      Year;
    UINT8       Month;
    UINT8       Day;
    UINT8       Hour;
    UINT8       Minute;
    UINT8       Second;
    UINT8       Pad1;
    UINT32      Nanosecond;
    INT16       TimeZone;
    UINT8       Daylight;
    UINT8       Pad2;
} EFI_TIME;

typedef UINT64          EFI_LBA;
typedef struct{
	UINT32 Data1;
	UINT16 Data2;
	UINT16 Data3;
	UINT8 Data4[8];
} EFI_GUID;
typedef enum {
    AllHandles,
    ByRegisterNotify,
    ByProtocol
} EFI_LOCATE_SEARCH_TYPE;



typedef enum {
    AllocateAnyPages,
    AllocateMaxAddress,
    AllocateAddress,
    MaxAllocateType
} EFI_ALLOCATE_TYPE;
typedef UINTN           EFI_STATUS;
typedef VOID            *EFI_HANDLE;
typedef UINT64          EFI_PHYSICAL_ADDRESS;
typedef UINT64          EFI_VIRTUAL_ADDRESS;
#define EFI_MEMORY_DESCRIPTOR_VERSION  1
typedef struct {
    UINT32                          Type;           // Field size is 32 bits followed by 32 bit pad
    UINT32                          Pad;
    EFI_PHYSICAL_ADDRESS            PhysicalStart;  // Field size is 64 bits
    EFI_VIRTUAL_ADDRESS             VirtualStart;   // Field size is 64 bits
    UINT64                          NumberOfPages;  // Field size is 64 bits
    UINT64                          Attribute;      // Field size is 64 bits
} EFI_MEMORY_DESCRIPTOR;

#define EFIERR(a)           (0x8000000000000000 | a)
#define EFI_ERROR(a)              (((INTN) a) < 0)
#define EFI_SUCCESS                             0
#define EFI_INVALID_PARAMETER           EFIERR(2)
#define EFI_BUFFER_TOO_SMALL            EFIERR(5)


typedef enum {
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
} EFI_MEMORY_TYPE;


typedef struct _EFI_TABLE_HEADER {
    UINT64                      Signature;
    UINT32                      Revision;
    UINT32                      HeaderSize;
    UINT32                      CRC32;
    UINT32                      Reserved;
} EFI_TABLE_HEADER;



#define EFI_SYSTEM_TABLE_SIGNATURE      0x5453595320494249
#define EFI_SYSTEM_TABLE_REVISION      (EFI_SPECIFICATION_MAJOR_REVISION<<16) | (EFI_SPECIFICATION_MINOR_REVISION)


typedef
EFI_STATUS
(EFIAPI *EFI_ALLOCATE_PAGES) (
     EFI_ALLOCATE_TYPE            Type,
     EFI_MEMORY_TYPE              MemoryType,
     UINTN                        NoPages,
     EFI_PHYSICAL_ADDRESS        *Memory
    );

typedef
EFI_STATUS
(EFIAPI *EFI_FREE_PAGES) (
     EFI_PHYSICAL_ADDRESS         Memory,
     UINTN                        NoPages
    );

typedef
EFI_STATUS
(EFIAPI *EFI_GET_MEMORY_MAP) (
      UINTN                    *MemoryMapSize,
      EFI_MEMORY_DESCRIPTOR    *MemoryMap,
     UINTN                       *MapKey,
     UINTN                       *DescriptorSize,
     UINT32                      *DescriptorVersion
    );

typedef
VOID
(EFIAPI *EFI_COPY_MEM) (
     VOID                     *Destination,
     VOID                     *Source,
     UINTN                    Length
    );

typedef
VOID
(EFIAPI *EFI_SET_MEM) (
     VOID                     *Buffer,
     UINTN                    Size,
     UINT8                    Value
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SET_VIRTUAL_ADDRESS_MAP) (
     UINTN                        MemoryMapSize,
     UINTN                        DescriptorSize,
     UINT32                       DescriptorVersion,
     EFI_MEMORY_DESCRIPTOR        *VirtualMap
    );

typedef
EFI_STATUS
(EFIAPI *EFI_EXIT_BOOT_SERVICES) (
     EFI_HANDLE                   ImageHandle,
     UINTN                        MapKey
    );


typedef
EFI_STATUS
(EFIAPI *EFI_ALLOCATE_POOL) (
     EFI_MEMORY_TYPE              PoolType,
     UINTN                        Size,
     VOID                        **Buffer
    );

typedef
EFI_STATUS
(EFIAPI *EFI_FREE_POOL) (
     VOID                         *Buffer
    );

typedef
EFI_STATUS
(EFIAPI *EFI_HANDLE_PROTOCOL) (
     EFI_HANDLE               Handle,
     EFI_GUID                 *Protocol,
     VOID                    **Interface
    );

typedef
EFI_STATUS
(EFIAPI *EFI_LOCATE_HANDLE) (
     EFI_LOCATE_SEARCH_TYPE   SearchType,
     EFI_GUID                 *Protocol,
     VOID                     *SearchKey,
      UINTN                *BufferSize,
     EFI_HANDLE              *Buffer
    );

typedef struct _EFI_BOOT_SERVICES {

    EFI_TABLE_HEADER                Hdr;
    void*/*EFI_RAISE_TPL*/                   RaiseTPL;
    void*/*EFI_RESTORE_TPL*/                 RestoreTPL;
    EFI_ALLOCATE_PAGES              AllocatePages;
    EFI_FREE_PAGES                  FreePages;
    EFI_GET_MEMORY_MAP              GetMemoryMap;
    EFI_ALLOCATE_POOL               AllocatePool;
    EFI_FREE_POOL                   FreePool;
    void*/*EFI_CREATE_EVENT*/                CreateEvent;
    void*/*EFI_SET_TIMER*/                   SetTimer;
    void*/*EFI_WAIT_FOR_EVENT*/              WaitForEvent;
    void*/*EFI_SIGNAL_EVENT*/                SignalEvent;
    void*/*EFI_CLOSE_EVENT*/                 CloseEvent;
    void*/*EFI_CHECK_EVENT*/                 CheckEvent;
    void*/*EFI_INSTALL_PROTOCOL_INTERFACE*/  InstallProtocolInterface;
    void*/*EFI_REINSTALL_PROTOCOL_INTERFACE*/ ReinstallProtocolInterface;
    void*/*EFI_UNINSTALL_PROTOCOL_INTERFACE*/ UninstallProtocolInterface;
    EFI_HANDLE_PROTOCOL             HandleProtocol;
    EFI_HANDLE_PROTOCOL             PCHandleProtocol;
    void*/*EFI_REGISTER_PROTOCOL_NOTIFY*/    RegisterProtocolNotify;
    EFI_LOCATE_HANDLE               LocateHandle;
    void*/*EFI_LOCATE_DEVICE_PATH*/          LocateDevicePath;
    void*/*EFI_INSTALL_CONFIGURATION_TABLE*/ InstallConfigurationTable;
    void*/*EFI_IMAGE_LOAD*/                  LoadImage;
    void*/*EFI_IMAGE_START*/                 StartImage;
    void*/*EFI_EXIT*/                        Exit;
    void*/*EFI_IMAGE_UNLOAD*/                UnloadImage;
    EFI_EXIT_BOOT_SERVICES          ExitBootServices;
    void*/*EFI_GET_NEXT_MONOTONIC_COUNT*/    GetNextMonotonicCount;
    void*/*EFI_STALL*/                       Stall;
    void*/*EFI_SET_WATCHDOG_TIMER*/          SetWatchdogTimer;
    void*/*EFI_CONNECT_CONTROLLER*/          ConnectController;
    void*/*EFI_DISCONNECT_CONTROLLER*/       DisconnectController;
    void*/*EFI_OPEN_PROTOCOL*/               OpenProtocol;
    void*/*EFI_CLOSE_PROTOCOL*/              CloseProtocol;
    void*/*EFI_OPEN_PROTOCOL_INFORMATION*/   OpenProtocolInformation;
    void*/*EFI_PROTOCOLS_PER_HANDLE*/        ProtocolsPerHandle;
    void*/*EFI_LOCATE_HANDLE_BUFFER*/        LocateHandleBuffer;
    void*/*EFI_LOCATE_PROTOCOL*/             LocateProtocol;
    void*/*EFI_INSTALL_MULTIPLE_PROTOCOL_INTERFACES*/ InstallMultipleProtocolInterfaces;
    void*/*EFI_UNINSTALL_MULTIPLE_PROTOCOL_INTERFACES*/ UninstallMultipleProtocolInterfaces;
    void*/*EFI_CALCULATE_CRC32*/             CalculateCrc32;
    EFI_COPY_MEM                    CopyMem;
    EFI_SET_MEM                     SetMem;
    void*/*EFI_CREATE_EVENT_EX*/             CreateEventEx;
} EFI_BOOT_SERVICES;


typedef
EFI_STATUS
(EFIAPI *EFI_GET_TIME) (
     EFI_TIME                    *Time,
    void       *Capabilities
    );

typedef struct  {
    EFI_TABLE_HEADER                Hdr;
    EFI_GET_TIME                    GetTime;
    void*/*EFI_SET_TIME*/                    SetTime;
    void*/*EFI_GET_WAKEUP_TIME*/             GetWakeupTime;
    void*/*EFI_SET_WAKEUP_TIME*/             SetWakeupTime;
    EFI_SET_VIRTUAL_ADDRESS_MAP     SetVirtualAddressMap;
    void*/*EFI_CONVERT_POINTER*/             ConvertPointer;
    void*/*EFI_GET_VARIABLE*/                GetVariable;
    void*/*EFI_GET_NEXT_VARIABLE_NAME*/      GetNextVariableName;
    void*/*EFI_SET_VARIABLE*/                SetVariable;
    void*/*EFI_GET_NEXT_HIGH_MONO_COUNT*/    GetNextHighMonotonicCount;
    void*/*EFI_RESET_SYSTEM*/                ResetSystem;
    void*/*EFI_UPDATE_CAPSULE*/              UpdateCapsule;
    void*/*EFI_QUERY_CAPSULE_CAPABILITIES*/  QueryCapsuleCapabilities;
    void*/*EFI_QUERY_VARIABLE_INFO*/         QueryVariableInfo;
} EFI_RUNTIME_SERVICES;

typedef struct _EFI_CONFIGURATION_TABLE {
    EFI_GUID                VendorGuid;
    VOID                    *VendorTable;
} EFI_CONFIGURATION_TABLE;


typedef struct _SIMPLE_TEXT_OUTPUT_INTERFACE SIMPLE_TEXT_OUTPUT_INTERFACE;
typedef
EFI_STATUS
(EFIAPI *EFI_TEXT_RESET) (
   SIMPLE_TEXT_OUTPUT_INTERFACE *This,
   BOOLEAN                       ExtendedVerification
  );

typedef
EFI_STATUS
(EFIAPI *EFI_TEXT_STRING) (
   SIMPLE_TEXT_OUTPUT_INTERFACE *This,
   CHAR16                       *String
  );

typedef struct _SIMPLE_TEXT_OUTPUT_INTERFACE {
  EFI_TEXT_RESET                  Reset;
  EFI_TEXT_STRING                 OutputString;
  void*/*EFI_TEXT_TEST_STRING*/            TestString;
  void*/*EFI_TEXT_QUERY_MODE*/             QueryMode;
  void*/*EFI_TEXT_SET_MODE*/               SetMode;
  void*/*EFI_TEXT_SET_ATTRIBUTE*/          SetAttribute;
  void*/*EFI_TEXT_CLEAR_SCREEN*/           ClearScreen;
  void*/*EFI_TEXT_SET_CURSOR_POSITION*/    SetCursorPosition;
  void*/*EFI_TEXT_ENABLE_CURSOR*/          EnableCursor;
  void*/*SIMPLE_TEXT_OUTPUT_MODE*/         *Mode;
} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;



typedef struct _EFI_SYSTEM_TABLE {
    EFI_TABLE_HEADER                Hdr;

    CHAR16                          *FirmwareVendor;
    UINT32                          FirmwareRevision;

    EFI_HANDLE                      ConsoleInHandle;
    void          *ConIn;

    EFI_HANDLE                      ConsoleOutHandle;
    SIMPLE_TEXT_OUTPUT_INTERFACE    *ConOut;

    EFI_HANDLE                      StandardErrorHandle;
    SIMPLE_TEXT_OUTPUT_INTERFACE    *StdErr;

    EFI_RUNTIME_SERVICES            *RuntimeServices;
    EFI_BOOT_SERVICES               *BootServices;

    UINTN                           NumberOfTableEntries;
    EFI_CONFIGURATION_TABLE         *ConfigurationTable;

} EFI_SYSTEM_TABLE;

#define EFI_BLOCK_IO_PROTOCOL_GUID \
    { 0x964e5b21, 0x6459, 0x11d2, {0x8e, 0x39, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b} }

#define ACPI_20_TABLE_GUID  \
    { 0x8868e871, 0xe4f1, 0x11d3, {0xbc, 0x22, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81} }

#define SMBIOS_TABLE_GUID    \
    { 0xeb9d2d31, 0x2d88, 0x11d3, {0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d} }

struct _EFI_BLOCK_IO_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI *EFI_BLOCK_RESET) (
    struct _EFI_BLOCK_IO_PROTOCOL  *This,
    BOOLEAN                        ExtendedVerification
    );

typedef
EFI_STATUS
(EFIAPI *EFI_BLOCK_READ) (
    struct _EFI_BLOCK_IO_PROTOCOL  *This,
    UINT32                         MediaId,
    EFI_LBA                        LBA,
    UINTN                          BufferSize,
    VOID                          *Buffer
    );


typedef
EFI_STATUS
(EFIAPI *EFI_BLOCK_WRITE) (
    struct _EFI_BLOCK_IO_PROTOCOL  *This,
    UINT32                         MediaId,
    EFI_LBA                        LBA,
    UINTN                          BufferSize,
    VOID                           *Buffer
    );


typedef
EFI_STATUS
(EFIAPI *EFI_BLOCK_FLUSH) (
    struct _EFI_BLOCK_IO_PROTOCOL  *This
    );



typedef struct{
    UINT32              MediaId;
    BOOLEAN             RemovableMedia;
    BOOLEAN             MediaPresent;
    BOOLEAN             LogicalPartition;
    BOOLEAN             ReadOnly;
    BOOLEAN             WriteCaching;
    UINT32              BlockSize;
    UINT32              IoAlign;
    EFI_LBA             LastBlock;
    EFI_LBA             LowestAlignedLba;
    UINT32              LogicalBlocksPerPhysicalBlock;
    UINT32              OptimalTransferLengthGranularity;
} EFI_BLOCK_IO_MEDIA;


typedef struct _EFI_BLOCK_IO_PROTOCOL {
    UINT64                  Revision;

    EFI_BLOCK_IO_MEDIA      *Media;

    EFI_BLOCK_RESET         Reset;
    EFI_BLOCK_READ          ReadBlocks;
    EFI_BLOCK_WRITE         WriteBlocks;
    EFI_BLOCK_FLUSH         FlushBlocks;

} EFI_BLOCK_IO_PROTOCOL;

#endif
