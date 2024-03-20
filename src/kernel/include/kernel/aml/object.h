#ifndef _KERNEL_AML_OBJECT_H_
#define _KERNEL_AML_OBJECT_H_ 1
#include <kernel/memory/smm.h>
#include <kernel/types.h>



#define AML_OBJECT_TYPE_NONE 0
#define AML_OBJECT_TYPE_BUFFER 1
#define AML_OBJECT_TYPE_BUFFER_FIELD 2
#define AML_OBJECT_TYPE_DEBUG 3
#define AML_OBJECT_TYPE_DEVICE 4
#define AML_OBJECT_TYPE_EVENT 5
#define AML_OBJECT_TYPE_FIELD_UNIT 6
#define AML_OBJECT_TYPE_INTEGER 7
#define AML_OBJECT_TYPE_METHOD 8
#define AML_OBJECT_TYPE_MUTEX 9
#define AML_OBJECT_TYPE_PACKAGE 10
#define AML_OBJECT_TYPE_POWER_RESOURCE 11
#define AML_OBJECT_TYPE_PROCESSOR 12
#define AML_OBJECT_TYPE_REFERENCE 13
#define AML_OBJECT_TYPE_REGION 14
#define AML_OBJECT_TYPE_STRING 15
#define AML_OBJECT_TYPE_THERMAL_ZONE 16
#define AML_OBJECT_TYPE_DYNAMIC 17



typedef struct _AML_OBJECT{
	u8 type;
	u32 rc;
	union{
		struct{
			u32 size;
			u8* data;
		} buffer;
		struct{
			// undefined
		} buffer_field;
		struct{
			// undefined
		} debug;
		struct{
			// undefined
		} device;
		struct{
			// undefined
		} event;
		struct{
			u8 type;
			u8 flags;
			u64 address;
			u32 offset;
			u32 size;
		} field_unit;
		u64 integer;
		struct{
			u8 flags;
			const u8* code;
			u64 code_length;
			struct _AML_NAMESPACE* namespace;
		} method;
		struct{
			u8 sync_flags;
		} mutex;
		struct{
			u8 length;
			struct _AML_OBJECT** data;
		} package;
		struct{
			// undefined
		} power_resource;
		struct{
			u8 id;
			u32 block_address;
			u8 block_length;
		} processor;
		struct{
			// undefined
		} reference;
		struct{
			u8 type;
			u64 address;
			u64 length;
		} region;
		string_t* string;
		struct{
			// undefined
		} thermal_zone;
		struct{
			struct _AML_NAMESPACE* namespace;
			string_t* name;
		} dynamic;
	};
} aml_object_t;



aml_object_t* aml_object_alloc(u8 type);



aml_object_t* aml_object_alloc_none(void);



aml_object_t* aml_object_alloc_buffer(u32 size,u8* data);



aml_object_t* aml_object_alloc_buffer_field(void);



aml_object_t* aml_object_alloc_debug(void);



aml_object_t* aml_object_alloc_device(void);



aml_object_t* aml_object_alloc_event(void);



aml_object_t* aml_object_alloc_field_unit(u8 type,u8 flags,u64 address,u32 offset,u32 size);



aml_object_t* aml_object_alloc_integer(u64 integer);



aml_object_t* aml_object_alloc_method(u8 flags,const u8* code,u64 code_length,struct _AML_NAMESPACE* namespace);



aml_object_t* aml_object_alloc_mutex(u8 sync_flags);



aml_object_t* aml_object_alloc_package(u8 length);



aml_object_t* aml_object_alloc_power_resource(void);



aml_object_t* aml_object_alloc_processor(u8 id,u32 block_address,u8 block_length);



aml_object_t* aml_object_alloc_reference(void);



aml_object_t* aml_object_alloc_region(u8 type,u64 address,u64 length);



aml_object_t* aml_object_alloc_string(string_t* string);



aml_object_t* aml_object_alloc_thermal_zone(void);



aml_object_t* aml_object_alloc_dynamic(struct _AML_NAMESPACE* namespace,string_t* name);



void aml_object_dealloc(aml_object_t* object);



void aml_object_print(aml_object_t* object);



#endif
