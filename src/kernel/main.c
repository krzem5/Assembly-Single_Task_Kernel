#include <kernel/acpi/acpi.h>
#include <kernel/aml/bus.h>
#include <kernel/aml/runtime.h>
#include <kernel/bios/bios.h>
#include <kernel/clock/clock.h>
#include <kernel/cpu/cpu.h>
#include <kernel/elf/elf.h>
#include <kernel/handle/handle.h>
#include <kernel/isr/isr.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/umm.h>
#include <kernel/memory/vmm.h>
#include <kernel/network/layer1.h>
#include <kernel/network/layer2.h>
#include <kernel/network/layer3.h>
#include <kernel/partition/partition.h>
#include <kernel/pci/pci.h>
#include <kernel/random/random.h>
#include <kernel/sandbox/sandbox.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/serial/serial.h>
#include <kernel/types.h>
#include <kernel/user/data.h>
#define KERNEL_LOG_NAME "main"



void KERNEL_ENTRY_CODE KERNEL_NORETURN KERNEL_NOCOVERAGE main(void){
	cpu_check_features();
	LOG_CORE("Starting kernel...");
	kernel_init();
	pmm_init();
	vmm_init();
	pmm_init_high_mem();
	kmm_init();
	pci_enumerate(1);
	partition_load();
	kernel_load();
	// From this point onwards all kernel functions can be used
	handle_init();
	sandbox_init();
	isr_init();
	pci_enumerate(0);
	clock_init();
	acpi_load();
	aml_runtime_init_irq();
	aml_bus_enumerate();
	bios_get_system_data();
	network_layer1_init_irq();
	network_layer2_init();
	network_layer3_init();
	random_init();
	serial_init_irq();
	user_data_generate();
	scheduler_init();
	cpu_start_all_cores();
	elf_load("/kernel/loader.elf");
	scheduler_enable();
	scheduler_start();
	for (;;);
}
