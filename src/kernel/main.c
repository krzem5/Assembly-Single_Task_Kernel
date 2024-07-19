#include <kernel/acl/acl.h>
#include <kernel/aslr/aslr.h>
#include <kernel/cpu/cpu.h>
#include <kernel/kernel.h>
#include <kernel/lock/profiling.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/module/module.h>
#include <kernel/mp/thread.h>
#include <kernel/random/random.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/serial/serial.h>
#include <kernel/signature/signature.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "main"



static KERNEL_AWAITS void _main_thread(void){
	LOG("Main thread started");
	kernel_execute_initializers();
	kernel_adjust_memory_flags_after_init();
	handle_release(&(module_load("module_loader",MODULE_LOAD_FLAG_ASYNC|MODULE_LOAD_FLAG_REQUIRED)->handle));
}



static void KERNEL_NORETURN KERNEL_EARLY_EXEC _main_relocated(void){
	pmm_init_high_memory();
	kernel_adjust_memory_flags();
	vmm_alloc_counter();
	cpu_init_early_header();
	amm_init();
	acl_init();
	kernel_early_execute_initializers();
	scheduler_enable();
	cpu_start_all_cores();
	lock_profiling_enable_dependency_graph();
	handle_release(&(thread_create_kernel_thread(NULL,"kernel.main",_main_thread,0)->handle));
	scheduler_yield();
	scheduler_task_wait_loop();
}



void KERNEL_NORETURN KERNEL_EARLY_EXEC main(const kernel_data_t* bootloader_kernel_data){
	serial_init();
	cpu_check_features();
	LOG("Starting kernel...");
	kernel_init(bootloader_kernel_data);
	pmm_init();
	vmm_init();
	signature_verify_kernel();
	random_init();
	aslr_reloc_kernel(_main_relocated);
}
