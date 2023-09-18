#ifndef _KERNEL_KERNEL_H_
#define _KERNEL_KERNEL_H_ 1
#include <kernel/types.h>



#define KERNEL_OFFSET 0xffffffffc0000000ull

#define KERNEL_DATA ((const kernel_data_t*)0xffffffffc0007000)



typedef struct __attribute__((packed)) _KERNEL_DATA{
	u16 mmap_size;
	u8 _padding[6];
	struct{
		u64 base;
		u64 length;
		u32 type;
		u8 _padding[4];
	} mmap[42];
} kernel_data_t;



extern u64 __KERNEL_START__[1];
extern u64 __KERNEL_CORE_END__[1];
extern u64 __KERNEL_CPU_LOCAL_START__[1];
extern u64 __KERNEL_CPU_LOCAL_END__[1];
extern u64 __KERNEL_COMMON_START__[1];
extern u64 __KERNEL_END__[1];
extern u64 __KERNEL_BSS_START__[1];
extern u64 __KERNEL_BSS_END__[1];

extern const u64 __core_version;



static inline u64 KERNEL_CORE_CODE KERNEL_NOCOVERAGE kernel_get_version(void){
	return __core_version;
}



static inline u64 KERNEL_CORE_CODE KERNEL_NOCOVERAGE kernel_get_start(void){
	return (u64)__KERNEL_START__;
}



static inline u64 KERNEL_CORE_CODE KERNEL_NOCOVERAGE kernel_get_core_end(void){
	return (u64)__KERNEL_CORE_END__;
}



static inline u64 KERNEL_CORE_CODE KERNEL_NOCOVERAGE kernel_get_cpu_local_start(void){
	return (u64)__KERNEL_CPU_LOCAL_START__;
}



static inline u64 KERNEL_CORE_CODE KERNEL_NOCOVERAGE kernel_get_cpu_local_end(void){
	return (u64)__KERNEL_CPU_LOCAL_END__;
}



static inline u64 KERNEL_CORE_CODE KERNEL_NOCOVERAGE kernel_get_common_start(void){
	return (u64)__KERNEL_COMMON_START__;
}



static inline u64 KERNEL_CORE_CODE KERNEL_NOCOVERAGE kernel_get_end(void){
	return (u64)__KERNEL_END__;
}



static inline u64 KERNEL_CORE_CODE KERNEL_NOCOVERAGE kernel_get_bss_start(void){
	return (u64)__KERNEL_BSS_START__;
}



static inline u64 KERNEL_CORE_CODE KERNEL_NOCOVERAGE kernel_get_bss_end(void){
	return (u64)__KERNEL_BSS_END__;
}



static inline u64 KERNEL_CORE_CODE KERNEL_NOCOVERAGE kernel_get_offset(void){
	return KERNEL_OFFSET;
}



void kernel_init(void);



void kernel_load(void);



#endif
