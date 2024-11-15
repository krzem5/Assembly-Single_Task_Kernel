#ifndef _UEFI_EFI_BLOCK_IO_H_
#define _UEFI_EFI_BLOCK_IO_H_ 1
#include <uefi/efi/types.h>



#define EFI_BLOCK_IO_PROTOCOL_GUID {0x964e5b21,0x6459,0x11d2,{0x8e,0x39,0x0,0xa0,0xc9,0x69,0x72,0x3b}}



typedef u64 efi_lba_t;



typedef struct _EFI_BLOCK_IO_MEDIA{
	u32 MediaId;
	efi_bool_t RemovableMedia;
	efi_bool_t MediaPresent;
	efi_bool_t LogicalPartition;
	efi_bool_t ReadOnly;
	efi_bool_t WriteCaching;
	u32 BlockSize;
	u32 IoAlign;
	efi_lba_t LastBlock;
	efi_lba_t LowestAlignedLba;
	u32 LogicalBlocksPerPhysicalBlock;
	u32 OptimalTransferLengthGranularity;
} efi_block_io_media_t;



typedef struct _EFI_BLOCK_IO_PROTOCOL{
	u64 Revision;
	efi_block_io_media_t* Media;
	efi_status_t (EFI_API* Reset)(struct _EFI_BLOCK_IO_PROTOCOL* This,efi_bool_t ExtendedVerification);
	efi_status_t (EFI_API* ReadBlocks)(struct _EFI_BLOCK_IO_PROTOCOL* This,u32 MediaId,efi_lba_t LBA,u64 BufferSize,void* Buffer);
	efi_status_t (EFI_API* WriteBlocks)(struct _EFI_BLOCK_IO_PROTOCOL* This,u32 MediaId,efi_lba_t LBA,u64 BufferSize,void* Buffer);
	efi_status_t (EFI_API* FlushBlocks)(struct _EFI_BLOCK_IO_PROTOCOL* This);
} efi_block_io_protocol_t;



#endif
