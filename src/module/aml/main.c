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
		aml_runtime_register_irq(acpi_fadt->sci_int);
		aml_runtime_context_t ctx={
			acpi_dsdt->data,
			acpi_dsdt->header.length-sizeof(acpi_dsdt_t),
			NULL,
			NULL
		};
		aml_runtime_execute(&ctx);
	}
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
