#include <efi.h>
#include <uefi/relocator.h>



#define MAX_BLOCK_SIZE 4096

#define KFS2_ROOT_BLOCK_SIGNATURE 0x544f4f523253464b



typedef struct _KFS2_ROOT_BLOCK{
	uint64_t signature;
} kfs2_root_block_t;



static EFI_GUID efi_block_io_protocol_guid=EFI_BLOCK_IO_PROTOCOL_GUID;



// static void _output_int(EFI_SYSTEM_TABLE* system_table,uint64_t value){
// 	uint16_t buffer[21];
// 	buffer[20]=0;
// 	uint8_t i=20;
// 	do{
// 		i--;
// 		buffer[i]=(value%10)+48;
// 		value/=10;
// 	} while (value);
// 	system_table->ConOut->OutputString(system_table->ConOut,buffer+i);
// }



static void _output_int_hex(EFI_SYSTEM_TABLE* system_table,uint64_t value){
	uint16_t buffer[17];
	for (uint8_t i=0;i<16;i++){
		uint8_t j=(value>>((15-i)<<2))&0xf;
		buffer[i]=j+(j>9?87:48);
	}
	buffer[16]=0;
	system_table->ConOut->OutputString(system_table->ConOut,buffer);
}



EFI_STATUS efi_main(EFI_HANDLE image,EFI_SYSTEM_TABLE* system_table){
	relocate_executable();
	system_table->ConOut->Reset(system_table->ConOut,0);
	UINTN buffer_size=0;
	system_table->BootServices->LocateHandle(ByProtocol,&efi_block_io_protocol_guid,NULL,&buffer_size,NULL);
	EFI_HANDLE* buffer;
	system_table->BootServices->AllocatePool(0x80000000,buffer_size,(void**)(&buffer));
	system_table->BootServices->LocateHandle(ByProtocol,&efi_block_io_protocol_guid,NULL,&buffer_size,buffer);
	for (UINTN i=0;i<buffer_size/sizeof(EFI_HANDLE);i++){
		/**********************************************************************/
		EFI_BLOCK_IO_PROTOCOL* block_io_protocol;
		if (EFI_ERROR(system_table->BootServices->HandleProtocol(buffer[i],&efi_block_io_protocol_guid,(void**)(&block_io_protocol)))||!block_io_protocol->Media->LastBlock||block_io_protocol->Media->BlockSize>MAX_BLOCK_SIZE){
			continue;
		}
		uint8_t buffer[MAX_BLOCK_SIZE];
		if (EFI_ERROR(block_io_protocol->ReadBlocks(block_io_protocol,block_io_protocol->Media->MediaId,0,MAX_BLOCK_SIZE,buffer))){
			continue;
		}
		kfs2_root_block_t kfs2_root_block=*((const kfs2_root_block_t*)buffer);
		if (kfs2_root_block.signature!=KFS2_ROOT_BLOCK_SIGNATURE){
			continue;
		}
		_output_int_hex(system_table,kfs2_root_block.signature);
		system_table->ConOut->OutputString(system_table->ConOut,L"\r\n");
		/**********************************************************************/
	}
	system_table->BootServices->FreePool(buffer);
	system_table->RuntimeServices->ResetSystem(EfiResetShutdown,EFI_SUCCESS,0,NULL);
	return EFI_SUCCESS;
}
