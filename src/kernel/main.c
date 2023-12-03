#include <kernel/acpi/acpi.h>
#include <kernel/bios/bios.h>
#include <kernel/clock/clock.h>
#include <kernel/cpu/cpu.h>
#include <kernel/id/group.h>
#include <kernel/id/user.h>
#include <kernel/initramfs/initramfs.h>
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
#include <kernel/network/layer1.h>
#include <kernel/network/layer2.h>
#include <kernel/pci/pci.h>
#include <kernel/random/random.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/serial/serial.h>
#include <kernel/shutdown/shutdown.h>
#include <kernel/socket/socket.h>
#include <kernel/symbol/symbol.h>
#include <kernel/time/time.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "main"



static void _main_thread(void){
	LOG("Main thread started");
	shutdown_init();
	bios_get_system_data();
	initramfs_init();
	pci_enumerate();
	random_init();
	socket_init();
	network_layer1_init();
	network_layer2_init();
	serial_init_irq();
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
	amm_init();
	symbol_init();
	clock_init();
	time_init();
	isr_init();
	acpi_load();
	gid_init();
	uid_init();
	scheduler_init();
	process_init();
	cpu_start_all_cores();
	thread_new_kernel_thread(NULL,_main_thread,0x200000,0);
	scheduler_enable();
	scheduler_yield();
	scheduler_task_wait_loop();
}
