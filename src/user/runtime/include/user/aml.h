#ifndef _USER_AML_H_
#define _USER_AML_H_ 1
#include <user/types.h>



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



typedef struct _AML_NODE{
	char name[5];
	u8 type;
	u8 _padding;
	struct _AML_NODE* parent;
	struct _AML_NODE* next;
	struct _AML_NODE* child;
	union{
		struct{
			u64 length;
			u8* data;
		} buffer;
		struct{
			u64 size;
			void* pointer;
		} buffer_field;
		struct{
			u8 type;
			u8 access_type;
			u8 _padding[4];
			u64 address;
			u64 size;
		} field_unit;
		u64 integer;
		struct{
			u8 flags;
			u32 object_count;
			const void* objects;
			struct _AML_NODE* namespace;
		} method;
		struct{
			u8 type;
			u64 offset;
			u64 length;
		} region;
		struct{
			u8 length;
			struct _AML_NODE* elements;
		} package;
		struct{
			u8 id;
			u64 block_address;
			u8 block_length;
		} processor;
		struct{
			u64 length;
			const char* data;
		} string;
	} data;
} aml_node_t;



extern const aml_node_t* aml_root_node;



void aml_print_node(const aml_node_t* node);



#endif
