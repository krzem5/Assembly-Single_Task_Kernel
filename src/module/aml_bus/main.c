#include <aml/namespace.h>
#include <aml/object.h>
#include <aml/runtime.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#define KERNEL_LOG_NAME "aml_bus"



static void _enumerate_system_bus(aml_namespace_t* bus){
	for (aml_namespace_t* device=bus->first_child;device;device=device->next_sibling){
		if (!device->value||device->value->type!=AML_OBJECT_TYPE_DEVICE){
			continue;
		}
		u64 status=0xf;
		aml_namespace_t* sta_object=aml_namespace_lookup(device,"_STA",0);
		if (sta_object&&sta_object->value){
			aml_object_t* value=aml_runtime_execute_method(sta_object->value,0,NULL);
			if (!value){
				continue;
			}
			if (value->type!=AML_OBJECT_TYPE_INTEGER){
				ERROR("_enumerate_system_bus: _STA: value is not an integer");
				aml_object_dealloc(value);
				continue;
			}
			status=value->integer;
			aml_object_dealloc(value);
		}
		if (!(status&8)){
			continue;
		}
		WARN("%s: %x",device->name,status);
		// if (status&1){
		// 	_parse_device(device);
		// }
		_enumerate_system_bus(device);
	}
}



static _Bool _init(module_t* module){
	aml_namespace_t* system_bus=aml_namespace_lookup(NULL,"\\_SB_",0);
	if (!system_bus){
		return 0;
	}
	_enumerate_system_bus(system_bus);
	for (;;);
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
