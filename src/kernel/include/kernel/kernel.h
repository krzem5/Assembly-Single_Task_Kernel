#ifndef _KERNEL_KERNEL_H_
#define _KERNEL_KERNEL_H_ 1
#include <kernel/types.h>



#define KERNEL_OFFSET 0xffffffffc0000000ull



extern u64 __KERNEL_START__[1];
extern u64 __KERNEL_CORE_END__[1];
extern u64 __KERNEL_COMMON_START__[1];
extern u64 __KERNEL_END__[1];

extern const u64 __core_version;



static inline u64 kernel_get_version(void){
	return __core_version;
}



static inline u64 kernel_get_start(void){
	return (u64)__KERNEL_START__;
}



static inline u64 kernel_get_core_end(void){
	return (u64)__KERNEL_CORE_END__;
}



static inline u64 kernel_get_common_start(void){
	return (u64)__KERNEL_COMMON_START__;
}



static inline u64 kernel_get_end(void){
	return (u64)__KERNEL_END__;
}



static inline u64 kernel_get_offset(void){
	return KERNEL_OFFSET;
}



const kernel_data_t* kernel_init(void);



void kernel_load(void);



#endif
