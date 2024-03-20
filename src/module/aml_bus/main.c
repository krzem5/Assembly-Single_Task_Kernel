#include <kernel/aml/namespace.h>
#include <kernel/aml/object.h>
#include <kernel/aml/runtime.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/module/module.h>
#define KERNEL_LOG_NAME "aml_bus"



#define AML_BUS_ADDRESS_TYPE_ADR 0
#define AML_BUS_ADDRESS_TYPE_HID 1
#define AML_BUS_ADDRESS_TYPE_HID_STR 2

#define AML_BUS_UID_TYPE_INT 0
#define AML_BUS_UID_TYPE_STR 1

#define AML_BUS_DEVICE_SLOT_UNKNOWN 0xffffffffffffffffull



typedef struct _AML_BUS_DEVICE{
	u8 address_type;
	u8 uid_type;
	union{
		u64 adr;
		u64 hid;
		string_t* hid_str;
	} address;
	handle_t handle;
	aml_namespace_t* device;
	struct _AML_BUS_DEVICE* parent;
	u64 slot_unique_id;
	union{
		u64 uid;
		string_t* uid_str;
	};
} aml_bus_device_t;



static omm_allocator_t* _aml_bus_device_allocator=NULL;

KERNEL_PUBLIC handle_type_t aml_bus_device_handle_type=0;



KERNEL_PUBLIC _Bool aml_bus_device_get_crs(aml_bus_device_t* device){
	aml_namespace_t* crs_object=aml_namespace_lookup(device->device,"_CRS",AML_NAMESPACE_LOOKUP_FLAG_LOCAL);
	if (!crs_object||!crs_object->value){
		return 0;
	}
	aml_object_t* value=aml_runtime_execute_method(crs_object->value,0,NULL);
	if (!value){
		return 0;
	}
	if (value->type!=AML_OBJECT_TYPE_BUFFER){
		ERROR("_CRS object is not a buffer");
		aml_object_dealloc(value);
		return 0;
	}
	for (u32 i=0;i<value->buffer.size;){
		u8 type=value->buffer.data[i];
		i++;
		u16 length=0;
		if (type>>7){
			length=value->buffer.data[i]|(value->buffer.data[i+1]<<8);
			i+=2;
		}
		else{
			length=type&7;
			type>>=3;
		}
		if (type==0x0f){
			break;
		}
		switch (type){
			case 0x04:
				INFO("IRQ Format Descriptor");
				break;
			case 0x05:
				INFO("DMA Format Descriptor");
				break;
			case 0x06:
				INFO("Start Dependent Functions Descriptor");
				break;
			case 0x07:
				INFO("End Dependent Functions Descriptor");
				break;
			case 0x08:
				INFO("I/O Port Descriptor");
				break;
			case 0x09:
				INFO("Fixed Location I/O Port Descriptor");
				break;
			case 0x0a:
				INFO("Fixed DMA Descriptor");
				break;
			case 0x0e:
				INFO("Vendor Defined Descriptor");
				break;
			case 0x81:
				INFO("24-Bit Memory Range Descriptor");
				break;
			case 0x82:
				INFO("Generic Register Descriptor");
				break;
			case 0x84:
				INFO("Vendor-Defined Descriptor");
				break;
			case 0x85:
				INFO("32-Bit Memory Range Descriptor");
				break;
			case 0x86:
				INFO("32-Bit Fixed Memory Range Descriptor: %p - %p (%s)",*((const u32*)(value->buffer.data+i+1)),*((const u32*)(value->buffer.data+i+5)),((value->buffer.data[0]&1)?"RW":"RD"));
				break;
			case 0x87:
				INFO("Address Space Resource Descriptors");
				break;
			case 0x88:
				INFO("Word Address Space Descriptor");
				break;
			case 0x89:
				INFO("Extended Interrupt Descriptor");
				break;
			case 0x8a:
				INFO("QWord Address Space Descriptor");
				break;
			case 0x8b:
				INFO("Extended Address Space Descriptor");
				break;
			case 0x8c:
				INFO("GPIO Connection Descriptor");
				break;
			case 0x8d:
				INFO("Pin Function Descriptor");
				break;
			case 0x8e:
				INFO("GenericSerialBus Connection Descriptors");
				break;
			case 0x8f:
				INFO("Pin Configuration Descriptor");
				break;
			case 0x90:
				INFO("Pin Group Descriptor");
				break;
			case 0x91:
				INFO("Pin Group Function Descriptor");
				break;
			case 0x92:
				INFO("Pin Group Configuration Descriptor");
				break;
			default:
				ERROR("Unrecognized _CRS tag: %X [%u bytes]",type,length);
				break;
		}
		i+=length;
	}
	aml_object_dealloc(value);
	return 1;
}



static aml_bus_device_t* _parse_device_descriptor(aml_namespace_t* device,aml_bus_device_t* out){
	out->uid_type=AML_BUS_UID_TYPE_INT;
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
	aml_namespace_t* uid_object=aml_namespace_lookup(device,"_UID",AML_NAMESPACE_LOOKUP_FLAG_LOCAL);
	if (uid_object&&uid_object->value){
		aml_object_t* value=aml_runtime_execute_method(uid_object->value,0,NULL);
		if (!value){
			goto _cleanup;
		}
		if (value->type!=AML_OBJECT_TYPE_INTEGER&&value->type!=AML_OBJECT_TYPE_STRING){
			ERROR("_UID object is not an integer or string");
			aml_object_dealloc(value);
			goto _cleanup;
		}
		if (value->type==AML_OBJECT_TYPE_INTEGER){
			out->uid_type=AML_BUS_UID_TYPE_INT;
			out->uid=value->integer;
		}
		else{
			out->uid_type=AML_BUS_UID_TYPE_STR;
			out->uid_str=smm_duplicate(value->string);
		}
		aml_object_dealloc(value);
	}
	handle_new(out,aml_bus_device_handle_type,&(out->handle));
	handle_finish_setup(&(out->handle));
	aml_bus_device_get_crs(out);
	return out;
_cleanup:
	if (out->address_type==AML_BUS_ADDRESS_TYPE_HID_STR){
		smm_dealloc(out->address.hid_str);
	}
	if (out->uid_type==AML_BUS_UID_TYPE_STR){
		smm_dealloc(out->uid_str);
	}
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
		aml_namespace_t* sta_object=aml_namespace_lookup(device,"_STA",AML_NAMESPACE_LOOKUP_FLAG_LOCAL);
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
	spinlock_init(&(_aml_bus_device_allocator->lock));
	aml_bus_device_handle_type=handle_alloc("aml_bus_device",NULL);
	_enumerate_system_bus(system_bus,NULL);
	// WARN("%p",*((u32*)(vmm_identity_map(0x00000000fed40000,0x0000000000005000)+0x0014)));
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
