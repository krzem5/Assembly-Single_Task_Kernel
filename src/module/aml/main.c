#include <kernel/acpi/fadt.h>
#include <kernel/log/log.h>
#include <kernel/memory/smm.h>
#include <kernel/module/module.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "aml"



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
#define AML_OPCODE_NAME_REFERENCE 0xfd
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



typedef struct _AML_READER_CONTEXT{
	const u8*const data;
	const u64 length;
	u64 offset;
} aml_reader_context_t;



typedef _Bool (*opcode_func_t)(aml_reader_context_t*);



static _Bool _execute_aml_single(aml_reader_context_t* ctx);



static _Bool _execute_aml(aml_reader_context_t* ctx);



static u32 _get_pkglength(aml_reader_context_t* ctx){
	u32 out=ctx->data[ctx->offset]&0x3f;
	for (u8 i=0;i<(ctx->data[ctx->offset]>>6);i++){
		out|=ctx->data[ctx->offset+i+1]<<(4+(i<<3));
	}
	ctx->offset+=(ctx->data[ctx->offset]>>6)+1;
	return out;
}



static string_t* _get_name(aml_reader_context_t* ctx){
	char buffer[4096];
	u16 length=0;
	while (ctx->data[ctx->offset]=='\\'||ctx->data[ctx->offset]=='^'){
		buffer[length]=ctx->data[ctx->offset];
		length++;
		ctx->offset++;
	}
	u8 segment_count=1;
	if (ctx->data[ctx->offset]=='.'){
		ctx->offset++;
		segment_count=2;
	}
	else if (ctx->data[ctx->offset]=='/'){
		ctx->offset++;
		segment_count=ctx->data[ctx->offset];
		ctx->offset++;
	}
	else if (!ctx->data[ctx->offset]){
		ctx->offset++;
		segment_count=0;
	}
	while (segment_count){
		segment_count--;
		for (u8 i=0;i<4;i++){
			buffer[length]=ctx->data[ctx->offset];
			length++;
			ctx->offset++;
		}
		if (segment_count){
			buffer[length]='.';
			length++;
		}
	}
	return smm_alloc(buffer,length);
}



static u8 _get_uint8(aml_reader_context_t* ctx){
	u8 out=ctx->data[ctx->offset];
	ctx->offset++;
	return out;
}



static u16 _get_uint16(aml_reader_context_t* ctx){
	u16 out=(ctx->data[ctx->offset]<<8)|ctx->data[ctx->offset+1];
	ctx->offset+=2;
	return out;
}



static u32 _get_uint32(aml_reader_context_t* ctx){
	u32 out=(ctx->data[ctx->offset]<<24)|(ctx->data[ctx->offset+1]<<16)|(ctx->data[ctx->offset+2]<<8)|ctx->data[ctx->offset+3];
	ctx->offset+=4;
	return out;
}



static _Bool _exec_opcode_zero(aml_reader_context_t* ctx){
	ERROR("zero");
	return 1;
}



static _Bool _exec_opcode_one(aml_reader_context_t* ctx){
	ERROR("one");
	return 1;
}



static _Bool _exec_opcode_alias(aml_reader_context_t* ctx){
	ERROR("alias");
	return 0;
}



static _Bool _exec_opcode_name(aml_reader_context_t* ctx){
	string_t* name=_get_name(ctx);
	_Bool object=_execute_aml_single(ctx);
	ERROR("name (name=%s, ref=%p)",name->data,object);
	return 1;
}



static _Bool _exec_opcode_byte_prefix(aml_reader_context_t* ctx){
	u8 out=_get_uint8(ctx);
	ERROR("byte_prefix (value=%u)",out);
	return 1;
}



static _Bool _exec_opcode_word_prefix(aml_reader_context_t* ctx){
	u16 out=_get_uint16(ctx);
	ERROR("word_prefix (value=%u)",out);
	return 1;
}



static _Bool _exec_opcode_dword_prefix(aml_reader_context_t* ctx){
	u32 out=_get_uint32(ctx);
	ERROR("dword_prefix (value=%u)",out);
	return 0;
}



static _Bool _exec_opcode_string_prefix(aml_reader_context_t* ctx){
	u32 length=0;
	for (;ctx->data[ctx->offset+length];length++);
	ERROR("string_prefix (data=%s)",ctx->data+ctx->offset);
	ctx->offset+=length+1;
	return 1;
}



static _Bool _exec_opcode_qword_prefix(aml_reader_context_t* ctx){
	ERROR("qword_prefix");
	return 0;
}



static _Bool _exec_opcode_scope(aml_reader_context_t* ctx){
	u64 end_offset=ctx->offset+_get_pkglength(ctx);
	string_t* name=_get_name(ctx);
	ERROR("scope (name=%s)",name->data);
	aml_reader_context_t child_ctx={
		ctx->data+ctx->offset,
		end_offset-ctx->offset
	};
	if (!_execute_aml(&child_ctx)){
		return 0;
	}
	ctx->offset=end_offset;
	return 1;
}



static _Bool _exec_opcode_buffer(aml_reader_context_t* ctx){
	u64 end_offset=ctx->offset+_get_pkglength(ctx);
	_Bool buffer_size=_execute_aml_single(ctx);
	ERROR("buffer (buffer_size=%p)",buffer_size);
	ctx->offset=end_offset;
	return 1;
}



static _Bool _exec_opcode_package(aml_reader_context_t* ctx){
	u64 end_offset=ctx->offset+_get_pkglength(ctx);
	u8 num_elements=_get_uint8(ctx);
	ERROR("package (num_elements=%p)",num_elements);
	aml_reader_context_t child_ctx={
		ctx->data+ctx->offset,
		end_offset-ctx->offset
	};
	if (!_execute_aml(&child_ctx)){
		return 0;
	}
	ctx->offset=end_offset;
	return 1;
}



static _Bool _exec_opcode_var_package(aml_reader_context_t* ctx){
	ERROR("var_package");
	return 0;
}



static _Bool _exec_opcode_method(aml_reader_context_t* ctx){
	u64 end_offset=ctx->offset+_get_pkglength(ctx);
	string_t* name=_get_name(ctx);
	u8 arg_count=_get_uint8(ctx);
	ERROR("method (name=%s, arg_count=%u)",name->data,arg_count);
	ctx->offset=end_offset;
	return 1;
}



static _Bool _exec_opcode_name_reference(aml_reader_context_t* ctx){
	ctx->offset--;
	string_t* name=_get_name(ctx);
	ERROR("name_reference: %s",name->data);
	return 1;
}



static _Bool _exec_opcode_local0(aml_reader_context_t* ctx){
	ERROR("local0");
	return 0;
}



static _Bool _exec_opcode_local1(aml_reader_context_t* ctx){
	ERROR("local1");
	return 0;
}



static _Bool _exec_opcode_local2(aml_reader_context_t* ctx){
	ERROR("local2");
	return 0;
}



static _Bool _exec_opcode_local3(aml_reader_context_t* ctx){
	ERROR("local3");
	return 0;
}



static _Bool _exec_opcode_local4(aml_reader_context_t* ctx){
	ERROR("local4");
	return 0;
}



static _Bool _exec_opcode_local5(aml_reader_context_t* ctx){
	ERROR("local5");
	return 0;
}



static _Bool _exec_opcode_local6(aml_reader_context_t* ctx){
	ERROR("local6");
	return 0;
}



static _Bool _exec_opcode_local7(aml_reader_context_t* ctx){
	ERROR("local7");
	return 0;
}



static _Bool _exec_opcode_arg0(aml_reader_context_t* ctx){
	ERROR("arg0");
	return 0;
}



static _Bool _exec_opcode_arg1(aml_reader_context_t* ctx){
	ERROR("arg1");
	return 0;
}



static _Bool _exec_opcode_arg2(aml_reader_context_t* ctx){
	ERROR("arg2");
	return 0;
}



static _Bool _exec_opcode_arg3(aml_reader_context_t* ctx){
	ERROR("arg3");
	return 0;
}



static _Bool _exec_opcode_arg4(aml_reader_context_t* ctx){
	ERROR("arg4");
	return 0;
}



static _Bool _exec_opcode_arg5(aml_reader_context_t* ctx){
	ERROR("arg5");
	return 0;
}



static _Bool _exec_opcode_arg6(aml_reader_context_t* ctx){
	ERROR("arg6");
	return 0;
}



static _Bool _exec_opcode_store(aml_reader_context_t* ctx){
	ERROR("store");
	return 0;
}



static _Bool _exec_opcode_ref_of(aml_reader_context_t* ctx){
	ERROR("ref_of");
	return 0;
}



static _Bool _exec_opcode_add(aml_reader_context_t* ctx){
	ERROR("add");
	return 0;
}



static _Bool _exec_opcode_concat(aml_reader_context_t* ctx){
	ERROR("concat");
	return 0;
}



static _Bool _exec_opcode_subtract(aml_reader_context_t* ctx){
	ERROR("subtract");
	return 0;
}



static _Bool _exec_opcode_increment(aml_reader_context_t* ctx){
	ERROR("increment");
	return 0;
}



static _Bool _exec_opcode_decrement(aml_reader_context_t* ctx){
	ERROR("decrement");
	return 0;
}



static _Bool _exec_opcode_multiply(aml_reader_context_t* ctx){
	ERROR("multiply");
	return 0;
}



static _Bool _exec_opcode_divide(aml_reader_context_t* ctx){
	ERROR("divide");
	return 0;
}



static _Bool _exec_opcode_shift_left(aml_reader_context_t* ctx){
	ERROR("shift_left");
	return 0;
}



static _Bool _exec_opcode_shift_right(aml_reader_context_t* ctx){
	ERROR("shift_right");
	return 0;
}



static _Bool _exec_opcode_and(aml_reader_context_t* ctx){
	ERROR("and");
	return 0;
}



static _Bool _exec_opcode_nand(aml_reader_context_t* ctx){
	ERROR("nand");
	return 0;
}



static _Bool _exec_opcode_or(aml_reader_context_t* ctx){
	ERROR("or");
	return 0;
}



static _Bool _exec_opcode_nor(aml_reader_context_t* ctx){
	ERROR("nor");
	return 0;
}



static _Bool _exec_opcode_xor(aml_reader_context_t* ctx){
	ERROR("xor");
	return 0;
}



static _Bool _exec_opcode_not(aml_reader_context_t* ctx){
	ERROR("not");
	return 0;
}



static _Bool _exec_opcode_find_set_left_bit(aml_reader_context_t* ctx){
	ERROR("find_set_left_bit");
	return 0;
}



static _Bool _exec_opcode_find_set_right_bit(aml_reader_context_t* ctx){
	ERROR("find_set_right_bit");
	return 0;
}



static _Bool _exec_opcode_deref_of(aml_reader_context_t* ctx){
	ERROR("deref_of");
	return 0;
}



static _Bool _exec_opcode_concat_res(aml_reader_context_t* ctx){
	ERROR("concat_res");
	return 0;
}



static _Bool _exec_opcode_mod(aml_reader_context_t* ctx){
	ERROR("mod");
	return 0;
}



static _Bool _exec_opcode_notify(aml_reader_context_t* ctx){
	ERROR("notify");
	return 0;
}



static _Bool _exec_opcode_size_of(aml_reader_context_t* ctx){
	ERROR("size_of");
	return 0;
}



static _Bool _exec_opcode_index(aml_reader_context_t* ctx){
	ERROR("index");
	return 0;
}



static _Bool _exec_opcode_match(aml_reader_context_t* ctx){
	ERROR("match");
	return 0;
}



static _Bool _exec_opcode_create_dword_field(aml_reader_context_t* ctx){
	ERROR("create_dword_field");
	return 0;
}



static _Bool _exec_opcode_create_word_field(aml_reader_context_t* ctx){
	ERROR("create_word_field");
	return 0;
}



static _Bool _exec_opcode_create_byte_field(aml_reader_context_t* ctx){
	ERROR("create_byte_field");
	return 0;
}



static _Bool _exec_opcode_create_bit_field(aml_reader_context_t* ctx){
	ERROR("create_bit_field");
	return 0;
}



static _Bool _exec_opcode_object_type(aml_reader_context_t* ctx){
	ERROR("object_type");
	return 0;
}



static _Bool _exec_opcode_create_qword_field(aml_reader_context_t* ctx){
	ERROR("create_qword_field");
	return 0;
}



static _Bool _exec_opcode_l_and(aml_reader_context_t* ctx){
	ERROR("l_and");
	return 0;
}



static _Bool _exec_opcode_l_or(aml_reader_context_t* ctx){
	ERROR("l_or");
	return 0;
}



static _Bool _exec_opcode_l_not(aml_reader_context_t* ctx){
	ERROR("l_not");
	return 0;
}



static _Bool _exec_opcode_l_equal(aml_reader_context_t* ctx){
	ERROR("l_equal");
	return 0;
}



static _Bool _exec_opcode_l_greater(aml_reader_context_t* ctx){
	ERROR("l_greater");
	return 0;
}



static _Bool _exec_opcode_l_less(aml_reader_context_t* ctx){
	ERROR("l_less");
	return 0;
}



static _Bool _exec_opcode_to_buffer(aml_reader_context_t* ctx){
	ERROR("to_buffer");
	return 0;
}



static _Bool _exec_opcode_to_decimal_string(aml_reader_context_t* ctx){
	ERROR("to_decimal_string");
	return 0;
}



static _Bool _exec_opcode_to_hex_string(aml_reader_context_t* ctx){
	ERROR("to_hex_string");
	return 0;
}



static _Bool _exec_opcode_to_integer(aml_reader_context_t* ctx){
	ERROR("to_integer");
	return 0;
}



static _Bool _exec_opcode_to_string(aml_reader_context_t* ctx){
	ERROR("to_string");
	return 0;
}



static _Bool _exec_opcode_copy_object(aml_reader_context_t* ctx){
	ERROR("copy_object");
	return 0;
}



static _Bool _exec_opcode_mid(aml_reader_context_t* ctx){
	ERROR("mid");
	return 0;
}



static _Bool _exec_opcode_continue(aml_reader_context_t* ctx){
	ERROR("continue");
	return 0;
}



static _Bool _exec_opcode_if(aml_reader_context_t* ctx){
	ERROR("if");
	return 0;
}



static _Bool _exec_opcode_else(aml_reader_context_t* ctx){
	ERROR("else");
	return 0;
}



static _Bool _exec_opcode_while(aml_reader_context_t* ctx){
	ERROR("while");
	return 0;
}



static _Bool _exec_opcode_noop(aml_reader_context_t* ctx){
	ERROR("noop");
	return 0;
}



static _Bool _exec_opcode_return(aml_reader_context_t* ctx){
	ERROR("return");
	return 0;
}



static _Bool _exec_opcode_break(aml_reader_context_t* ctx){
	ERROR("break");
	return 0;
}



static _Bool _exec_opcode_break_point(aml_reader_context_t* ctx){
	ERROR("break_point");
	return 0;
}



static _Bool _exec_opcode_ones(aml_reader_context_t* ctx){
	ERROR("ones");
	return 0;
}



static _Bool _exec_opcode_ext_mutex(aml_reader_context_t* ctx){
	string_t* name=_get_name(ctx);
	u8 sync_flags=_get_uint8(ctx);
	ERROR("ext_mutex (name=%s, sync_flags=%u)",name->data,sync_flags);
	return 1;
}



static _Bool _exec_opcode_ext_event(aml_reader_context_t* ctx){
	ERROR("ext_event");
	return 0;
}



static _Bool _exec_opcode_ext_cond_ref_of(aml_reader_context_t* ctx){
	ERROR("ext_cond_ref_of");
	return 0;
}



static _Bool _exec_opcode_ext_create_field(aml_reader_context_t* ctx){
	ERROR("ext_create_field");
	return 0;
}



static _Bool _exec_opcode_ext_load_table(aml_reader_context_t* ctx){
	ERROR("ext_load_table");
	return 0;
}



static _Bool _exec_opcode_ext_load(aml_reader_context_t* ctx){
	ERROR("ext_load");
	return 0;
}



static _Bool _exec_opcode_ext_stall(aml_reader_context_t* ctx){
	ERROR("ext_stall");
	return 0;
}



static _Bool _exec_opcode_ext_sleep(aml_reader_context_t* ctx){
	ERROR("ext_sleep");
	return 0;
}



static _Bool _exec_opcode_ext_acquire(aml_reader_context_t* ctx){
	ERROR("ext_acquire");
	return 0;
}



static _Bool _exec_opcode_ext_signal(aml_reader_context_t* ctx){
	ERROR("ext_signal");
	return 0;
}



static _Bool _exec_opcode_ext_wait(aml_reader_context_t* ctx){
	ERROR("ext_wait");
	return 0;
}



static _Bool _exec_opcode_ext_reset(aml_reader_context_t* ctx){
	ERROR("ext_reset");
	return 0;
}



static _Bool _exec_opcode_ext_release(aml_reader_context_t* ctx){
	ERROR("ext_release");
	return 0;
}



static _Bool _exec_opcode_ext_from_bcd(aml_reader_context_t* ctx){
	ERROR("ext_from_bcd");
	return 0;
}



static _Bool _exec_opcode_ext_to_bcd(aml_reader_context_t* ctx){
	ERROR("ext_to_bcd");
	return 0;
}



static _Bool _exec_opcode_ext_unload(aml_reader_context_t* ctx){
	ERROR("ext_unload");
	return 0;
}



static _Bool _exec_opcode_ext_revision(aml_reader_context_t* ctx){
	ERROR("ext_revision");
	return 0;
}



static _Bool _exec_opcode_ext_debug(aml_reader_context_t* ctx){
	ERROR("ext_debug");
	return 0;
}



static _Bool _exec_opcode_ext_fatal(aml_reader_context_t* ctx){
	ERROR("ext_fatal");
	return 0;
}



static _Bool _exec_opcode_ext_timer(aml_reader_context_t* ctx){
	ERROR("ext_timer");
	return 0;
}



static _Bool _exec_opcode_ext_region(aml_reader_context_t* ctx){
	string_t* name=_get_name(ctx);
	u8 region_space=_get_uint8(ctx);
	_Bool region_offset=_execute_aml_single(ctx);
	_Bool region_length=_execute_aml_single(ctx);
	ERROR("ext_region (name=%s, region_space=%u, region_offset=%p, region_length=%p)",name->data,region_space,region_offset,region_length);
	return 1;
}



static _Bool _exec_opcode_ext_field(aml_reader_context_t* ctx){
	u64 end_offset=ctx->offset+_get_pkglength(ctx);
	string_t* name=_get_name(ctx);
	u8 field_flags=_get_uint8(ctx);
	ERROR("ext_field (name=%s, field_flags=%u)",name->data,field_flags);
	ctx->offset=end_offset;
	return 1;
}



static _Bool _exec_opcode_ext_device(aml_reader_context_t* ctx){
	u64 end_offset=ctx->offset+_get_pkglength(ctx);
	string_t* name=_get_name(ctx);
	ERROR("ext_device (name=%s)",name->data);
	aml_reader_context_t child_ctx={
		ctx->data+ctx->offset,
		end_offset-ctx->offset
	};
	if (!_execute_aml(&child_ctx)){
		return 0;
	}
	ctx->offset=end_offset;
	return 1;
}



static _Bool _exec_opcode_ext_processor(aml_reader_context_t* ctx){
	u64 end_offset=ctx->offset+_get_pkglength(ctx);
	string_t* name=_get_name(ctx);
	u8 processor_id=_get_uint8(ctx);
	u32 processor_block_address=_get_uint32(ctx);
	u8 processor_block_length=_get_uint8(ctx);
	ERROR("ext_processor (name=%s, processor_id=%u, processor_block_address=%u, processor_block_length=%u)",name->data,processor_id,processor_block_address,processor_block_length);
	aml_reader_context_t child_ctx={
		ctx->data+ctx->offset,
		end_offset-ctx->offset
	};
	if (!_execute_aml(&child_ctx)){
		return 0;
	}
	ctx->offset=end_offset;
	return 1;
}



static _Bool _exec_opcode_ext_power_res(aml_reader_context_t* ctx){
	ERROR("ext_power_res");
	return 0;
}



static _Bool _exec_opcode_ext_thermal_zone(aml_reader_context_t* ctx){
	ERROR("ext_thermal_zone");
	return 0;
}



static _Bool _exec_opcode_ext_index_field(aml_reader_context_t* ctx){
	ERROR("ext_index_field");
	return 0;
}



static _Bool _exec_opcode_ext_bank_field(aml_reader_context_t* ctx){
	ERROR("ext_bank_field");
	return 0;
}



static _Bool _exec_opcode_ext_data_region(aml_reader_context_t* ctx){
	ERROR("ext_data_region");
	return 0;
}



static const opcode_func_t _aml_opcodes[256]={
	[AML_OPCODE_ZERO]=_exec_opcode_zero,
	[AML_OPCODE_ONE]=_exec_opcode_one,
	[AML_OPCODE_ALIAS]=_exec_opcode_alias,
	[AML_OPCODE_NAME]=_exec_opcode_name,
	[AML_OPCODE_BYTE_PREFIX]=_exec_opcode_byte_prefix,
	[AML_OPCODE_WORD_PREFIX]=_exec_opcode_word_prefix,
	[AML_OPCODE_DWORD_PREFIX]=_exec_opcode_dword_prefix,
	[AML_OPCODE_STRING_PREFIX]=_exec_opcode_string_prefix,
	[AML_OPCODE_QWORD_PREFIX]=_exec_opcode_qword_prefix,
	[AML_OPCODE_SCOPE]=_exec_opcode_scope,
	[AML_OPCODE_BUFFER]=_exec_opcode_buffer,
	[AML_OPCODE_PACKAGE]=_exec_opcode_package,
	[AML_OPCODE_VAR_PACKAGE]=_exec_opcode_var_package,
	[AML_OPCODE_METHOD]=_exec_opcode_method,
	['.']=_exec_opcode_name_reference,
	['/']=_exec_opcode_name_reference,
	['0']=_exec_opcode_name_reference,
	['1']=_exec_opcode_name_reference,
	['2']=_exec_opcode_name_reference,
	['3']=_exec_opcode_name_reference,
	['4']=_exec_opcode_name_reference,
	['5']=_exec_opcode_name_reference,
	['6']=_exec_opcode_name_reference,
	['7']=_exec_opcode_name_reference,
	['8']=_exec_opcode_name_reference,
	['9']=_exec_opcode_name_reference,
	['A']=_exec_opcode_name_reference,
	['B']=_exec_opcode_name_reference,
	['C']=_exec_opcode_name_reference,
	['D']=_exec_opcode_name_reference,
	['E']=_exec_opcode_name_reference,
	['F']=_exec_opcode_name_reference,
	['G']=_exec_opcode_name_reference,
	['H']=_exec_opcode_name_reference,
	['I']=_exec_opcode_name_reference,
	['J']=_exec_opcode_name_reference,
	['K']=_exec_opcode_name_reference,
	['L']=_exec_opcode_name_reference,
	['M']=_exec_opcode_name_reference,
	['N']=_exec_opcode_name_reference,
	['O']=_exec_opcode_name_reference,
	['P']=_exec_opcode_name_reference,
	['Q']=_exec_opcode_name_reference,
	['R']=_exec_opcode_name_reference,
	['S']=_exec_opcode_name_reference,
	['T']=_exec_opcode_name_reference,
	['U']=_exec_opcode_name_reference,
	['V']=_exec_opcode_name_reference,
	['W']=_exec_opcode_name_reference,
	['X']=_exec_opcode_name_reference,
	['Y']=_exec_opcode_name_reference,
	['Z']=_exec_opcode_name_reference,
	['\\']=_exec_opcode_name_reference,
	['^']=_exec_opcode_name_reference,
	['_']=_exec_opcode_name_reference,
	[AML_OPCODE_LOCAL0]=_exec_opcode_local0,
	[AML_OPCODE_LOCAL1]=_exec_opcode_local1,
	[AML_OPCODE_LOCAL2]=_exec_opcode_local2,
	[AML_OPCODE_LOCAL3]=_exec_opcode_local3,
	[AML_OPCODE_LOCAL4]=_exec_opcode_local4,
	[AML_OPCODE_LOCAL5]=_exec_opcode_local5,
	[AML_OPCODE_LOCAL6]=_exec_opcode_local6,
	[AML_OPCODE_LOCAL7]=_exec_opcode_local7,
	[AML_OPCODE_ARG0]=_exec_opcode_arg0,
	[AML_OPCODE_ARG1]=_exec_opcode_arg1,
	[AML_OPCODE_ARG2]=_exec_opcode_arg2,
	[AML_OPCODE_ARG3]=_exec_opcode_arg3,
	[AML_OPCODE_ARG4]=_exec_opcode_arg4,
	[AML_OPCODE_ARG5]=_exec_opcode_arg5,
	[AML_OPCODE_ARG6]=_exec_opcode_arg6,
	[AML_OPCODE_STORE]=_exec_opcode_store,
	[AML_OPCODE_REF_OF]=_exec_opcode_ref_of,
	[AML_OPCODE_ADD]=_exec_opcode_add,
	[AML_OPCODE_CONCAT]=_exec_opcode_concat,
	[AML_OPCODE_SUBTRACT]=_exec_opcode_subtract,
	[AML_OPCODE_INCREMENT]=_exec_opcode_increment,
	[AML_OPCODE_DECREMENT]=_exec_opcode_decrement,
	[AML_OPCODE_MULTIPLY]=_exec_opcode_multiply,
	[AML_OPCODE_DIVIDE]=_exec_opcode_divide,
	[AML_OPCODE_SHIFT_LEFT]=_exec_opcode_shift_left,
	[AML_OPCODE_SHIFT_RIGHT]=_exec_opcode_shift_right,
	[AML_OPCODE_AND]=_exec_opcode_and,
	[AML_OPCODE_NAND]=_exec_opcode_nand,
	[AML_OPCODE_OR]=_exec_opcode_or,
	[AML_OPCODE_NOR]=_exec_opcode_nor,
	[AML_OPCODE_XOR]=_exec_opcode_xor,
	[AML_OPCODE_NOT]=_exec_opcode_not,
	[AML_OPCODE_FIND_SET_LEFT_BIT]=_exec_opcode_find_set_left_bit,
	[AML_OPCODE_FIND_SET_RIGHT_BIT]=_exec_opcode_find_set_right_bit,
	[AML_OPCODE_DEREF_OF]=_exec_opcode_deref_of,
	[AML_OPCODE_CONCAT_RES]=_exec_opcode_concat_res,
	[AML_OPCODE_MOD]=_exec_opcode_mod,
	[AML_OPCODE_NOTIFY]=_exec_opcode_notify,
	[AML_OPCODE_SIZE_OF]=_exec_opcode_size_of,
	[AML_OPCODE_INDEX]=_exec_opcode_index,
	[AML_OPCODE_MATCH]=_exec_opcode_match,
	[AML_OPCODE_CREATE_DWORD_FIELD]=_exec_opcode_create_dword_field,
	[AML_OPCODE_CREATE_WORD_FIELD]=_exec_opcode_create_word_field,
	[AML_OPCODE_CREATE_BYTE_FIELD]=_exec_opcode_create_byte_field,
	[AML_OPCODE_CREATE_BIT_FIELD]=_exec_opcode_create_bit_field,
	[AML_OPCODE_OBJECT_TYPE]=_exec_opcode_object_type,
	[AML_OPCODE_CREATE_QWORD_FIELD]=_exec_opcode_create_qword_field,
	[AML_OPCODE_L_AND]=_exec_opcode_l_and,
	[AML_OPCODE_L_OR]=_exec_opcode_l_or,
	[AML_OPCODE_L_NOT]=_exec_opcode_l_not,
	[AML_OPCODE_L_EQUAL]=_exec_opcode_l_equal,
	[AML_OPCODE_L_GREATER]=_exec_opcode_l_greater,
	[AML_OPCODE_L_LESS]=_exec_opcode_l_less,
	[AML_OPCODE_TO_BUFFER]=_exec_opcode_to_buffer,
	[AML_OPCODE_TO_DECIMAL_STRING]=_exec_opcode_to_decimal_string,
	[AML_OPCODE_TO_HEX_STRING]=_exec_opcode_to_hex_string,
	[AML_OPCODE_TO_INTEGER]=_exec_opcode_to_integer,
	[AML_OPCODE_TO_STRING]=_exec_opcode_to_string,
	[AML_OPCODE_COPY_OBJECT]=_exec_opcode_copy_object,
	[AML_OPCODE_MID]=_exec_opcode_mid,
	[AML_OPCODE_CONTINUE]=_exec_opcode_continue,
	[AML_OPCODE_IF]=_exec_opcode_if,
	[AML_OPCODE_ELSE]=_exec_opcode_else,
	[AML_OPCODE_WHILE]=_exec_opcode_while,
	[AML_OPCODE_NOOP]=_exec_opcode_noop,
	[AML_OPCODE_RETURN]=_exec_opcode_return,
	[AML_OPCODE_BREAK]=_exec_opcode_break,
	[AML_OPCODE_BREAK_POINT]=_exec_opcode_break_point,
	[AML_OPCODE_ONES]=_exec_opcode_ones
};



static const opcode_func_t _aml_extended_opcodes[256]={
	[AML_OPCODE_EXT_MUTEX]=_exec_opcode_ext_mutex,
	[AML_OPCODE_EXT_EVENT]=_exec_opcode_ext_event,
	[AML_OPCODE_EXT_COND_REF_OF]=_exec_opcode_ext_cond_ref_of,
	[AML_OPCODE_EXT_CREATE_FIELD]=_exec_opcode_ext_create_field,
	[AML_OPCODE_EXT_LOAD_TABLE]=_exec_opcode_ext_load_table,
	[AML_OPCODE_EXT_LOAD]=_exec_opcode_ext_load,
	[AML_OPCODE_EXT_STALL]=_exec_opcode_ext_stall,
	[AML_OPCODE_EXT_SLEEP]=_exec_opcode_ext_sleep,
	[AML_OPCODE_EXT_ACQUIRE]=_exec_opcode_ext_acquire,
	[AML_OPCODE_EXT_SIGNAL]=_exec_opcode_ext_signal,
	[AML_OPCODE_EXT_WAIT]=_exec_opcode_ext_wait,
	[AML_OPCODE_EXT_RESET]=_exec_opcode_ext_reset,
	[AML_OPCODE_EXT_RELEASE]=_exec_opcode_ext_release,
	[AML_OPCODE_EXT_FROM_BCD]=_exec_opcode_ext_from_bcd,
	[AML_OPCODE_EXT_TO_BCD]=_exec_opcode_ext_to_bcd,
	[AML_OPCODE_EXT_UNLOAD]=_exec_opcode_ext_unload,
	[AML_OPCODE_EXT_REVISION]=_exec_opcode_ext_revision,
	[AML_OPCODE_EXT_DEBUG]=_exec_opcode_ext_debug,
	[AML_OPCODE_EXT_FATAL]=_exec_opcode_ext_fatal,
	[AML_OPCODE_EXT_TIMER]=_exec_opcode_ext_timer,
	[AML_OPCODE_EXT_REGION]=_exec_opcode_ext_region,
	[AML_OPCODE_EXT_FIELD]=_exec_opcode_ext_field,
	[AML_OPCODE_EXT_DEVICE]=_exec_opcode_ext_device,
	[AML_OPCODE_EXT_PROCESSOR]=_exec_opcode_ext_processor,
	[AML_OPCODE_EXT_POWER_RES]=_exec_opcode_ext_power_res,
	[AML_OPCODE_EXT_THERMAL_ZONE]=_exec_opcode_ext_thermal_zone,
	[AML_OPCODE_EXT_INDEX_FIELD]=_exec_opcode_ext_index_field,
	[AML_OPCODE_EXT_BANK_FIELD]=_exec_opcode_ext_bank_field,
	[AML_OPCODE_EXT_DATA_REGION]=_exec_opcode_ext_data_region
};



static opcode_func_t _parse_opcode(aml_reader_context_t* ctx){
	const opcode_func_t* table=_aml_opcodes;
	if (ctx->data[ctx->offset]==AML_EXTENDED_OPCODE){
		ctx->offset++;
		table=_aml_extended_opcodes;
	}
	opcode_func_t out=table[ctx->data[ctx->offset]];
	if (out){
		ctx->offset++;
		return out;
	}
	ERROR("Unknown AML opcode '%s%X'",(table==_aml_extended_opcodes?"5b ":""),ctx->data[ctx->offset]);
	return NULL;
}



static _Bool _execute_aml_single(aml_reader_context_t* ctx){
	opcode_func_t opcode=_parse_opcode(ctx);
	if (!opcode||!opcode(ctx)){
		return 0;
	}
	return 1;
}



static _Bool _execute_aml(aml_reader_context_t* ctx){
	ctx->offset=0;
	while (ctx->offset<ctx->length){
		opcode_func_t opcode=_parse_opcode(ctx);
		if (!opcode||!opcode(ctx)){
			return 0;
		}
	}
	return 1;
}



static _Bool _init(module_t* module){
	if (!acpi_dsdt||!acpi_dsdt->header.length){
		return 0;
	}
	aml_reader_context_t ctx={
		acpi_dsdt->data,
		acpi_dsdt->header.length-sizeof(acpi_dsdt_t)
	};
	_execute_aml(&ctx);
	// panic("test");
	// // PM1x flags
	// #define SLP_TYP_SHIFT 10
	// #define SLP_EN 0x2000
	// aml_runtime_init(aml_parse(acpi_dsdt->data,acpi_dsdt->header.length-sizeof(acpi_dsdt_t)),fadt->sci_int);
	// asm volatile("cli":::"memory");
	// u16 pm1a_value=(aml_runtime_get_node(NULL,"\\_S5_[0]")->data.integer<<SLP_TYP_SHIFT)|SLP_EN;
	// u16 pm1b_value=(aml_runtime_get_node(NULL,"\\_S5_[1]")->data.integer<<SLP_TYP_SHIFT)|SLP_EN;
	// io_port_out16(acpi_fadt->pm1a_control_block,pm1a_value);
	// if (acpi_fadt->pm1b_control_block){
	// 	io_port_out16(acpi_fadt->pm1b_control_block,pm1b_value);
	// }
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
