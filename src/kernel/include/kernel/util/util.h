#ifndef _KERNEL_UTIL_UTIL_H_
#define _KERNEL_UTIL_UTIL_H_ 1



static inline void __pause(void){
	asm volatile("pause":::"memory");
}



void* memcpy(void* dst,const void* src,u64 length);



void* memset(void* dst,u8 value,u64 length);



#endif
