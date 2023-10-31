#include <kernel/acpi/acpi.h>
#include <kernel/aml/bus.h>
#include <kernel/bios/bios.h>
#include <kernel/clock/clock.h>
#include <kernel/cpu/cpu.h>
#include <kernel/elf/elf.h>
#include <kernel/handle/handle.h>
#include <kernel/initramfs/initramfs.h>
#include <kernel/isr/isr.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/module/module.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/network/layer2.h>
#include <kernel/pci/pci.h>
#include <kernel/random/random.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/serial/serial.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "main"



static void _main_thread(void){
	LOG("Main thread started");
	bios_get_system_data();
	initramfs_init();
	pci_enumerate();
	aml_bus_enumerate();
	network_layer2_init();
	random_init();
	serial_init_irq();
	kernel_adjust_memory_flags_after_init();
	module_load(vfs_lookup(NULL,"/module/ahci.mod"));
	module_load(vfs_lookup(NULL,"/module/ata.mod"));
	module_load(vfs_lookup(NULL,"/module/gpt.mod"));
	module_load(vfs_lookup(NULL,"/module/i82540.mod"));
	module_load(vfs_lookup(NULL,"/module/iso9660.mod"));
	module_load(vfs_lookup(NULL,"/module/kfs2.mod"));
	module_load(vfs_lookup(NULL,"/module/nvme.mod"));
	module_load(vfs_lookup(NULL,"/module/usb_msc.mod"));
	module_load(vfs_lookup(NULL,"/module/xhci.mod"));
#if KERNEL_COVERAGE_ENABLED
	module_load(vfs_lookup(NULL,"/module/coverage.mod"));
#endif
	if (!elf_load(vfs_lookup(NULL,"/boot/boot.elf"))){
		panic("Unable to load boot file");
	}
}



void KERNEL_NORETURN KERNEL_NOCOVERAGE main(const kernel_data_t* bootloader_kernel_data){
	serial_init();
	cpu_check_features();
	LOG("Starting kernel...");
	kernel_init(bootloader_kernel_data);
	handle_init();
	pmm_init();
	vmm_init();
	pmm_init_high_mem();
	kernel_adjust_memory_flags();
	clock_init();
	isr_init();
	acpi_load();
	scheduler_init();
	process_init();
	cpu_start_all_cores();
	scheduler_enqueue_thread(thread_new_kernel_thread(process_kernel,(u64)_main_thread,0x200000,0));
	scheduler_enable();
	scheduler_start();
	for (;;);
}
