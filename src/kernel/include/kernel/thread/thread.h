#ifndef _KERNEL_THREAD_THREAD_H_
#define _KERNEL_THREAD_THREAD_H_ 1
#include <kernel/isr/isr.h>
#include <kernel/lock/lock.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>



typedef struct _THREAD_REGISTER_STATE{
	u64 cr3;
	u64 es;
	u64 ds;
	u64 rax;
	u64 rbx;
	u64 rcx;
	u64 rdx;
	u64 rsi;
	u64 rdi;
	u64 rbp;
	u64 r8;
	u64 r9;
	u64 r10;
	u64 r11;
	u64 r12;
	u64 r13;
	u64 r14;
	u64 r15;
} thread_register_state_t;



typedef struct _THREAD{
	lock_t lock;
	vmm_pagemap_t* pagemap;
	thread_register_state_t state;
	u64 fs_base;
	u64 fs_base;
} thread_t;



#endif
