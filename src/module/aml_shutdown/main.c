#include <aml/namespace.h>
#include <aml/object.h>
#include <aml/runtime.h>
#include <kernel/acpi/fadt.h>
#include <kernel/io/io.h>
#include <kernel/module/module.h>
#include <kernel/shutdown/shutdown.h>
#include <kernel/util/util.h>



// PM1x flags
#define SLP_TYP_SHIFT 10
#define SLP_EN 0x2000



static void _aml_shutdown_function(void){
	aml_namespace_t* s5_package=aml_namespace_lookup(NULL,"\\_S5_",0);
	if (!s5_package||!s5_package->value||s5_package->value->type!=AML_OBJECT_TYPE_PACKAGE||s5_package->value->package.length<2){
		return;
	}
	u16 pm1a_value=(s5_package->value->package.data[0]->integer<<SLP_TYP_SHIFT)|SLP_EN;
	u16 pm1b_value=(s5_package->value->package.data[1]->integer<<SLP_TYP_SHIFT)|SLP_EN;
	aml_namespace_t* tts_method=aml_namespace_lookup(NULL,"\\_TTS",0);
	if (tts_method&&tts_method->value&&tts_method->value->type==AML_OBJECT_TYPE_METHOD){
		aml_object_t* arg0=aml_object_alloc_integer(5);
		aml_object_dealloc(aml_runtime_execute_method(tts_method->value,1,&arg0));
		aml_object_dealloc(arg0);
	}
	aml_namespace_t* pts_method=aml_namespace_lookup(NULL,"\\_PTS",0);
	if (pts_method&&pts_method->value&&pts_method->value->type==AML_OBJECT_TYPE_METHOD){
		aml_object_t* arg0=aml_object_alloc_integer(5);
		aml_object_dealloc(aml_runtime_execute_method(pts_method->value,1,&arg0));
		aml_object_dealloc(arg0);
	}
	io_port_out16(acpi_fadt->pm1a_control_block,pm1a_value);
	if (acpi_fadt->pm1b_control_block){
		io_port_out16(acpi_fadt->pm1b_control_block,pm1b_value);
	}
	for (;;);
}



static _Bool _init(module_t* module){
	aml_namespace_t* s5_package=aml_namespace_lookup(NULL,"\\_S5_",0);
	if (!s5_package||!s5_package->value){
		return 0;
	}
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
