#include <aml/namespace.h>
#include <aml/object.h>
#include <aml/runtime.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/module/module.h>
#define KERNEL_LOG_NAME "aml_bus"



#define AML_BUS_ADDRESS_TYPE_ADR 0
#define AML_BUS_ADDRESS_TYPE_HID 1
#define AML_BUS_ADDRESS_TYPE_HID_STR 2

#define AML_BUS_DEVICE_SLOT_UNKNOWN 0xffffffffffffffffull



typedef struct _AML_BUS_DEVICE{
	u8 address_type;
	union{
		u64 adr;
		u64 hid;
		string_t* hid_str;
	} address;
	aml_namespace_t* device;
	struct _AML_BUS_DEVICE* parent;
	u64 slot_unique_id;
	string_t* uid;
} aml_bus_device_t;



static omm_allocator_t* _aml_bus_device_allocator=NULL;



static aml_bus_device_t* _parse_device_descriptor(aml_namespace_t* device,aml_bus_device_t* out){
	out->device=device;
	out->parent=NULL;
	out->slot_unique_id=AML_BUS_DEVICE_SLOT_UNKNOWN;
	aml_namespace_t* sun_object=aml_namespace_lookup(device,"_SUN",AML_NAMESPACE_LOOKUP_FLAG_LOCAL);
	if (sun_object&&sun_object->value){
		aml_object_t* value=aml_runtime_execute_method(sun_object->value,0,NULL);
		if (!value){
			goto _cleanup;
		}
		if (value->type!=AML_OBJECT_TYPE_INTEGER){
			ERROR("_SUB object is not an integer");
			aml_object_dealloc(value);
			goto _cleanup;
		}
		out->slot_unique_id=value->integer;
		aml_object_dealloc(value);
	}
	return out;
_cleanup:
	omm_dealloc(_aml_bus_device_allocator,out);
	return NULL;
}



static aml_bus_device_t* _parse_augmented_device_descriptor(aml_namespace_t* device){
	aml_namespace_t* adr_object=aml_namespace_lookup(device,"_ADR",AML_NAMESPACE_LOOKUP_FLAG_LOCAL);
	if (!adr_object->value||adr_object->value->type!=AML_OBJECT_TYPE_INTEGER){
		ERROR("_ADR object is not an integer");
		return NULL;
	}
	aml_bus_device_t* out=omm_alloc(_aml_bus_device_allocator);
	out->address_type=AML_BUS_ADDRESS_TYPE_ADR;
	out->address.adr=adr_object->value->integer;
	return _parse_device_descriptor(device,out);
}



static aml_bus_device_t* _parse_full_device_descriptor(aml_namespace_t* device){
	aml_namespace_t* hid_object=aml_namespace_lookup(device,"_HID",AML_NAMESPACE_LOOKUP_FLAG_LOCAL);
	if (!hid_object->value||(hid_object->value->type!=AML_OBJECT_TYPE_INTEGER&&hid_object->value->type!=AML_OBJECT_TYPE_STRING)){
		ERROR("_HID object is not an integer or string");
		return NULL;
	}
	aml_bus_device_t* out=omm_alloc(_aml_bus_device_allocator);
	out->address_type=(hid_object->value->type==AML_OBJECT_TYPE_INTEGER?AML_BUS_ADDRESS_TYPE_HID:AML_BUS_ADDRESS_TYPE_HID_STR);
	if (out->address_type==AML_BUS_ADDRESS_TYPE_HID){
		out->address.hid=hid_object->value->integer;
	}
	else{
		out->address.hid_str=smm_duplicate(hid_object->value->string);
	}
	return _parse_device_descriptor(device,out);
}



static void _enumerate_system_bus(aml_namespace_t* bus,aml_bus_device_t* bus_aml_device){
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
		aml_bus_device_t* aml_device=NULL;
		if (status&1){
			if (aml_namespace_lookup(device,"_HID",AML_NAMESPACE_LOOKUP_FLAG_LOCAL)){
				aml_device=_parse_full_device_descriptor(device);
			}
			else if (aml_namespace_lookup(device,"_ADR",AML_NAMESPACE_LOOKUP_FLAG_LOCAL)){
				aml_device=_parse_augmented_device_descriptor(device);
			}
		}
		if (aml_device){
			aml_device->parent=bus_aml_device;
		}
		_enumerate_system_bus(device,aml_device);
	}
}



static _Bool _init(module_t* module){
	aml_namespace_t* system_bus=aml_namespace_lookup(NULL,"\\_SB_",0);
	if (!system_bus){
		return 0;
	}
	_aml_bus_device_allocator=omm_init("aml_bus_device",sizeof(aml_bus_device_t),8,2,pmm_alloc_counter("omm_aml_bus_device"));
	_enumerate_system_bus(system_bus,NULL);
	// for (;;);
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
