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
