#ifndef _COMMON_KFS2_UTIL_H_
#define _COMMON_KFS2_UTIL_H_ 1
#if BUILD_MODULE
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/util/util.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long long int u64;

typedef signed char s8;
typedef signed short int s16;
typedef signed int s32;
typedef signed long long int s64;



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
