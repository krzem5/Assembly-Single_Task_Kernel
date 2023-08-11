#include <kernel/acpi/acpi.h>
#include <kernel/bios/bios.h>
#include <kernel/clock/clock.h>
#include <kernel/cpu/cpu.h>
#include <kernel/drive/drive_list.h>
#include <kernel/driver/ahci.h>
#include <kernel/driver/ata.h>
#include <kernel/driver/i82540.h>
#include <kernel/driver/nvme.h>
#include <kernel/fd/fd.h>
#include <kernel/idt/idt.h>
#include <kernel/isr/isr.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/mmap/mmap.h>
#include <kernel/network/layer1.h>
#include <kernel/network/layer2.h>
#include <kernel/network/layer3.h>
#include <kernel/partition/partition.h>
#include <kernel/pci/pci.h>
#include <kernel/random/random.h>
#include <kernel/serial/serial.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "main"



void KERNEL_CORE_CODE KERNEL_NORETURN main(void){
	cpu_check_features();
	LOG_CORE("Starting kernel...");
	const kernel_data_t* kernel_data=kernel_init();
	pmm_init(kernel_data);
	vmm_init(kernel_data);
	pmm_init_high_mem(kernel_data);
	driver_ahci_init();
	driver_ata_init();
	driver_i82540_init();
	driver_nvme_init();
	partition_init();
	drive_list_init();
	network_layer1_init();
	pci_init();
	drive_list_load_partitions();
	kernel_load();
	// From this point onwards all kernel functions can be used
	vmm_set_common_kernel_pagemap();
	clock_init();
	fd_init();
	mmap_init();
	syscall_init();
	idt_init();
	isr_init();
	acpi_load();
	bios_get_system_data();
	network_layer1_init_irq();
	network_layer2_init();
	network_layer3_init();
	random_init();
	serial_init_irq();
	cpu_start_all_cores();
}
