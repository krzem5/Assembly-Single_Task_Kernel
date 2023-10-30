#ifndef _KERNEL_AML_RUNTIME_H_
#define _KERNEL_AML_RUNTIME_H_ 1
#include <kernel/aml/parser.h>
#include <kernel/lock/spinlock.h>
#include <kernel/types.h>
#include <kernel/vfs/name.h>



#define AML_NODE_TYPE_UNDEFINED 0
#define AML_NODE_TYPE_BUFFER 1
#define AML_NODE_TYPE_BUFFER_FIELD 2
#define AML_NODE_TYPE_DEBUG 3
#define AML_NODE_TYPE_DEVICE 4
#define AML_NODE_TYPE_EVENT 5
#define AML_NODE_TYPE_FIELD_UNIT 6
#define AML_NODE_TYPE_INTEGER 7
#define AML_NODE_TYPE_METHOD 8
#define AML_NODE_TYPE_MUTEX 9
#define AML_NODE_TYPE_REFERENCE 10
#define AML_NODE_TYPE_REGION 11
#define AML_NODE_TYPE_PACKAGE 12
#define AML_NODE_TYPE_POWER_RESOURCE 13
#define AML_NODE_TYPE_PROCESSOR 14
#define AML_NODE_TYPE_STRING 15
#define AML_NODE_TYPE_THERMAL_ZONE 16
#define AML_NODE_TYPE_SCOPE 17

#define AML_NODE_FLAG_LOCAL 1



typedef struct _AML_NODE{
	char name[5];
	u8 type;
	u8 flags;
	struct _AML_NODE* parent;
	struct _AML_NODE* next;
	struct _AML_NODE* child;
	union{
		vfs_name_t* buffer;
		struct{
			u64 size;
			void* pointer;
		} buffer_field;
		struct{
			// Unimplemented
		} event;
		struct{
			u8 type;
			u8 access_type;
			spinlock_t lock;
			u64 address;
			u64 size;
		} field_unit;
		u64 integer;
		struct{
			u8 flags;
			u32 child_count;
			const aml_object_t* child;
			struct _AML_NODE* namespace;
		} method;
		struct{
			// Unimplemented
		} mutex;
		struct{
			// Unimplemented
		} reference;
		struct{
			u8 type;
			u64 offset;
			u64 length;
		} region;
		struct{
			u8 length;
			struct _AML_NODE* child;
		} package;
		struct{
			// Unimplemented
		} power_resource;
		struct{
			u8 id;
			u64 block_address;
			u8 block_length;
		} processor;
		vfs_name_t* string;
		struct{
			// Unimplemented
		} thermal_zone;
	} data;
} aml_node_t;



extern aml_node_t* aml_root_node;



void aml_runtime_init(aml_object_t* root,u16 irq);



aml_node_t* aml_runtime_get_node(aml_node_t* root,const char* path);



aml_node_t* aml_runtime_evaluate_node(aml_node_t* node);



void aml_runtime_print_node(aml_node_t* node);



#endif
