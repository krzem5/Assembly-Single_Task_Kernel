#ifndef _COMMON_UEFI_H_
#define _COMMON_UEFI_H_ 1
#ifndef BUILD_UEFI
#error Wrong build mode
#endif
#include <efi.h>



extern EFI_SYSTEM_TABLE* uefi_global_system_table;



static inline __attribute__((always_inline)) void mem_copy(void* dst,const void* src,u64 length){
	uefi_global_system_table->BootServices->CopyMem(dst,(void*)src,length);
}



static inline __attribute__((always_inline)) void mem_fill(void* ptr,u64 length,u8 value){
	uefi_global_system_table->BootServices->SetMem(ptr,length,value);
}



static inline __attribute__((always_inline,noreturn)) void panic(const char* msg){
	u16 buffer[512];
	u32 i=0;
	for (;msg[i];i++){
		buffer[i]=msg[i];
	}
	buffer[i]='\r';
	buffer[i+1]='\n';
	buffer[i+2]=0;
	uefi_global_system_table->ConOut->OutputString(uefi_global_system_table->ConOut,buffer);
	for (;;);
}



#endif
