#ifndef _KERNEL_AML_AML_H_
#define _KERNEL_AML_AML_H_ 1
#include <kernel/types.h>



#define AML_EXTENDED_OPCODE 0x5b



#define AML_OPCODE_ZERO 0x00
#define AML_OPCODE_ONE 0x01
#define AML_OPCODE_ALIAS 0x06
#define AML_OPCODE_NAME 0x08
#define AML_OPCODE_BYTE_PREFIX 0x0a
#define AML_OPCODE_WORD_PREFIX 0x0b
#define AML_OPCODE_DWORD_PREFIX 0x0c
#define AML_OPCODE_STRING_PREFIX 0x0d
#define AML_OPCODE_QWORD_PREFIX 0x0e
#define AML_OPCODE_SCOPE 0x10
#define AML_OPCODE_BUFFER 0x11
#define AML_OPCODE_PACKAGE 0x12
#define AML_OPCODE_VAR_PACKAGE 0x13
#define AML_OPCODE_METHOD 0x14
#define AML_OPCODE_LOCAL0 0x60
#define AML_OPCODE_LOCAL1 0x61
#define AML_OPCODE_LOCAL2 0x62
#define AML_OPCODE_LOCAL3 0x63
#define AML_OPCODE_LOCAL4 0x64
#define AML_OPCODE_LOCAL5 0x65
#define AML_OPCODE_LOCAL6 0x66
#define AML_OPCODE_LOCAL7 0x67
#define AML_OPCODE_ARG0 0x68
#define AML_OPCODE_ARG1 0x69
#define AML_OPCODE_ARG2 0x6a
#define AML_OPCODE_ARG3 0x6b
#define AML_OPCODE_ARG4 0x6c
#define AML_OPCODE_ARG5 0x6d
#define AML_OPCODE_ARG6 0x6e
#define AML_OPCODE_STORE 0x70
#define AML_OPCODE_REF_OF 0x71
#define AML_OPCODE_ADD 0x72
#define AML_OPCODE_CONCAT 0x73
#define AML_OPCODE_SUBTRACT 0x74
#define AML_OPCODE_INCREMENT 0x75
#define AML_OPCODE_DECREMENT 0x76
#define AML_OPCODE_MULTIPLY 0x77
#define AML_OPCODE_DIVIDE 0x78
#define AML_OPCODE_SHIFT_LEFT 0x79
#define AML_OPCODE_SHIFT_RIGHT 0x7a
#define AML_OPCODE_AND 0x7b
#define AML_OPCODE_NAND 0x7c
#define AML_OPCODE_OR 0x7d
#define AML_OPCODE_NOR 0x7e
#define AML_OPCODE_XOR 0x7f
#define AML_OPCODE_NOT 0x80
#define AML_OPCODE_FIND_SET_LEFT_BIT 0x81
#define AML_OPCODE_FIND_SET_RIGHT_BIT 0x82
#define AML_OPCODE_DEREF_OF 0x83
#define AML_OPCODE_CONCAT_RES 0x84
#define AML_OPCODE_MOD 0x85
#define AML_OPCODE_NOTIFY 0x86
#define AML_OPCODE_SIZE_OF 0x87
#define AML_OPCODE_INDEX 0x88
#define AML_OPCODE_MATCH 0x89
#define AML_OPCODE_CREATE_DWORD_FIELD 0x8a
#define AML_OPCODE_CREATE_WORD_FIELD 0x8b
#define AML_OPCODE_CREATE_BYTE_FIELD 0x8c
#define AML_OPCODE_CREATE_BIT_FIELD 0x8d
#define AML_OPCODE_OBJECT_TYPE 0x8e
#define AML_OPCODE_CREATE_QWORD_FIELD 0x8f
#define AML_OPCODE_L_AND 0x90
#define AML_OPCODE_L_OR 0x91
#define AML_OPCODE_L_NOT 0x92
#define AML_OPCODE_L_EQUAL 0x93
#define AML_OPCODE_L_GREATER 0x94
#define AML_OPCODE_L_LESS 0x95
#define AML_OPCODE_TO_BUFFER 0x96
#define AML_OPCODE_TO_DECIMAL_STRING 0x97
#define AML_OPCODE_TO_HEX_STRING 0x98
#define AML_OPCODE_TO_INTEGER 0x99
#define AML_OPCODE_TO_STRING 0x9c
#define AML_OPCODE_COPY_OBJECT 0x9d
#define AML_OPCODE_MID 0x9e
#define AML_OPCODE_CONTINUE 0x9f
#define AML_OPCODE_IF 0xa0
#define AML_OPCODE_ELSE 0xa1
#define AML_OPCODE_WHILE 0xa2
#define AML_OPCODE_NOOP 0xa3
#define AML_OPCODE_RETURN 0xa4
#define AML_OPCODE_BREAK 0xa5
#define AML_OPCODE_BREAK_POINT 0xcc
#define AML_OPCODE_STRING 0xfd
#define AML_OPCODE_ROOT 0xfe
#define AML_OPCODE_ONES 0xff

#define AML_OPCODE_EXT_MUTEX 0x01
#define AML_OPCODE_EXT_EVENT 0x02
#define AML_OPCODE_EXT_COND_REF_OF 0x12
#define AML_OPCODE_EXT_CREATE_FIELD 0x13
#define AML_OPCODE_EXT_LOAD_TABLE 0x1f
#define AML_OPCODE_EXT_LOAD 0x20
#define AML_OPCODE_EXT_STALL 0x21
#define AML_OPCODE_EXT_SLEEP 0x22
#define AML_OPCODE_EXT_ACQUIRE 0x23
#define AML_OPCODE_EXT_SIGNAL 0x24
#define AML_OPCODE_EXT_WAIT 0x25
#define AML_OPCODE_EXT_RESET 0x26
#define AML_OPCODE_EXT_RELEASE 0x27
#define AML_OPCODE_EXT_FROM_BCD 0x28
#define AML_OPCODE_EXT_TO_BCD 0x29
#define AML_OPCODE_EXT_UNLOAD 0x2a
#define AML_OPCODE_EXT_REVISION 0x30
#define AML_OPCODE_EXT_DEBUG 0x31
#define AML_OPCODE_EXT_FATAL 0x32
#define AML_OPCODE_EXT_TIMER 0x33
#define AML_OPCODE_EXT_REGION 0x80
#define AML_OPCODE_EXT_FIELD 0x81
#define AML_OPCODE_EXT_DEVICE 0x82
#define AML_OPCODE_EXT_PROCESSOR 0x83
#define AML_OPCODE_EXT_POWER_RES 0x84
#define AML_OPCODE_EXT_THERMAL_ZONE 0x85
#define AML_OPCODE_EXT_INDEX_FIELD 0x86
#define AML_OPCODE_EXT_BANK_FIELD 0x87
#define AML_OPCODE_EXT_DATA_REGION 0x88

#define AML_OBJECT_FLAG_BYTE_DATA 0x01

#define AML_OBJECT_ARG_TYPE_UINT8 1
#define AML_OBJECT_ARG_TYPE_UINT16 2
#define AML_OBJECT_ARG_TYPE_UINT32 3
#define AML_OBJECT_ARG_TYPE_UINT64 4
#define AML_OBJECT_ARG_TYPE_STRING 5
#define AML_OBJECT_ARG_TYPE_NAME 6
#define AML_OBJECT_ARG_TYPE_OBJECT 7

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



typedef struct _AML_OBJECT{
	u8 opcode[2];
	u8 arg_count;
	u8 flags;
	u32 data_length;
	struct{
		u8 type;
		u16 string_length;
		union{
			u64 number;
			const char* string;
			struct _AML_OBJECT* object;
		};
	} args[6];
	union{
		const u8* bytes;
		struct _AML_OBJECT* objects;
	} data;
} aml_object_t;



typedef struct _AML_NODE{
	char name[5];
	u8 type;
	u8 flags;
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
			// Unimplemented
		} event;
		struct{
			u8 type;
			u8 access_type;
			u64 address;
			u64 size;
		} field_unit;
		u64 integer;
		struct{
			u8 flags;
			const aml_object_t* objects;
			u32 object_count;
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
			struct _AML_NODE* elements;
		} package;
		struct{
			// Unimplemented
		} power_resource;
		struct{
			u8 id;
			u64 block_address;
			u8 block_length;
		} processor;
		struct{
			u64 length;
			const char* data;
		} string;
		struct{
			// Unimplemented
		} thermal_zone;
	} data;
} aml_node_t;



extern aml_node_t* aml_root_node;



aml_object_t* aml_parse(const u8* data,u32 length);



u32 aml_parse_pkglength(const u8* data,u32* out);



void aml_build_runtime(aml_object_t* root,u16 irq);



void aml_init_irq(void);



aml_node_t* aml_get_node(aml_node_t* root,const char* path);



#endif
