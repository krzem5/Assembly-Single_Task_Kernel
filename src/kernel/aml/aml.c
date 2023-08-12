#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
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



#define OBJECT_TYPE_SCOPE 0x01



typedef struct _OBJECT_HEADER{
	u8 type;
} object_header_t;


typedef struct _OBJECT_SCOPE{
	object_header_t header;
	u32 data;
} object_scope_t;



typedef struct _STACK_ENTRY{
	u8 op_type;
	u32 count;
	u32 length;
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
	while (data[out]){
		out++;
	}
	out-=*offset;
	(*offset)+=out;
	return out;
}



void aml_load(const u8* data,u32 length){
	LOG("Loading AML...");
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
			case OP_SCOPE:
				u32 length=_parse_pkglength(data,&offset);
				const u8* name=data+offset;
				u32 name_length=_parse_string(data,&offset);
				(void)name_length;
				ERROR("%u %s",length,name);for (;;);
				break;
		}
	}
	(void)stack;
	for (;;);
}
