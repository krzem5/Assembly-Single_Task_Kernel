#include <kernel/cpu/cpu.h>
#include <kernel/io/io.h>
#include <kernel/lock/bitlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/vmm.h>
#include <kernel/symbol/symbol.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



static u32 _panic_bitlock=0;



KERNEL_PUBLIC void KERNEL_NORETURN KERNEL_NOCOVERAGE panic(const char* error){
	if (!bitlock_try_acquire(&_panic_bitlock,0)){
		for (;;);
	}
	log_direct("\x1b[1m\x1b[1m\x1b[38;2;192;28;40mFatal error: %s\x1b[0m\n",error);
	u64 rip=(u64)panic;
	u64 rbp=(u64)__builtin_frame_address(0);
	while (1){
		const symbol_t* symbol=symbol_lookup(rip);
		if (symbol){
			log_direct("\x1b[1m\x1b[1m\x1b[38;2;192;28;40m[%u] %s:%s+%u\x1b[0m\n",CPU_HEADER_DATA->index,symbol->module,symbol->name->data,rip-symbol->rb_node.key);
		}
		else{
			log_direct("\x1b[1m\x1b[1m\x1b[38;2;192;28;40m[%u] %p\x1b[0m\n",CPU_HEADER_DATA->index,rip);
		}
		if (!rbp){
			break;
		}
		if (!vmm_virtual_to_physical(&vmm_kernel_pagemap,rbp)||!vmm_virtual_to_physical(&vmm_kernel_pagemap,rbp+8)){
			log_direct("\x1b[1m\x1b[1m\x1b[38;2;192;28;40m[%u] <rbp: %p>\x1b[0m\n",CPU_HEADER_DATA->index,rbp);
			break;
		}
		rip=*((u64*)(rbp+8));
		rbp=*((u64*)rbp);
	}
	io_port_out16(0x604,0x2000);
	for (;;);
}
