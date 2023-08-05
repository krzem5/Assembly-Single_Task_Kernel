#ifndef _KERNEL_UTIL_UTIL_H_
#define _KERNEL_UTIL_UTIL_H_ 1



static inline void __pause(void){
	asm volatile("pause":::"memory");
}



#endif
