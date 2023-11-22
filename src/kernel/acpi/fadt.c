#include <kernel/acpi/fadt.h>
#include <kernel/acpi/structures.h>
#include <kernel/aml/parser.h>
#include <kernel/aml/runtime.h>
#include <kernel/io/io.h>
#include <kernel/log/log.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "fadt"



// PM1x flags
#define SCI_EN 0x0001
#define SLP_TYP_SHIFT 10
#define SLP_EN 0x2000



static u32 KERNEL_INIT_WRITE _fadt_pm1a_control_block;
static u32 KERNEL_INIT_WRITE _fadt_pm1b_control_block;



void acpi_fadt_load(const acpi_fadt_t* fadt){
	LOG("Loading FADT...");
	LOG("Enabling ACPI...");
	if (io_port_in16(fadt->pm1a_control_block)&SCI_EN){
		INFO("ACPI already enabled");
	}
	else{
		io_port_out8(fadt->smi_command_port,fadt->acpi_enable);
		SPINLOOP(!(io_port_in16(fadt->pm1a_control_block)&SCI_EN));
		if (fadt->pm1b_control_block){
			SPINLOOP(!(io_port_in16(fadt->pm1b_control_block)&SCI_EN));
		}
	}
	_fadt_pm1a_control_block=fadt->pm1a_control_block;
	_fadt_pm1b_control_block=fadt->pm1b_control_block;
	INFO("Found DSDT at %p",fadt->dsdt);
	const acpi_dsdt_t* dsdt=(void*)(u64)(fadt->dsdt);
	dsdt=(void*)vmm_identity_map((u64)dsdt,((const acpi_dsdt_t*)vmm_identity_map((u64)dsdt,sizeof(acpi_dsdt_t)))->header.length);
	aml_runtime_init(aml_parse(dsdt->data,dsdt->header.length-sizeof(acpi_dsdt_t)),fadt->sci_int);
}



KERNEL_PUBLIC void KERNEL_NORETURN KERNEL_NOCOVERAGE acpi_fadt_shutdown(_Bool restart){
	asm volatile("cli":::"memory");
	if (restart){
		_acpi_fadt_reboot();
	}
	u16 pm1a_value=(aml_runtime_get_node(NULL,"\\_S5_[0]")->data.integer<<SLP_TYP_SHIFT)|SLP_EN;
	u16 pm1b_value=(aml_runtime_get_node(NULL,"\\_S5_[1]")->data.integer<<SLP_TYP_SHIFT)|SLP_EN;
	io_port_out16(_fadt_pm1a_control_block,pm1a_value);
	if (_fadt_pm1b_control_block){
		io_port_out16(_fadt_pm1b_control_block,pm1b_value);
	}
	for (;;);
}
