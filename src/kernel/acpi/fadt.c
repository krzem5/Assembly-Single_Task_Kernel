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



const acpi_fadt_t* KERNEL_INIT_WRITE acpi_fadt=NULL;
const acpi_dsdt_t* KERNEL_INIT_WRITE acpi_dsdt=NULL;



void KERNEL_EARLY_EXEC acpi_fadt_load(const acpi_fadt_t* fadt){
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
	INFO("Found DSDT at %p",fadt->dsdt);
	acpi_fadt=fadt;
	acpi_dsdt=(void*)vmm_identity_map(fadt->dsdt,((const acpi_dsdt_t*)vmm_identity_map(fadt->dsdt,sizeof(acpi_dsdt_t)))->header.length);
	aml_runtime_init(aml_parse(acpi_dsdt->data,acpi_dsdt->header.length-sizeof(acpi_dsdt_t)),fadt->sci_int);
}



KERNEL_PUBLIC void KERNEL_NORETURN KERNEL_NOCOVERAGE acpi_fadt_shutdown(_Bool restart){
	asm volatile("cli":::"memory");
	u16 pm1a_value=(aml_runtime_get_node(NULL,"\\_S5_[0]")->data.integer<<SLP_TYP_SHIFT)|SLP_EN;
	u16 pm1b_value=(aml_runtime_get_node(NULL,"\\_S5_[1]")->data.integer<<SLP_TYP_SHIFT)|SLP_EN;
	io_port_out16(acpi_fadt->pm1a_control_block,pm1a_value);
	if (acpi_fadt->pm1b_control_block){
		io_port_out16(acpi_fadt->pm1b_control_block,pm1b_value);
	}
	for (;;);
}
