#include <aml/runtime.h>
#include <kernel/acpi/fadt.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#define KERNEL_LOG_NAME "aml"



static _Bool _init(module_t* module){
	if (!acpi_dsdt||!acpi_dsdt->header.length){
		WARN("No AML code present");
	}
	else{
		aml_runtime_context_t ctx={
			acpi_dsdt->data,
			acpi_dsdt->header.length-sizeof(acpi_dsdt_t),
			NULL
		};
		aml_runtime_execute(&ctx);
		// panic("test");
	}
	// INFO("Registering AML IRQ...");
	// ioapic_redirect_irq(fadt->sci_int,isr_allocate());
	// // PM1x flags
	// #define SLP_TYP_SHIFT 10
	// #define SLP_EN 0x2000
	// u16 pm1a_value=(aml_runtime_get_node(NULL,"\\_S5_[0]")->data.integer<<SLP_TYP_SHIFT)|SLP_EN;
	// u16 pm1b_value=(aml_runtime_get_node(NULL,"\\_S5_[1]")->data.integer<<SLP_TYP_SHIFT)|SLP_EN;
	// io_port_out16(acpi_fadt->pm1a_control_block,pm1a_value);
	// if (acpi_fadt->pm1b_control_block){
	// 	io_port_out16(acpi_fadt->pm1b_control_block,pm1b_value);
	// }
	return 1;
}



static void _deinit(module_t* module){
	return;
}



MODULE_DECLARE(
	_init,
	_deinit,
	0
);
