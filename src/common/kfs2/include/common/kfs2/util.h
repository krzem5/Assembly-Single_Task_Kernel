#ifndef _COMMON_KFS2_UTIL_H_
#define _COMMON_KFS2_UTIL_H_ 1
#if BUILD_MODULE
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/util/util.h>
#else



typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long long int u64;

typedef signed char s8;
typedef signed short int s16;
typedef signed int s32;
typedef signed long long int s64;



#if BUILD_UEFI
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



#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



static inline __attribute__((always_inline)) void mem_copy(void* dst,const void* src,u64 length){
	memcpy(dst,src,length);
}



static inline __attribute__((always_inline)) void mem_fill(void* ptr,u64 length,u8 value){
	memset(ptr,value,length);
}



static inline __attribute__((always_inline,noreturn)) void panic(const char* msg){
	printf("%s\n",msg);
	exit(1);
}



#endif
#endif
#endif
