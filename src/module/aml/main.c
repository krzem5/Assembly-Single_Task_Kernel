#include <kernel/module/module.h>



static _Bool _init(module_t* module){
	// // PM1x flags
	// #define SLP_TYP_SHIFT 10
	// #define SLP_EN 0x2000
	// aml_runtime_init(aml_parse(acpi_dsdt->data,acpi_dsdt->header.length-sizeof(acpi_dsdt_t)),fadt->sci_int);
	// asm volatile("cli":::"memory");
	// u16 pm1a_value=(aml_runtime_get_node(NULL,"\\_S5_[0]")->data.integer<<SLP_TYP_SHIFT)|SLP_EN;
	// u16 pm1b_value=(aml_runtime_get_node(NULL,"\\_S5_[1]")->data.integer<<SLP_TYP_SHIFT)|SLP_EN;
	// io_port_out16(acpi_fadt->pm1a_control_block,pm1a_value);
	// if (acpi_fadt->pm1b_control_block){
	// 	io_port_out16(acpi_fadt->pm1b_control_block,pm1b_value);
	// }
	// for (;;);
	return 0;
}



static void _deinit(module_t* module){
	return;
}



MODULE_DECLARE(
	_init,
	_deinit,
	0
);
