#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/kernel.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "aml"



#define OP_ZERO 0x01
#define OP_ONE 0x02
#define OP_ALIAS 0x03
#define OP_NAME 0x04
#define OP_BYTE_PREFIX 0x05
#define OP_WORD_PREFIX 0x06
#define OP_DWORD_PREFIX 0x07
#define OP_STRING_PREFIX 0x08
#define OP_QWORD_PREFIX 0x09
#define OP_SCOPE 0x0a
#define OP_BUFFER 0x0b
#define OP_PACKAGE 0x0c
#define OP_VAR_PACKAGE 0x0d
#define OP_METHOD 0x0e
#define OP_DUAL_NAME_PREFIX 0x0f
#define OP_MULTI_NAME_PREFIX 0x10
#define OP_ROOT_CH 0x11
#define OP_PARENT_PREFIX_CH 0x12
#define OP_NAME_CH 0x13
#define OP_LOCAL0 0x14
#define OP_LOCAL1 0x15
#define OP_LOCAL2 0x16
#define OP_LOCAL3 0x17
#define OP_LOCAL4 0x18
#define OP_LOCAL5 0x19
#define OP_LOCAL6 0x1a
#define OP_LOCAL7 0x1b
#define OP_ARG0 0x1c
#define OP_ARG1 0x1d
#define OP_ARG2 0x1e
#define OP_ARG3 0x1f
#define OP_ARG4 0x20
#define OP_ARG5 0x21
#define OP_ARG6 0x22
#define OP_STORE 0x23
#define OP_REF_OF 0x24
#define OP_ADD 0x25
#define OP_CONCAT 0x26
#define OP_SUBTRACT 0x27
#define OP_INCREMENT 0x28
#define OP_DECREMENT 0x29
#define OP_MULTIPLY 0x2a
#define OP_DIVIDE 0x2b
#define OP_SHIFT_LEFT 0x2c
#define OP_SHIFT_RIGHT 0x2d
#define OP_AND 0x2e
#define OP_NAND 0x2f
#define OP_OR 0x30
#define OP_NOR 0x31
#define OP_XOR 0x32
#define OP_NOT 0x33
#define OP_FIND_SET_LEFT_BIT 0x34
#define OP_FIND_SET_RIGHT_BIT 0x35
#define OP_DEREF_OF 0x36
#define OP_CONCAT_RES 0x37
#define OP_MOD 0x38
#define OP_NOTIFY 0x39
#define OP_SIZE_OF 0x3a
#define OP_INDEX 0x3b
#define OP_MATCH 0x3c
#define OP_CREATE_D_WORD_FIELD 0x3d
#define OP_CREATE_WORD_FIELD 0x3e
#define OP_CREATE_BYTE_FIELD 0x3f
#define OP_CREATE_BIT_FIELD 0x40
#define OP_OBJECT_TYPE 0x41
#define OP_CREATE_Q_WORD_FIELD 0x42
#define OP_L_AND 0x43
#define OP_L_OR 0x44
#define OP_L_NOT_EQUAL 0x45
#define OP_L_LESS_EQUAL 0x46
#define OP_L_GREATER_EQUAL 0x47
#define OP_L_NOT 0x48
#define OP_L_EQUAL 0x49
#define OP_L_GREATER 0x4a
#define OP_L_LESS 0x4b
#define OP_TO_BUFFER 0x4c
#define OP_TO_DECIMAL_STRING 0x4d
#define OP_TO_HEX_STRING 0x4e
#define OP_TO_INTEGER 0x4f
#define OP_TO_STRING 0x50
#define OP_COPY_OBJECT 0x51
#define OP_MID 0x52
#define OP_CONTINUE 0x53
#define OP_IF 0x54
#define OP_ELSE 0x55
#define OP_WHILE 0x56
#define OP_NOOP 0x57
#define OP_RETURN 0x58
#define OP_BREAK 0x59
#define OP_BREAK_POINT 0x5a
#define OP_ONES 0x5b

#define OP_EXT_MUTEX 0x81
#define OP_EXT_EVENT 0x82
#define OP_EXT_COND_REF_OF 0x83
#define OP_EXT_CREATE_FIELD 0x84
#define OP_EXT_LOAD_TABLE 0x85
#define OP_EXT_LOAD 0x86
#define OP_EXT_STALL 0x87
#define OP_EXT_SLEEP 0x88
#define OP_EXT_ACQUIRE 0x89
#define OP_EXT_SIGNAL 0x8a
#define OP_EXT_WAIT 0x8b
#define OP_EXT_RESET 0x8c
#define OP_EXT_RELEASE 0x8d
#define OP_EXT_FROM_BCD 0x8e
#define OP_EXT_TO_BCD 0x8f
#define OP_EXT_REVISION 0x90
#define OP_EXT_DEBUG 0x91
#define OP_EXT_FATAL 0x92
#define OP_EXT_TIMER 0x93
#define OP_EXT_OP_REGION 0x94
#define OP_EXT_FIELD 0x95
#define OP_EXT_DEVICE 0x96
#define OP_EXT_POWER_RES 0x97
#define OP_EXT_THERMAL_ZONE 0x98
#define OP_EXT_INDEX_FIELD 0x99
#define OP_EXT_BANK_FIELD 0x9a
#define OP_EXT_DATA_REGION 0x9b

#define STACK_SIZE 128



#define OBJECT_NO_ARRAY_SIZE 0xffffffff



#define OBJECT_TYPE_SCOPE 0x01
#define OBJECT_TYPE_OP_REGION 0x02
#define OBJECT_TYPE_WHILE 0x03
#define OBJECT_TYPE_LESS 0x04
#define OBJECT_TYPE_LOCAL_VAR 0x05
#define OBJECT_TYPE_STORE 0x06
#define OBJECT_TYPE_DEREF 0x07
#define OBJECT_TYPE_INDEX 0x08



typedef struct _OBJECT_HEADER{
	u8 type;
} object_header_t;



typedef struct _OBJECT_ARRAY_HEADER{
	u8 type;
	u32 length;
	object_header_t** data;
} object_array_header_t;



typedef struct _OBJECT_NAMED_ARRAY_HEADER{
	object_array_header_t header;
	u32 name_length;
	const char* name;
} object_named_array_header_t;



typedef object_named_array_header_t object_scope_t;



typedef object_array_header_t object_while_t;



typedef object_array_header_t object_less_t;



typedef struct _OBJECT_LOCAL_VAR{
	object_header_t header;
	u8 index;
} object_local_var_t;



typedef object_named_array_header_t object_store_t;



typedef object_named_array_header_t object_index_t;



typedef object_array_header_t object_deref_t;



typedef struct _OBJECT_OP_REGION{
	object_named_array_header_t header;
	u8 type;
} object_op_region_t;



typedef struct _ALLOCATOR{
	u64 top;
	u64 max_top;
} allocator_t;



typedef struct _STACK_ENTRY{
	object_array_header_t* object;
	u32 count;
} stack_entry_t;



typedef struct _STACK{
	stack_entry_t data[STACK_SIZE];
	u32 size;
} stack_t;



static const u8 _aml_opcode_translation_table[256]={
	[0x00]=OP_ZERO,
	[0x01]=OP_ONE,
	[0x06]=OP_ALIAS,
	[0x08]=OP_NAME,
	[0x0a]=OP_BYTE_PREFIX,
	[0x0b]=OP_WORD_PREFIX,
	[0x0c]=OP_DWORD_PREFIX,
	[0x0d]=OP_STRING_PREFIX,
	[0x0e]=OP_QWORD_PREFIX,
	[0x10]=OP_SCOPE,
	[0x11]=OP_BUFFER,
	[0x12]=OP_PACKAGE,
	[0x13]=OP_VAR_PACKAGE,
	[0x14]=OP_METHOD,
	[0x2e]=OP_DUAL_NAME_PREFIX,
	[0x2f]=OP_MULTI_NAME_PREFIX,
	[0x5c]=OP_ROOT_CH,
	[0x5e]=OP_PARENT_PREFIX_CH,
	[0x5f]=OP_NAME_CH,
	[0x60]=OP_LOCAL0,
	[0x61]=OP_LOCAL1,
	[0x62]=OP_LOCAL2,
	[0x63]=OP_LOCAL3,
	[0x64]=OP_LOCAL4,
	[0x65]=OP_LOCAL5,
	[0x66]=OP_LOCAL6,
	[0x67]=OP_LOCAL7,
	[0x68]=OP_ARG0,
	[0x69]=OP_ARG1,
	[0x6a]=OP_ARG2,
	[0x6b]=OP_ARG3,
	[0x6c]=OP_ARG4,
	[0x6d]=OP_ARG5,
	[0x6e]=OP_ARG6,
	[0x70]=OP_STORE,
	[0x71]=OP_REF_OF,
	[0x72]=OP_ADD,
	[0x73]=OP_CONCAT,
	[0x74]=OP_SUBTRACT,
	[0x75]=OP_INCREMENT,
	[0x76]=OP_DECREMENT,
	[0x77]=OP_MULTIPLY,
	[0x78]=OP_DIVIDE,
	[0x79]=OP_SHIFT_LEFT,
	[0x7a]=OP_SHIFT_RIGHT,
	[0x7b]=OP_AND,
	[0x7c]=OP_NAND,
	[0x7d]=OP_OR,
	[0x7e]=OP_NOR,
	[0x7f]=OP_XOR,
	[0x80]=OP_NOT,
	[0x81]=OP_FIND_SET_LEFT_BIT,
	[0x82]=OP_FIND_SET_RIGHT_BIT,
	[0x83]=OP_DEREF_OF,
	[0x84]=OP_CONCAT_RES,
	[0x85]=OP_MOD,
	[0x86]=OP_NOTIFY,
	[0x87]=OP_SIZE_OF,
	[0x88]=OP_INDEX,
	[0x89]=OP_MATCH,
	[0x8a]=OP_CREATE_D_WORD_FIELD,
	[0x8b]=OP_CREATE_WORD_FIELD,
	[0x8c]=OP_CREATE_BYTE_FIELD,
	[0x8d]=OP_CREATE_BIT_FIELD,
	[0x8e]=OP_OBJECT_TYPE,
	[0x8f]=OP_CREATE_Q_WORD_FIELD,
	[0x90]=OP_L_AND,
	[0x91]=OP_L_OR,
	[0x93]=OP_L_EQUAL,
	[0x94]=OP_L_GREATER,
	[0x95]=OP_L_LESS,
	[0x96]=OP_TO_BUFFER,
	[0x97]=OP_TO_DECIMAL_STRING,
	[0x98]=OP_TO_HEX_STRING,
	[0x99]=OP_TO_INTEGER,
	[0x9c]=OP_TO_STRING,
	[0x9d]=OP_COPY_OBJECT,
	[0x9e]=OP_MID,
	[0x9f]=OP_CONTINUE,
	[0xa0]=OP_IF,
	[0xa1]=OP_ELSE,
	[0xa2]=OP_WHILE,
	[0xa3]=OP_NOOP,
	[0xa4]=OP_RETURN,
	[0xa5]=OP_BREAK,
	[0xcc]=OP_BREAK_POINT,
	[0xff]=OP_ONES,
};



static const u8 _aml_extended_opcode_translation_table[256]={
	[0x01]=OP_EXT_MUTEX,
	[0x02]=OP_EXT_EVENT,
	[0x12]=OP_EXT_COND_REF_OF,
	[0x13]=OP_EXT_CREATE_FIELD,
	[0x1f]=OP_EXT_LOAD_TABLE,
	[0x20]=OP_EXT_LOAD,
	[0x21]=OP_EXT_STALL,
	[0x22]=OP_EXT_SLEEP,
	[0x23]=OP_EXT_ACQUIRE,
	[0x24]=OP_EXT_SIGNAL,
	[0x25]=OP_EXT_WAIT,
	[0x26]=OP_EXT_RESET,
	[0x27]=OP_EXT_RELEASE,
	[0x28]=OP_EXT_FROM_BCD,
	[0x29]=OP_EXT_TO_BCD,
	[0x30]=OP_EXT_REVISION,
	[0x31]=OP_EXT_DEBUG,
	[0x32]=OP_EXT_FATAL,
	[0x33]=OP_EXT_TIMER,
	[0x80]=OP_EXT_OP_REGION,
	[0x81]=OP_EXT_FIELD,
	[0x82]=OP_EXT_DEVICE,
	[0x84]=OP_EXT_POWER_RES,
	[0x85]=OP_EXT_THERMAL_ZONE,
	[0x86]=OP_EXT_INDEX_FIELD,
	[0x87]=OP_EXT_BANK_FIELD,
	[0x88]=OP_EXT_DATA_REGION,
};



static void* _allocate_data(allocator_t* allocator,u32 size){
	size=(size+7)&0xfffffffffffffff8ull;
	while (allocator->top+size>allocator->max_top){
		vmm_map_page(&vmm_kernel_pagemap,pmm_alloc(1,PMM_COUNTER_CPU),allocator->max_top,VMM_PAGE_FLAG_PRESENT|VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_USER);
		allocator->max_top+=PAGE_SIZE;
	}
	void* out=(void*)(allocator->top);
	allocator->top+=size;
	return out;
}



static void* _allocate_object(allocator_t* allocator,u8 type,u32 size,u32 array_size){
	object_header_t* out=_allocate_data(allocator,size);
	out->type=type;
	if (array_size!=OBJECT_NO_ARRAY_SIZE){
		object_array_header_t* array=(object_array_header_t*)out;
		array->length=array_size;
		array->data=_allocate_data(allocator,array_size*sizeof(object_header_t*));
	}
	return out;
}



static u32 _parse_pkglength(const u8* data,u32* offset){
	u32 out=data[*offset]&0x3f;
	for (u8 i=0;i<(data[*offset]>>6);i++){
		out=(out<<8)|data[*offset+i+1];
	}
	(*offset)+=(data[*offset]>>6)+1;
	return out;
}



static u32 _parse_string(const u8* data,u32* offset){
	u32 out=*offset;
	do{
		out++;
	} while (data[out-1]);
	out-=*offset;
	(*offset)+=out;
	return out;
}



static void _push_stack_value(stack_t* stack,void* object){
	stack_entry_t* entry=stack->data+stack->size;
	while (stack->size){
		entry--;
		entry->object->data[entry->count]=object;
		entry->count++;
		if (entry->count<entry->object->length){
			return;
		}
		if (entry->object->type==OBJECT_TYPE_STORE){
			ERROR("Unimplemented ~ AAA");for (;;);
		}
		else if (entry->object->type==OBJECT_TYPE_INDEX){
			ERROR("Unimplemented ~ BBB");for (;;);
		}
		object=(object_header_t*)(entry->object);
		stack->size--;
	}
	if (!stack->size){
		WARN("Output object: %p",object);
		return;
	}
}



static void _push_stack_array(stack_t* stack,void* object){
	if (!((object_array_header_t*)object)->length){
		_push_stack_value(stack,object);
		return;
	}
	if (stack->size==STACK_SIZE){
		ERROR("Stack overflow");
		return;
	}
	(stack->data+stack->size)->object=object;
	(stack->data+stack->size)->count=0;
	stack->size++;
}



void aml_load(const u8* data,u32 length){
	LOG("Loading AML...");
	allocator_t allocator={
		pmm_align_up_address(kernel_get_bss_end()+kernel_get_offset()),
		pmm_align_up_address(kernel_get_bss_end()+kernel_get_offset())
	};
	stack_t stack={
		.size=0
	};
	u32 offset=0;
	while (offset<length){
		u8 op_type=0;
		if (data[offset]==0x5b){
			offset++;
			op_type=_aml_extended_opcode_translation_table[data[offset]];
		}
		else if (data[offset]==0x92){
			if (data[offset+1]==0x93){
				offset++;
				op_type=OP_L_NOT_EQUAL;
			}
			else if (data[offset+1]==0x94){
				offset++;
				op_type=OP_L_LESS_EQUAL;
			}
			else if (data[offset+1]==0x95){
				offset++;
				op_type=OP_L_GREATER_EQUAL;
			}
			else{
				op_type=OP_L_NOT;
			}
		}
		else{
			op_type=_aml_opcode_translation_table[data[offset]];
		}
		if (!op_type){
			ERROR("Unknown AML opcode '%x'",data[offset]);
			return;
		}
		offset++;
		switch (op_type){
			default:
				ERROR("Unimplemented: %x",op_type);for (;;);
				break;
			case OP_ZERO:
				ERROR("Unimplemented: OP_ZERO");for (;;);
				break;
			case OP_ONE:
				ERROR("Unimplemented: OP_ONE");for (;;);
				break;
			case OP_ALIAS:
				ERROR("Unimplemented: OP_ALIAS");for (;;);
				break;
			case OP_NAME:
				ERROR("Unimplemented: OP_NAME");for (;;);
				break;
			case OP_BYTE_PREFIX:
				ERROR("Unimplemented: OP_BYTE_PREFIX");for (;;);
				break;
			case OP_WORD_PREFIX:
				ERROR("Unimplemented: OP_WORD_PREFIX");for (;;);
				break;
			case OP_DWORD_PREFIX:
				ERROR("Unimplemented: OP_DWORD_PREFIX");for (;;);
				break;
			case OP_STRING_PREFIX:
				ERROR("Unimplemented: OP_STRING_PREFIX");for (;;);
				break;
			case OP_QWORD_PREFIX:
				ERROR("Unimplemented: OP_QWORD_PREFIX");for (;;);
				break;
			case OP_SCOPE:
				{
					object_scope_t* scope=_allocate_object(&allocator,OBJECT_TYPE_SCOPE,sizeof(object_scope_t),_parse_pkglength(data,&offset));
					scope->name=(const char*)(data+offset);
					scope->name_length=_parse_string(data,&offset);
					_push_stack_array(&stack,scope);
					break;
				}
			case OP_BUFFER:
				ERROR("Unimplemented: OP_BUFFER");for (;;);
				break;
			case OP_PACKAGE:
				ERROR("Unimplemented: OP_PACKAGE");for (;;);
				break;
			case OP_VAR_PACKAGE:
				ERROR("Unimplemented: OP_VAR_PACKAGE");for (;;);
				break;
			case OP_METHOD:
				ERROR("Unimplemented: OP_METHOD");for (;;);
				break;
			case OP_DUAL_NAME_PREFIX:
				ERROR("Unimplemented: OP_DUAL_NAME_PREFIX");for (;;);
				break;
			case OP_MULTI_NAME_PREFIX:
				ERROR("Unimplemented: OP_MULTI_NAME_PREFIX");for (;;);
				break;
			case OP_ROOT_CH:
				ERROR("Unimplemented: OP_ROOT_CH");for (;;);
				break;
			case OP_PARENT_PREFIX_CH:
				ERROR("Unimplemented: OP_PARENT_PREFIX_CH");for (;;);
				break;
			case OP_NAME_CH:
				ERROR("Unimplemented: OP_NAME_CH");for (;;);
				break;
			case OP_LOCAL0:
			case OP_LOCAL1:
			case OP_LOCAL2:
			case OP_LOCAL3:
			case OP_LOCAL4:
			case OP_LOCAL5:
			case OP_LOCAL6:
			case OP_LOCAL7:
				{
					object_local_var_t* local_var=_allocate_object(&allocator,OBJECT_TYPE_LOCAL_VAR,sizeof(object_local_var_t),OBJECT_NO_ARRAY_SIZE);
					local_var->index=op_type-OP_LOCAL0;
					_push_stack_value(&stack,local_var);
					break;
				}
			case OP_ARG0:
				ERROR("Unimplemented: OP_ARG0");for (;;);
				break;
			case OP_ARG1:
				ERROR("Unimplemented: OP_ARG1");for (;;);
				break;
			case OP_ARG2:
				ERROR("Unimplemented: OP_ARG2");for (;;);
				break;
			case OP_ARG3:
				ERROR("Unimplemented: OP_ARG3");for (;;);
				break;
			case OP_ARG4:
				ERROR("Unimplemented: OP_ARG4");for (;;);
				break;
			case OP_ARG5:
				ERROR("Unimplemented: OP_ARG5");for (;;);
				break;
			case OP_ARG6:
				ERROR("Unimplemented: OP_ARG6");for (;;);
				break;
			case OP_STORE:
				_push_stack_array(&stack,_allocate_object(&allocator,OBJECT_TYPE_STORE,sizeof(object_store_t),1));
				break;
			case OP_REF_OF:
				ERROR("Unimplemented: OP_REF_OF");for (;;);
				break;
			case OP_ADD:
				ERROR("Unimplemented: OP_ADD");for (;;);
				break;
			case OP_CONCAT:
				ERROR("Unimplemented: OP_CONCAT");for (;;);
				break;
			case OP_SUBTRACT:
				ERROR("Unimplemented: OP_SUBTRACT");for (;;);
				break;
			case OP_INCREMENT:
				ERROR("Unimplemented: OP_INCREMENT");for (;;);
				break;
			case OP_DECREMENT:
				ERROR("Unimplemented: OP_DECREMENT");for (;;);
				break;
			case OP_MULTIPLY:
				ERROR("Unimplemented: OP_MULTIPLY");for (;;);
				break;
			case OP_DIVIDE:
				ERROR("Unimplemented: OP_DIVIDE");for (;;);
				break;
			case OP_SHIFT_LEFT:
				ERROR("Unimplemented: OP_SHIFT_LEFT");for (;;);
				break;
			case OP_SHIFT_RIGHT:
				ERROR("Unimplemented: OP_SHIFT_RIGHT");for (;;);
				break;
			case OP_AND:
				ERROR("Unimplemented: OP_AND");for (;;);
				break;
			case OP_NAND:
				ERROR("Unimplemented: OP_NAND");for (;;);
				break;
			case OP_OR:
				ERROR("Unimplemented: OP_OR");for (;;);
				break;
			case OP_NOR:
				ERROR("Unimplemented: OP_NOR");for (;;);
				break;
			case OP_XOR:
				ERROR("Unimplemented: OP_XOR");for (;;);
				break;
			case OP_NOT:
				ERROR("Unimplemented: OP_NOT");for (;;);
				break;
			case OP_FIND_SET_LEFT_BIT:
				ERROR("Unimplemented: OP_FIND_SET_LEFT_BIT");for (;;);
				break;
			case OP_FIND_SET_RIGHT_BIT:
				ERROR("Unimplemented: OP_FIND_SET_RIGHT_BIT");for (;;);
				break;
			case OP_DEREF_OF:
				_push_stack_array(&stack,_allocate_object(&allocator,OBJECT_TYPE_DEREF,sizeof(object_deref_t),1));
				break;
			case OP_CONCAT_RES:
				ERROR("Unimplemented: OP_CONCAT_RES");for (;;);
				break;
			case OP_MOD:
				ERROR("Unimplemented: OP_MOD");for (;;);
				break;
			case OP_NOTIFY:
				ERROR("Unimplemented: OP_NOTIFY");for (;;);
				break;
			case OP_SIZE_OF:
				ERROR("Unimplemented: OP_SIZE_OF");for (;;);
				break;
			case OP_INDEX:
				_push_stack_array(&stack,_allocate_object(&allocator,OBJECT_TYPE_INDEX,sizeof(object_index_t),2));
				break;
			case OP_MATCH:
				ERROR("Unimplemented: OP_MATCH");for (;;);
				break;
			case OP_CREATE_D_WORD_FIELD:
				ERROR("Unimplemented: OP_CREATE_D_WORD_FIELD");for (;;);
				break;
			case OP_CREATE_WORD_FIELD:
				ERROR("Unimplemented: OP_CREATE_WORD_FIELD");for (;;);
				break;
			case OP_CREATE_BYTE_FIELD:
				ERROR("Unimplemented: OP_CREATE_BYTE_FIELD");for (;;);
				break;
			case OP_CREATE_BIT_FIELD:
				ERROR("Unimplemented: OP_CREATE_BIT_FIELD");for (;;);
				break;
			case OP_OBJECT_TYPE:
				ERROR("Unimplemented: OP_OBJECT_TYPE");for (;;);
				break;
			case OP_CREATE_Q_WORD_FIELD:
				ERROR("Unimplemented: OP_CREATE_Q_WORD_FIELD");for (;;);
				break;
			case OP_L_AND:
				ERROR("Unimplemented: OP_L_AND");for (;;);
				break;
			case OP_L_OR:
				ERROR("Unimplemented: OP_L_OR");for (;;);
				break;
			case OP_L_NOT_EQUAL:
				ERROR("Unimplemented: OP_L_NOT_EQUAL");for (;;);
				break;
			case OP_L_LESS_EQUAL:
				ERROR("Unimplemented: OP_L_LESS_EQUAL");for (;;);
				break;
			case OP_L_GREATER_EQUAL:
				ERROR("Unimplemented: OP_L_GREATER_EQUAL");for (;;);
				break;
			case OP_L_NOT:
				ERROR("Unimplemented: OP_L_NOT");for (;;);
				break;
			case OP_L_EQUAL:
				ERROR("Unimplemented: OP_L_EQUAL");for (;;);
				break;
			case OP_L_GREATER:
				ERROR("Unimplemented: OP_L_GREATER");for (;;);
				break;
			case OP_L_LESS:
				_push_stack_array(&stack,_allocate_object(&allocator,OBJECT_TYPE_LESS,sizeof(object_less_t),2));
				break;
			case OP_TO_BUFFER:
				ERROR("Unimplemented: OP_TO_BUFFER");for (;;);
				break;
			case OP_TO_DECIMAL_STRING:
				ERROR("Unimplemented: OP_TO_DECIMAL_STRING");for (;;);
				break;
			case OP_TO_HEX_STRING:
				ERROR("Unimplemented: OP_TO_HEX_STRING");for (;;);
				break;
			case OP_TO_INTEGER:
				ERROR("Unimplemented: OP_TO_INTEGER");for (;;);
				break;
			case OP_TO_STRING:
				ERROR("Unimplemented: OP_TO_STRING");for (;;);
				break;
			case OP_COPY_OBJECT:
				ERROR("Unimplemented: OP_COPY_OBJECT");for (;;);
				break;
			case OP_MID:
				ERROR("Unimplemented: OP_MID");for (;;);
				break;
			case OP_CONTINUE:
				ERROR("Unimplemented: OP_CONTINUE");for (;;);
				break;
			case OP_IF:
				ERROR("Unimplemented: OP_IF");for (;;);
				break;
			case OP_ELSE:
				ERROR("Unimplemented: OP_ELSE");for (;;);
				break;
			case OP_WHILE:
				_push_stack_array(&stack,_allocate_object(&allocator,OBJECT_TYPE_WHILE,sizeof(object_while_t),_parse_pkglength(data,&offset)+1));
				break;
			case OP_NOOP:
				ERROR("Unimplemented: OP_NOOP");for (;;);
				break;
			case OP_RETURN:
				ERROR("Unimplemented: OP_RETURN");for (;;);
				break;
			case OP_BREAK:
				ERROR("Unimplemented: OP_BREAK");for (;;);
				break;
			case OP_BREAK_POINT:
				ERROR("Unimplemented: OP_BREAK_POINT");for (;;);
				break;
			case OP_ONES:
				ERROR("Unimplemented: OP_ONES");for (;;);
				break;
			case OP_EXT_MUTEX:
				ERROR("Unimplemented: OP_EXT_MUTEX");for (;;);
				break;
			case OP_EXT_EVENT:
				ERROR("Unimplemented: OP_EXT_EVENT");for (;;);
				break;
			case OP_EXT_COND_REF_OF:
				ERROR("Unimplemented: OP_EXT_COND_REF_OF");for (;;);
				break;
			case OP_EXT_CREATE_FIELD:
				ERROR("Unimplemented: OP_EXT_CREATE_FIELD");for (;;);
				break;
			case OP_EXT_LOAD_TABLE:
				ERROR("Unimplemented: OP_EXT_LOAD_TABLE");for (;;);
				break;
			case OP_EXT_LOAD:
				ERROR("Unimplemented: OP_EXT_LOAD");for (;;);
				break;
			case OP_EXT_STALL:
				ERROR("Unimplemented: OP_EXT_STALL");for (;;);
				break;
			case OP_EXT_SLEEP:
				ERROR("Unimplemented: OP_EXT_SLEEP");for (;;);
				break;
			case OP_EXT_ACQUIRE:
				ERROR("Unimplemented: OP_EXT_ACQUIRE");for (;;);
				break;
			case OP_EXT_SIGNAL:
				ERROR("Unimplemented: OP_EXT_SIGNAL");for (;;);
				break;
			case OP_EXT_WAIT:
				ERROR("Unimplemented: OP_EXT_WAIT");for (;;);
				break;
			case OP_EXT_RESET:
				ERROR("Unimplemented: OP_EXT_RESET");for (;;);
				break;
			case OP_EXT_RELEASE:
				ERROR("Unimplemented: OP_EXT_RELEASE");for (;;);
				break;
			case OP_EXT_FROM_BCD:
				ERROR("Unimplemented: OP_EXT_FROM_BCD");for (;;);
				break;
			case OP_EXT_TO_BCD:
				ERROR("Unimplemented: OP_EXT_TO_BCD");for (;;);
				break;
			case OP_EXT_REVISION:
				ERROR("Unimplemented: OP_EXT_REVISION");for (;;);
				break;
			case OP_EXT_DEBUG:
				ERROR("Unimplemented: OP_EXT_DEBUG");for (;;);
				break;
			case OP_EXT_FATAL:
				ERROR("Unimplemented: OP_EXT_FATAL");for (;;);
				break;
			case OP_EXT_TIMER:
				ERROR("Unimplemented: OP_EXT_TIMER");for (;;);
				break;
			case OP_EXT_OP_REGION:
				{
					object_op_region_t* op_region=_allocate_object(&allocator,OBJECT_TYPE_OP_REGION,sizeof(object_op_region_t),2);
					op_region->header.name=(const char*)(data+offset);
					op_region->header.name_length=_parse_string(data,&offset);
					op_region->type=data[offset];
					offset++;
					_push_stack_array(&stack,op_region);
					break;
				}
			case OP_EXT_FIELD:
				ERROR("Unimplemented: OP_EXT_FIELD");for (;;);
				break;
			case OP_EXT_DEVICE:
				ERROR("Unimplemented: OP_EXT_DEVICE");for (;;);
				break;
			case OP_EXT_POWER_RES:
				ERROR("Unimplemented: OP_EXT_POWER_RES");for (;;);
				break;
			case OP_EXT_THERMAL_ZONE:
				ERROR("Unimplemented: OP_EXT_THERMAL_ZONE");for (;;);
				break;
			case OP_EXT_INDEX_FIELD:
				ERROR("Unimplemented: OP_EXT_INDEX_FIELD");for (;;);
				break;
			case OP_EXT_BANK_FIELD:
				ERROR("Unimplemented: OP_EXT_BANK_FIELD");for (;;);
				break;
			case OP_EXT_DATA_REGION:
				ERROR("Unimplemented: OP_EXT_DATA_REGION");for (;;);
				break;
		}
	}
	(void)stack;
	for (;;);
}
