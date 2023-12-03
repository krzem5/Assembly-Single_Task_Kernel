#include <aml/object.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/types.h>



#define PRINT_BUFFER_BYTES_PER_LINE 16



static omm_allocator_t* _aml_object_allocator=NULL;



static aml_object_t* _alloc_object(u8 type){
	if (!_aml_object_allocator){
		_aml_object_allocator=omm_init("aml_object",sizeof(aml_object_t),8,4,pmm_alloc_counter("omm_aml_object"));
	}
	aml_object_t* out=omm_alloc(_aml_object_allocator);
	out->type=type;
	out->rc=1;
	return out;
}



static void _print_object(aml_object_t* object,u32 indent){
	if (!object){
		log("(null)");
		return;
	}
	for (u32 i=0;i<indent;i++){
		log(" ");
	}
	switch (object->type){
		case AML_OBJECT_TYPE_NONE:
			log("<none>");
			break;
		case AML_OBJECT_TYPE_BUFFER:
			if (!object->buffer->length){
				log("[0]{}");
				break;
			}
			log("[%u]{",object->buffer->length);
			if (object->buffer->length<=PRINT_BUFFER_BYTES_PER_LINE){
				for (u32 i=0;i<object->buffer->length;i++){
					log("%s%X",(i?" ":""),object->buffer->data[i]);
				}
				log("}");
				break;
			}
			for (u8 i=0;i<object->buffer->length;i++){
				if (!(i&(PRINT_BUFFER_BYTES_PER_LINE-1))){
					log("\n   ");
					for (u32 i=0;i<indent;i++){
						log(" ");
					}
				}
				log(" %X",object->buffer->data[i]);
			}
			log("\n");
			for (u32 i=0;i<indent;i++){
				log(" ");
			}
			log("}");
			break;
		case AML_OBJECT_TYPE_BUFFER_FIELD:
			log("buffer_field");
			break;
		case AML_OBJECT_TYPE_DEBUG:
			log("debug");
			break;
		case AML_OBJECT_TYPE_DEVICE:
			log("device");
			break;
		case AML_OBJECT_TYPE_EVENT:
			log("event");
			break;
		case AML_OBJECT_TYPE_FIELD_UNIT:
			log("field_unit");
			break;
		case AML_OBJECT_TYPE_INTEGER:
			log("0x%lx",object->integer);
			break;
		case AML_OBJECT_TYPE_METHOD:
			log("method");
			break;
		case AML_OBJECT_TYPE_MUTEX:
			log("mutex");
			break;
		case AML_OBJECT_TYPE_PACKAGE:
			if (!object->package.length){
				log("{}");
				break;
			}
			log("{\n");
			for (u8 i=0;i<object->package.length;i++){
				_print_object(object->package.data[i],indent+4);
				if (i<object->package.length-1){
					log(",\n");
				}
				else{
					log("\n");
				}
			}
			for (u32 i=0;i<indent;i++){
				log(" ");
			}
			log("}");
			break;
		case AML_OBJECT_TYPE_POWER_RESOURCE:
			log("power_resource");
			break;
		case AML_OBJECT_TYPE_PROCESSOR:
			log("processor");
			break;
		case AML_OBJECT_TYPE_REFERENCE:
			log("reference");
			break;
		case AML_OBJECT_TYPE_REGION:
			log("region");
			break;
		case AML_OBJECT_TYPE_STRING:
			log("\"%s\"",object->string->data);
			break;
		case AML_OBJECT_TYPE_THERMAL_ZONE:
			log("thermal_zone");
			break;
	}
}



KERNEL_PUBLIC aml_object_t* aml_object_alloc_none(void){
	return _alloc_object(AML_OBJECT_TYPE_NONE);
}



KERNEL_PUBLIC aml_object_t* aml_object_alloc_buffer(string_t* buffer){
	aml_object_t* out=_alloc_object(AML_OBJECT_TYPE_BUFFER);
	out->buffer=buffer;
	return out;
}



KERNEL_PUBLIC aml_object_t* aml_object_alloc_buffer_field(void){
	aml_object_t* out=_alloc_object(AML_OBJECT_TYPE_BUFFER_FIELD);
	return out;
}



KERNEL_PUBLIC aml_object_t* aml_object_alloc_debug(void){
	aml_object_t* out=_alloc_object(AML_OBJECT_TYPE_DEBUG);
	return out;
}



KERNEL_PUBLIC aml_object_t* aml_object_alloc_device(void){
	aml_object_t* out=_alloc_object(AML_OBJECT_TYPE_DEVICE);
	return out;
}



KERNEL_PUBLIC aml_object_t* aml_object_alloc_event(void){
	aml_object_t* out=_alloc_object(AML_OBJECT_TYPE_EVENT);
	return out;
}



KERNEL_PUBLIC aml_object_t* aml_object_alloc_field_unit(u8 type,u8 flags,u64 address,u32 offset,u32 size){
	aml_object_t* out=_alloc_object(AML_OBJECT_TYPE_FIELD_UNIT);
	out->field_unit.type=type;
	out->field_unit.flags=flags;
	out->field_unit.address=address;
	out->field_unit.offset=offset;
	out->field_unit.size=size;
	return out;
}



KERNEL_PUBLIC aml_object_t* aml_object_alloc_integer(u64 integer){
	aml_object_t* out=_alloc_object(AML_OBJECT_TYPE_INTEGER);
	out->integer=integer;
	return out;
}



KERNEL_PUBLIC aml_object_t* aml_object_alloc_method(u8 flags,const u8* code,u64 code_length,struct _AML_NAMESPACE* namespace){
	aml_object_t* out=_alloc_object(AML_OBJECT_TYPE_METHOD);
	out->method.flags=flags;
	out->method.code=code;
	out->method.code_length=code_length;
	out->method.namespace=namespace;
	return out;
}



KERNEL_PUBLIC aml_object_t* aml_object_alloc_mutex(u8 sync_flags){
	aml_object_t* out=_alloc_object(AML_OBJECT_TYPE_MUTEX);
	out->mutex.sync_flags=sync_flags;
	return out;
}



KERNEL_PUBLIC aml_object_t* aml_object_alloc_package(u8 length){
	aml_object_t* out=_alloc_object(AML_OBJECT_TYPE_PACKAGE);
	out->package.length=length;
	out->package.data=amm_alloc(length*sizeof(aml_object_t*));
	return out;
}



KERNEL_PUBLIC aml_object_t* aml_object_alloc_power_resource(void){
	aml_object_t* out=_alloc_object(AML_OBJECT_TYPE_POWER_RESOURCE);
	return out;
}



KERNEL_PUBLIC aml_object_t* aml_object_alloc_processor(u8 id,u32 block_address,u8 block_length){
	aml_object_t* out=_alloc_object(AML_OBJECT_TYPE_PROCESSOR);
	out->processor.id=id;
	out->processor.block_address=block_address;
	out->processor.block_length=block_length;
	return out;
}



KERNEL_PUBLIC aml_object_t* aml_object_alloc_reference(void){
	aml_object_t* out=_alloc_object(AML_OBJECT_TYPE_REFERENCE);
	return out;
}



KERNEL_PUBLIC aml_object_t* aml_object_alloc_region(u8 type,u64 address,u64 length){
	aml_object_t* out=_alloc_object(AML_OBJECT_TYPE_REGION);
	out->region.type=type;
	out->region.address=address;
	out->region.length=length;
	return out;
}



KERNEL_PUBLIC aml_object_t* aml_object_alloc_string(string_t* string){
	aml_object_t* out=_alloc_object(AML_OBJECT_TYPE_STRING);
	out->string=string;
	return out;
}



KERNEL_PUBLIC aml_object_t* aml_object_alloc_thermal_zone(void){
	aml_object_t* out=_alloc_object(AML_OBJECT_TYPE_THERMAL_ZONE);
	return out;
}



KERNEL_PUBLIC void aml_object_dealloc(aml_object_t* object){
	if (!object){
		return;
	}
	object->rc--;
	if (object->rc){
		return;
	}
	if (object->type==AML_OBJECT_TYPE_BUFFER){
		smm_dealloc(object->buffer);
	}
	else if (object->type==AML_OBJECT_TYPE_STRING){
		smm_dealloc(object->string);
	}
	else if (object->type==AML_OBJECT_TYPE_PACKAGE){
		for (u8 i=0;i<object->package.length;i++){
			aml_object_dealloc(object->package.data[i]);
		}
		amm_dealloc(object->package.data);
	}
	omm_dealloc(_aml_object_allocator,object);
}



KERNEL_PUBLIC void aml_object_print(aml_object_t* object){
	_print_object(object,0);
	log("\n");
}
