#ifndef _KERNEL_TYPES_H_
#define _KERNEL_TYPES_H_ 1



#define KERNEL_CORE_CODE __attribute__((section(".ctext")))
#define KERNEL_CORE_DATA __attribute__((section(".cdata")))
#define KERNEL_CORE_RDATA __attribute__((section(".crdata")))
#define KERNEL_CORE_BSS __attribute__((section(".cbss")))
#define KERNEL_NORETURN __attribute__((noreturn))
#define KERNEL_NOOPT __attribute__((noipa,optimize("O0")))



typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long long int u64;

typedef signed char s8;
typedef signed short int s16;
typedef signed int s32;
typedef signed long long int s64;



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



#endif
