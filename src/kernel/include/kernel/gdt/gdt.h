#ifndef _KERNEL_GDT_GDT_H_
#define _KERNEL_GDT_GDT_H_ 1
#include <kernel/types.h>



typedef struct __attribute__((packed)) _TSS{
	u8 _padding[4];
	u64 rsp0;
	u64 rsp1;
	u64 rsp2;
	u8 _padding1[8];
	u64 ist1;
	u64 ist2;
	u64 ist3;
	u64 ist4;
	u64 ist5;
	u64 ist6;
	u64 ist7;
	u8 _padding2[8];
	u32 iopb;
} tss_t;



void gdt_enable(void* tss);



#endif
