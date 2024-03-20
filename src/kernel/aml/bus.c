#include <kernel/aml/bus.h>
#include <kernel/aml/namespace.h>
#include <kernel/aml/object.h>
#include <kernel/aml/runtime.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/module/module.h>
#define KERNEL_LOG_NAME "aml_bus"



static omm_allocator_t* KERNEL_INIT_WRITE _aml_bus_device_allocator=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _aml_bus_device_resource_allocator=NULL;

KERNEL_PUBLIC handle_type_t KERNEL_INIT_WRITE aml_bus_device_handle_type=0;



static aml_bus_device_resource_t* _create_device_resource(aml_bus_device_t* device,u32 type){
	aml_bus_device_resource_t* out=omm_alloc(_aml_bus_device_resource_allocator);
	out->prev=device->resource_tail;
	out->next=NULL;
	out->type=type;
	if (device->resource_tail){
		device->resource_tail->next=out;
	}
	else{
		device->resource_head=out;
	}
	device->resource_tail=out;
	return out;
}



static _Bool _get_device_crs(aml_bus_device_t* device){
	device->resource_head=NULL;
	device->resource_tail=NULL;
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
				{
					u32 flags=0;
					if (length>=3){
						if (value->buffer.data[i+2]&1){
							flags|=AML_BUS_RESOURCE_INTERRUPT_FLAG_EDGE_TRIGGER;
						}
						if (value->buffer.data[i+2]&8){
							flags|=AML_BUS_RESOURCE_INTERRUPT_FLAG_ACTIVE_LOW;
						}
						if (value->buffer.data[i+2]&16){
							flags|=AML_BUS_RESOURCE_INTERRUPT_FLAG_SHARED;
						}
						if (value->buffer.data[i+2]&32){
							flags|=AML_BUS_RESOURCE_INTERRUPT_FLAG_WAKE_CAPALE;
						}
					}
					for (u16 mask=*((const u16*)(value->buffer.data+i));mask;mask&=mask-1){
						aml_bus_device_resource_t* res=_create_device_resource(device,AML_BUS_RESOURCE_TYPE_INTERRUPT);
						res->interrupt.pin=__builtin_ffs(mask)-1;
						res->interrupt.flags=flags;
					}
					break;
				}
			case 0x05:
				ERROR("DMA Format Descriptor");
				break;
			case 0x06:
				ERROR("Start Dependent Functions Descriptor");
				break;
			case 0x07:
				ERROR("End Dependent Functions Descriptor");
				break;
			case 0x08:
				{
					aml_bus_device_resource_t* res=_create_device_resource(device,AML_BUS_RESOURCE_TYPE_IO_PORT);
					res->io_port.base_min=*((const u16*)(value->buffer.data+i+1));
					res->io_port.base_max=*((const u16*)(value->buffer.data+i+3));
					res->io_port.base_alignment=value->buffer.data[i+5];
					res->io_port.length=value->buffer.data[i+6];
					res->io_port.decode_all_address_bits=!!(value->buffer.data[i]&1);
					break;
				}
			case 0x09:
				ERROR("Fixed Location I/O Port Descriptor");
				break;
			case 0x0a:
				ERROR("Fixed DMA Descriptor");
				break;
			case 0x0e:
				ERROR("Vendor Defined Descriptor");
				break;
			case 0x81:
				ERROR("24-Bit Memory Range Descriptor");
				break;
			case 0x82:
				ERROR("Generic Register Descriptor");
				break;
			case 0x84:
				ERROR("Vendor-Defined Descriptor");
				break;
			case 0x85:
				ERROR("32-Bit Memory Range Descriptor");
				break;
			case 0x86:
				{
					aml_bus_device_resource_t* res=_create_device_resource(device,AML_BUS_RESOURCE_TYPE_MEMORY_REGION);
					res->memory_region.base=*((const u32*)(value->buffer.data+i+1));
					res->memory_region.size=*((const u32*)(value->buffer.data+i+5));
					res->memory_region.writable=!!(value->buffer.data[i]&1);
					break;
				}
			case 0x87:
			case 0x88:
			case 0x8a:
				{
					aml_bus_device_resource_t* res=_create_device_resource(device,AML_BUS_RESOURCE_TYPE_ADDRESS_SPACE);
					res->address_space.type=value->buffer.data[i];
					res->address_space.flags=0;
					if (value->buffer.data[i+1]&4){
						res->address_space.flags|=AML_BUS_RESOURCE_ADDRESS_SPACE_FLAG_MIN_FIXED;
					}
					if (value->buffer.data[i+1]&8){
						res->address_space.flags|=AML_BUS_RESOURCE_ADDRESS_SPACE_FLAG_MAX_FIXED;
					}
					res->address_space.extra_flags=value->buffer.data[i+2];
					res->address_space.granularity=*((const u32*)(value->buffer.data+i+3));
					if (type==0x87){
						res->address_space.min=*((const u32*)(value->buffer.data+i+7));
						res->address_space.max=*((const u32*)(value->buffer.data+i+11));
						res->address_space.translation_offset=*((const u32*)(value->buffer.data+i+15));
						res->address_space.length=*((const u32*)(value->buffer.data+i+19));
					}
					else if (type==0x88){
						res->address_space.min=*((const u16*)(value->buffer.data+i+7));
						res->address_space.max=*((const u16*)(value->buffer.data+i+9));
						res->address_space.translation_offset=*((const u16*)(value->buffer.data+i+11));
						res->address_space.length=*((const u16*)(value->buffer.data+i+13));
					}
					else if (type==0x8a){
						res->address_space.min=*((const u64*)(value->buffer.data+i+7));
						res->address_space.max=*((const u64*)(value->buffer.data+i+15));
						res->address_space.translation_offset=*((const u64*)(value->buffer.data+i+23));
						res->address_space.length=*((const u64*)(value->buffer.data+i+31));
					}
					break;
				}
			case 0x89:
				{
					u32 flags=0;
					if (value->buffer.data[i]&2){
						flags|=AML_BUS_RESOURCE_INTERRUPT_FLAG_EDGE_TRIGGER;
					}
					if (value->buffer.data[i]&4){
						flags|=AML_BUS_RESOURCE_INTERRUPT_FLAG_ACTIVE_LOW;
					}
					if (value->buffer.data[i]&8){
						flags|=AML_BUS_RESOURCE_INTERRUPT_FLAG_SHARED;
					}
					if (value->buffer.data[i]&16){
						flags|=AML_BUS_RESOURCE_INTERRUPT_FLAG_WAKE_CAPALE;
					}
					for (u32 j=0;j<value->buffer.data[i+1];j++){
						aml_bus_device_resource_t* res=_create_device_resource(device,AML_BUS_RESOURCE_TYPE_INTERRUPT);
						res->interrupt.pin=*((const u32*)(value->buffer.data+i+j*4+2));
						res->interrupt.flags=flags;
					}
					break;
				}
			case 0x8b:
				ERROR("Extended Address Space Descriptor");
				break;
			case 0x8c:
				ERROR("GPIO Connection Descriptor");
				break;
			case 0x8d:
				ERROR("Pin Function Descriptor");
				break;
			case 0x8e:
				ERROR("GenericSerialBus Connection Descriptors");
				break;
			case 0x8f:
				ERROR("Pin Configuration Descriptor");
				break;
			case 0x90:
				ERROR("Pin Group Descriptor");
				break;
			case 0x91:
				ERROR("Pin Group Function Descriptor");
				break;
			case 0x92:
				ERROR("Pin Group Configuration Descriptor");
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
	_get_device_crs(out);
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



void aml_bus_scan(void){
	LOG("Scanning AML System Bus...");
	aml_namespace_t* system_bus=aml_namespace_lookup(NULL,"\\_SB_",0);
	if (!system_bus){
		return;
	}
	_aml_bus_device_allocator=omm_init("aml_bus_device",sizeof(aml_bus_device_t),8,2,pmm_alloc_counter("omm_aml_bus_device"));
	spinlock_init(&(_aml_bus_device_allocator->lock));
	_aml_bus_device_resource_allocator=omm_init("aml_bus_device_resource",sizeof(aml_bus_device_resource_t),8,2,pmm_alloc_counter("omm_aml_bus_device_resource"));
	spinlock_init(&(_aml_bus_device_resource_allocator->lock));
	aml_bus_device_handle_type=handle_alloc("aml_bus_device",NULL);
	_enumerate_system_bus(system_bus,NULL);
}
