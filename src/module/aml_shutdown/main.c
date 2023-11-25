#include <aml/namespace.h>
#include <aml/object.h>
#include <kernel/acpi/fadt.h>
#include <kernel/io/io.h>
#include <kernel/module/module.h>
#include <kernel/shutdown/shutdown.h>
#include <kernel/util/util.h>



// PM1x flags
#define SLP_TYP_SHIFT 10
#define SLP_EN 0x2000



static u16 _aml_shutdown_pm1a_value;
static u16 _aml_shutdown_pm1b_value;



static void _aml_shutdown_function(void){
	io_port_out16(acpi_fadt->pm1a_control_block,(_aml_shutdown_pm1a_value<<SLP_TYP_SHIFT)|SLP_EN);
	if (acpi_fadt->pm1b_control_block){
		io_port_out16(acpi_fadt->pm1b_control_block,(_aml_shutdown_pm1b_value<<SLP_TYP_SHIFT)|SLP_EN);
	}
	for (;;);
}



static _Bool _init(module_t* module){
	aml_namespace_t* s5_package=aml_namespace_lookup(NULL,"\\_S5_",0);
	if (!s5_package||!s5_package->value||s5_package->value->type!=AML_OBJECT_TYPE_PACKAGE||s5_package->value->package.length<2){
		return 0;
	}
	_aml_shutdown_pm1a_value=s5_package->value->package.data[0]->integer;
	_aml_shutdown_pm1b_value=s5_package->value->package.data[1]->integer;
	shutdown_register_shutdown_function(_aml_shutdown_function,1);
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
