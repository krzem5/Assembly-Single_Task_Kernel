#include <kernel/acl/acl.h>
#include <kernel/clock/clock.h>
#include <kernel/cpu/cpu.h>
#include <kernel/id/group.h>
#include <kernel/id/user.h>
#include <kernel/isr/isr.h>
#include <kernel/kernel.h>
#include <kernel/lock/profiling.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/module/module.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/serial/serial.h>
#include <kernel/symbol/symbol.h>
#include <kernel/time/time.h>
#include <kernel/timer/timer.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "main"



static void _main_thread(void){
	LOG("Main thread started");
	kernel_execute_initializers();
	kernel_adjust_memory_flags_after_init();
	module_load("os_loader");
}



void KERNEL_NORETURN KERNEL_NOCOVERAGE KERNEL_EARLY_EXEC main(const kernel_data_t* bootloader_kernel_data){
	serial_init();
	cpu_check_features();
	LOG("Starting kernel...");
	kernel_init(bootloader_kernel_data);
	pmm_init();
	vmm_init();
	pmm_init_high_mem();
	kernel_adjust_memory_flags();
	cpu_init_early_header();
	amm_init();
	acl_init();
	kernel_early_execute_initializers();
	cpu_start_all_cores();
	__lock_profiling_enable_dependency_graph();
	thread_create_kernel_thread(NULL,"main",_main_thread,0x200000,0);
	scheduler_enable();
	scheduler_yield();
	scheduler_task_wait_loop();
}
