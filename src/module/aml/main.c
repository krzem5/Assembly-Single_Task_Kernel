#include <aml/namespace.h>
#include <aml/object.h>
#include <aml/opcode.h>
#include <kernel/acpi/fadt.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/module/module.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "aml"



// opcode parser table: https://github.com/krzem5/Assembly-Single_User_Kernel/blob/9687e45bd042c74e0f7212cb1ed52193084c1349/src/module/aml/aml/parser.c#L35-L149



typedef struct _AML_READER_CONTEXT{
	const u8*const data;
	const u64 length;
	aml_namespace_t* namespace;
	u64 offset;
} aml_reader_context_t;



typedef aml_object_t* (*opcode_func_t)(aml_reader_context_t*);



static aml_object_t* _execute_aml_single(aml_reader_context_t* ctx);



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
	u16 out=(ctx->data[ctx->offset+1]<<8)|ctx->data[ctx->offset];
	ctx->offset+=2;
	return out;
}



static u32 _get_uint32(aml_reader_context_t* ctx){
	u32 out=(ctx->data[ctx->offset+3]<<24)|(ctx->data[ctx->offset+2]<<16)|(ctx->data[ctx->offset+1]<<8)|ctx->data[ctx->offset];
	ctx->offset+=4;
	return out;
}



static aml_object_t* _exec_opcode_zero(aml_reader_context_t* ctx){
	return aml_object_alloc_integer(0);
}



static aml_object_t* _exec_opcode_one(aml_reader_context_t* ctx){
	return aml_object_alloc_integer(1);
}



static aml_object_t* _exec_opcode_alias(aml_reader_context_t* ctx){
	ERROR("alias");
	return NULL;
}



static aml_object_t* _exec_opcode_name(aml_reader_context_t* ctx){
	string_t* name=_get_name(ctx);
	aml_object_t* value=_execute_aml_single(ctx);
	if (!value){
		return NULL;
	}
	if (value->type==AML_OBJECT_TYPE_NONE){
		panic("_exec_opcode_name: value is not an object");
	}
	aml_namespace_lookup(ctx->namespace,name->data,AML_NAMESPACE_LOOKUP_FLAG_CREATE|AML_NAMESPACE_LOOKUP_FLAG_CLEAR)->value=value;
	return aml_object_alloc_none();
}



static aml_object_t* _exec_opcode_byte_prefix(aml_reader_context_t* ctx){
	return aml_object_alloc_integer(_get_uint8(ctx));
}



static aml_object_t* _exec_opcode_word_prefix(aml_reader_context_t* ctx){
	return aml_object_alloc_integer(_get_uint16(ctx));
}



static aml_object_t* _exec_opcode_dword_prefix(aml_reader_context_t* ctx){
	return aml_object_alloc_integer(_get_uint32(ctx));
}



static aml_object_t* _exec_opcode_string_prefix(aml_reader_context_t* ctx){
	u32 length=0;
	for (;ctx->data[ctx->offset+length];length++);
	aml_object_t* out=aml_object_alloc_string(smm_alloc((const char*)(ctx->data+ctx->offset),length));
	ctx->offset+=length+1;
	return out;
}



static aml_object_t* _exec_opcode_qword_prefix(aml_reader_context_t* ctx){
	ERROR("qword_prefix");
	return NULL;
}



static aml_object_t* _exec_opcode_scope(aml_reader_context_t* ctx){
	u64 end_offset=ctx->offset+_get_pkglength(ctx);
	string_t* name=_get_name(ctx);
	aml_reader_context_t child_ctx={
		ctx->data+ctx->offset,
		end_offset-ctx->offset,
		aml_namespace_lookup(ctx->namespace,name->data,AML_NAMESPACE_LOOKUP_FLAG_CREATE)
	};
	smm_dealloc(name);
	if (!_execute_aml(&child_ctx)){
		return NULL;
	}
	ctx->offset=end_offset;
	return aml_object_alloc_none();
}



static aml_object_t* _exec_opcode_buffer(aml_reader_context_t* ctx){
	u64 end_offset=ctx->offset+_get_pkglength(ctx);
	aml_object_t* buffer_size=_execute_aml_single(ctx);
	if (!buffer_size){
		return NULL;
	}
	if (buffer_size->type!=AML_OBJECT_TYPE_INTEGER){
		panic("_exec_opcode_name: buffer_size is not an integer");
	}
	string_t* buffer=smm_alloc(NULL,buffer_size->integer);
	u64 size=end_offset-ctx->offset;
	if (size>buffer_size->integer){
		size=buffer_size->integer;
	}
	memcpy(buffer->data,ctx->data+ctx->offset,size);
	ctx->offset=end_offset;
	return aml_object_alloc_buffer(buffer);
}



static aml_object_t* _exec_opcode_package(aml_reader_context_t* ctx){
	u64 end_offset=ctx->offset+_get_pkglength(ctx);
	u8 num_elements=_get_uint8(ctx);
	aml_reader_context_t child_ctx={
		ctx->data+ctx->offset,
		end_offset-ctx->offset,
		NULL,
		0
	};
	aml_object_t* out=aml_object_alloc_package(num_elements);
	u8 i=0;
	while (child_ctx.offset<child_ctx.length){
		aml_object_t* value=_execute_aml_single(&child_ctx);
		if (!value){
			return NULL;
		}
		if (i>=num_elements){
			aml_object_dealloc(value);
		}
		else{
			out->package.data[i]=value;
			i++;
		}
	}
	for (;i<num_elements;i++){
		out->package.data[i]=aml_object_alloc_integer(0);
	}
	ctx->offset=end_offset;
	return out;
}



static aml_object_t* _exec_opcode_var_package(aml_reader_context_t* ctx){
	ERROR("var_package");
	return NULL;
}



static aml_object_t* _exec_opcode_method(aml_reader_context_t* ctx){
	u64 end_offset=ctx->offset+_get_pkglength(ctx);
	string_t* name=_get_name(ctx);
	aml_namespace_lookup(ctx->namespace,name->data,AML_NAMESPACE_LOOKUP_FLAG_CREATE|AML_NAMESPACE_LOOKUP_FLAG_CLEAR)->value=aml_object_alloc_method(_get_uint8(ctx),ctx->data+ctx->offset,end_offset-ctx->offset);
	smm_dealloc(name);
	ctx->offset=end_offset;
	return aml_object_alloc_none();
}



static aml_object_t* _exec_opcode_name_reference(aml_reader_context_t* ctx){
	ctx->offset--;
	string_t* name=_get_name(ctx);
	ERROR("name_reference: %s",name->data);
	return aml_object_alloc_none();
}



static aml_object_t* _exec_opcode_local0(aml_reader_context_t* ctx){
	ERROR("local0");
	return NULL;
}



static aml_object_t* _exec_opcode_local1(aml_reader_context_t* ctx){
	ERROR("local1");
	return NULL;
}



static aml_object_t* _exec_opcode_local2(aml_reader_context_t* ctx){
	ERROR("local2");
	return NULL;
}



static aml_object_t* _exec_opcode_local3(aml_reader_context_t* ctx){
	ERROR("local3");
	return NULL;
}



static aml_object_t* _exec_opcode_local4(aml_reader_context_t* ctx){
	ERROR("local4");
	return NULL;
}



static aml_object_t* _exec_opcode_local5(aml_reader_context_t* ctx){
	ERROR("local5");
	return NULL;
}



static aml_object_t* _exec_opcode_local6(aml_reader_context_t* ctx){
	ERROR("local6");
	return NULL;
}



static aml_object_t* _exec_opcode_local7(aml_reader_context_t* ctx){
	ERROR("local7");
	return NULL;
}



static aml_object_t* _exec_opcode_arg0(aml_reader_context_t* ctx){
	ERROR("arg0");
	return NULL;
}



static aml_object_t* _exec_opcode_arg1(aml_reader_context_t* ctx){
	ERROR("arg1");
	return NULL;
}



static aml_object_t* _exec_opcode_arg2(aml_reader_context_t* ctx){
	ERROR("arg2");
	return NULL;
}



static aml_object_t* _exec_opcode_arg3(aml_reader_context_t* ctx){
	ERROR("arg3");
	return NULL;
}



static aml_object_t* _exec_opcode_arg4(aml_reader_context_t* ctx){
	ERROR("arg4");
	return NULL;
}



static aml_object_t* _exec_opcode_arg5(aml_reader_context_t* ctx){
	ERROR("arg5");
	return NULL;
}



static aml_object_t* _exec_opcode_arg6(aml_reader_context_t* ctx){
	ERROR("arg6");
	return NULL;
}



static aml_object_t* _exec_opcode_store(aml_reader_context_t* ctx){
	ERROR("store");
	return NULL;
}



static aml_object_t* _exec_opcode_ref_of(aml_reader_context_t* ctx){
	ERROR("ref_of");
	return NULL;
}



static aml_object_t* _exec_opcode_add(aml_reader_context_t* ctx){
	ERROR("add");
	return NULL;
}



static aml_object_t* _exec_opcode_concat(aml_reader_context_t* ctx){
	ERROR("concat");
	return NULL;
}



static aml_object_t* _exec_opcode_subtract(aml_reader_context_t* ctx){
	ERROR("subtract");
	return NULL;
}



static aml_object_t* _exec_opcode_increment(aml_reader_context_t* ctx){
	ERROR("increment");
	return NULL;
}



static aml_object_t* _exec_opcode_decrement(aml_reader_context_t* ctx){
	ERROR("decrement");
	return NULL;
}



static aml_object_t* _exec_opcode_multiply(aml_reader_context_t* ctx){
	ERROR("multiply");
	return NULL;
}



static aml_object_t* _exec_opcode_divide(aml_reader_context_t* ctx){
	ERROR("divide");
	return NULL;
}



static aml_object_t* _exec_opcode_shift_left(aml_reader_context_t* ctx){
	ERROR("shift_left");
	return NULL;
}



static aml_object_t* _exec_opcode_shift_right(aml_reader_context_t* ctx){
	ERROR("shift_right");
	return NULL;
}



static aml_object_t* _exec_opcode_and(aml_reader_context_t* ctx){
	ERROR("and");
	return NULL;
}



static aml_object_t* _exec_opcode_nand(aml_reader_context_t* ctx){
	ERROR("nand");
	return NULL;
}



static aml_object_t* _exec_opcode_or(aml_reader_context_t* ctx){
	ERROR("or");
	return NULL;
}



static aml_object_t* _exec_opcode_nor(aml_reader_context_t* ctx){
	ERROR("nor");
	return NULL;
}



static aml_object_t* _exec_opcode_xor(aml_reader_context_t* ctx){
	ERROR("xor");
	return NULL;
}



static aml_object_t* _exec_opcode_not(aml_reader_context_t* ctx){
	ERROR("not");
	return NULL;
}



static aml_object_t* _exec_opcode_find_set_left_bit(aml_reader_context_t* ctx){
	ERROR("find_set_left_bit");
	return NULL;
}



static aml_object_t* _exec_opcode_find_set_right_bit(aml_reader_context_t* ctx){
	ERROR("find_set_right_bit");
	return NULL;
}



static aml_object_t* _exec_opcode_deref_of(aml_reader_context_t* ctx){
	ERROR("deref_of");
	return NULL;
}



static aml_object_t* _exec_opcode_concat_res(aml_reader_context_t* ctx){
	ERROR("concat_res");
	return NULL;
}



static aml_object_t* _exec_opcode_mod(aml_reader_context_t* ctx){
	ERROR("mod");
	return NULL;
}



static aml_object_t* _exec_opcode_notify(aml_reader_context_t* ctx){
	ERROR("notify");
	return NULL;
}



static aml_object_t* _exec_opcode_size_of(aml_reader_context_t* ctx){
	ERROR("size_of");
	return NULL;
}



static aml_object_t* _exec_opcode_index(aml_reader_context_t* ctx){
	ERROR("index");
	return NULL;
}



static aml_object_t* _exec_opcode_match(aml_reader_context_t* ctx){
	ERROR("match");
	return NULL;
}



static aml_object_t* _exec_opcode_create_dword_field(aml_reader_context_t* ctx){
	ERROR("create_dword_field");
	return NULL;
}



static aml_object_t* _exec_opcode_create_word_field(aml_reader_context_t* ctx){
	ERROR("create_word_field");
	return NULL;
}



static aml_object_t* _exec_opcode_create_byte_field(aml_reader_context_t* ctx){
	ERROR("create_byte_field");
	return NULL;
}



static aml_object_t* _exec_opcode_create_bit_field(aml_reader_context_t* ctx){
	ERROR("create_bit_field");
	return NULL;
}



static aml_object_t* _exec_opcode_object_type(aml_reader_context_t* ctx){
	ERROR("object_type");
	return NULL;
}



static aml_object_t* _exec_opcode_create_qword_field(aml_reader_context_t* ctx){
	ERROR("create_qword_field");
	return NULL;
}



static aml_object_t* _exec_opcode_l_and(aml_reader_context_t* ctx){
	ERROR("l_and");
	return NULL;
}



static aml_object_t* _exec_opcode_l_or(aml_reader_context_t* ctx){
	ERROR("l_or");
	return NULL;
}



static aml_object_t* _exec_opcode_l_not(aml_reader_context_t* ctx){
	ERROR("l_not");
	return NULL;
}



static aml_object_t* _exec_opcode_l_equal(aml_reader_context_t* ctx){
	ERROR("l_equal");
	return NULL;
}



static aml_object_t* _exec_opcode_l_greater(aml_reader_context_t* ctx){
	ERROR("l_greater");
	return NULL;
}



static aml_object_t* _exec_opcode_l_less(aml_reader_context_t* ctx){
	ERROR("l_less");
	return NULL;
}



static aml_object_t* _exec_opcode_to_buffer(aml_reader_context_t* ctx){
	ERROR("to_buffer");
	return NULL;
}



static aml_object_t* _exec_opcode_to_decimal_string(aml_reader_context_t* ctx){
	ERROR("to_decimal_string");
	return NULL;
}



static aml_object_t* _exec_opcode_to_hex_string(aml_reader_context_t* ctx){
	ERROR("to_hex_string");
	return NULL;
}



static aml_object_t* _exec_opcode_to_integer(aml_reader_context_t* ctx){
	ERROR("to_integer");
	return NULL;
}



static aml_object_t* _exec_opcode_to_string(aml_reader_context_t* ctx){
	ERROR("to_string");
	return NULL;
}



static aml_object_t* _exec_opcode_copy_object(aml_reader_context_t* ctx){
	ERROR("copy_object");
	return NULL;
}



static aml_object_t* _exec_opcode_mid(aml_reader_context_t* ctx){
	ERROR("mid");
	return NULL;
}



static aml_object_t* _exec_opcode_continue(aml_reader_context_t* ctx){
	ERROR("continue");
	return NULL;
}



static aml_object_t* _exec_opcode_if(aml_reader_context_t* ctx){
	ERROR("if");
	return NULL;
}



static aml_object_t* _exec_opcode_else(aml_reader_context_t* ctx){
	ERROR("else");
	return NULL;
}



static aml_object_t* _exec_opcode_while(aml_reader_context_t* ctx){
	ERROR("while");
	return NULL;
}



static aml_object_t* _exec_opcode_noop(aml_reader_context_t* ctx){
	ERROR("noop");
	return NULL;
}



static aml_object_t* _exec_opcode_return(aml_reader_context_t* ctx){
	ERROR("return");
	return NULL;
}



static aml_object_t* _exec_opcode_break(aml_reader_context_t* ctx){
	ERROR("break");
	return NULL;
}



static aml_object_t* _exec_opcode_break_point(aml_reader_context_t* ctx){
	ERROR("break_point");
	return NULL;
}



static aml_object_t* _exec_opcode_ones(aml_reader_context_t* ctx){
	return aml_object_alloc_integer(0xffffffffffffffffull);
}



static aml_object_t* _exec_opcode_ext_mutex(aml_reader_context_t* ctx){
	string_t* name=_get_name(ctx);
	u8 sync_flags=_get_uint8(ctx);
	aml_namespace_lookup(ctx->namespace,name->data,AML_NAMESPACE_LOOKUP_FLAG_CREATE|AML_NAMESPACE_LOOKUP_FLAG_CLEAR)->value=aml_object_alloc_mutex(sync_flags);
	return aml_object_alloc_none();
}



static aml_object_t* _exec_opcode_ext_event(aml_reader_context_t* ctx){
	ERROR("ext_event");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_cond_ref_of(aml_reader_context_t* ctx){
	ERROR("ext_cond_ref_of");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_create_field(aml_reader_context_t* ctx){
	ERROR("ext_create_field");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_load_table(aml_reader_context_t* ctx){
	ERROR("ext_load_table");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_load(aml_reader_context_t* ctx){
	ERROR("ext_load");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_stall(aml_reader_context_t* ctx){
	ERROR("ext_stall");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_sleep(aml_reader_context_t* ctx){
	ERROR("ext_sleep");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_acquire(aml_reader_context_t* ctx){
	ERROR("ext_acquire");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_signal(aml_reader_context_t* ctx){
	ERROR("ext_signal");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_wait(aml_reader_context_t* ctx){
	ERROR("ext_wait");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_reset(aml_reader_context_t* ctx){
	ERROR("ext_reset");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_release(aml_reader_context_t* ctx){
	ERROR("ext_release");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_from_bcd(aml_reader_context_t* ctx){
	ERROR("ext_from_bcd");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_to_bcd(aml_reader_context_t* ctx){
	ERROR("ext_to_bcd");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_unload(aml_reader_context_t* ctx){
	ERROR("ext_unload");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_revision(aml_reader_context_t* ctx){
	ERROR("ext_revision");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_debug(aml_reader_context_t* ctx){
	ERROR("ext_debug");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_fatal(aml_reader_context_t* ctx){
	ERROR("ext_fatal");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_timer(aml_reader_context_t* ctx){
	ERROR("ext_timer");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_region(aml_reader_context_t* ctx){
	string_t* name=_get_name(ctx);
	u8 region_space=_get_uint8(ctx);
	aml_object_t* region_offset=_execute_aml_single(ctx);
	if (!region_offset){
		return NULL;
	}
	if (region_offset->type!=AML_OBJECT_TYPE_INTEGER){
		panic("_exec_opcode_ext_region: region_offset is not an integer");
	}
	aml_object_t* region_length=_execute_aml_single(ctx);
	if (!region_length){
		return NULL;
	}
	if (region_length->type!=AML_OBJECT_TYPE_INTEGER){
		panic("_exec_opcode_ext_region: region_length is not an integer");
	}
	aml_namespace_lookup(ctx->namespace,name->data,AML_NAMESPACE_LOOKUP_FLAG_CREATE|AML_NAMESPACE_LOOKUP_FLAG_CLEAR)->value=aml_object_alloc_region(region_space,region_offset->integer,region_length->integer);
	smm_dealloc(name);
	return aml_object_alloc_none();
}



static aml_object_t* _exec_opcode_ext_field(aml_reader_context_t* ctx){
	u64 end_offset=ctx->offset+_get_pkglength(ctx);
	string_t* name=_get_name(ctx);
	u8 field_flags=_get_uint8(ctx);
	aml_namespace_t* namespace=aml_namespace_lookup(ctx->namespace,name->data,0);
	smm_dealloc(name);
	if (!namespace){
		panic("_exec_opcode_ext_field: namespace not found");
	}
	if (!namespace->value||namespace->value->type!=AML_OBJECT_TYPE_REGION){
		panic("_exec_opcode_ext_field: namespace is not a region");
	}
	u64 offset=0;
	while (ctx->offset<end_offset){
		if (!ctx->data[ctx->offset]){
			ctx->offset++;
			offset+=_get_pkglength(ctx);
		}
		else if (ctx->data[ctx->offset]==1){
			panic("_exec_opcode_ext_field: access field");
		}
		else if (ctx->data[ctx->offset]==2){
			panic("_exec_opcode_ext_field: connect field");
		}
		else if (ctx->data[ctx->offset]==3){
			panic("_exec_opcode_ext_field: extended access field");
		}
		else{
			char name[5]={
				ctx->data[ctx->offset],
				ctx->data[ctx->offset+1],
				ctx->data[ctx->offset+2],
				ctx->data[ctx->offset+3],
				0
			};
			ctx->offset+=4;
			u32 size=_get_pkglength(ctx);
			aml_namespace_lookup(ctx->namespace,name,AML_NAMESPACE_LOOKUP_FLAG_CREATE|AML_NAMESPACE_LOOKUP_FLAG_CLEAR)->value=aml_object_alloc_field_unit(namespace->value->region.type,field_flags,namespace->value->region.address,offset,size);
			offset+=size;
		}
	}
	if (((offset+7)>>3)>namespace->value->region.length){
		WARN("_exec_opcode_ext_field: overflowing fields");
	}
	ctx->offset=end_offset;
	return aml_object_alloc_none();
}



static aml_object_t* _exec_opcode_ext_device(aml_reader_context_t* ctx){
	u64 end_offset=ctx->offset+_get_pkglength(ctx);
	string_t* name=_get_name(ctx);
	aml_reader_context_t child_ctx={
		ctx->data+ctx->offset,
		end_offset-ctx->offset,
		aml_namespace_lookup(ctx->namespace,name->data,AML_NAMESPACE_LOOKUP_FLAG_CREATE|AML_NAMESPACE_LOOKUP_FLAG_CLEAR)
	};
	smm_dealloc(name);
	if (!_execute_aml(&child_ctx)){
		return NULL;
	}
	ctx->offset=end_offset;
	return aml_object_alloc_none();
}



static aml_object_t* _exec_opcode_ext_processor(aml_reader_context_t* ctx){
	u64 end_offset=ctx->offset+_get_pkglength(ctx);
	string_t* name=_get_name(ctx);
	u8 processor_id=_get_uint8(ctx);
	u32 processor_block_address=_get_uint32(ctx);
	u8 processor_block_length=_get_uint8(ctx);
	aml_reader_context_t child_ctx={
		ctx->data+ctx->offset,
		end_offset-ctx->offset,
		aml_namespace_lookup(ctx->namespace,name->data,AML_NAMESPACE_LOOKUP_FLAG_CREATE|AML_NAMESPACE_LOOKUP_FLAG_CLEAR)
	};
	child_ctx.namespace->value=aml_object_alloc_processor(processor_id,processor_block_address,processor_block_length);
	smm_dealloc(name);
	if (!_execute_aml(&child_ctx)){
		return NULL;
	}
	ctx->offset=end_offset;
	return aml_object_alloc_none();
}



static aml_object_t* _exec_opcode_ext_power_res(aml_reader_context_t* ctx){
	ERROR("ext_power_res");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_thermal_zone(aml_reader_context_t* ctx){
	ERROR("ext_thermal_zone");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_index_field(aml_reader_context_t* ctx){
	ERROR("ext_index_field");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_bank_field(aml_reader_context_t* ctx){
	ERROR("ext_bank_field");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_data_region(aml_reader_context_t* ctx){
	ERROR("ext_data_region");
	return NULL;
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



static aml_object_t* _execute_aml_single(aml_reader_context_t* ctx){
	const opcode_func_t* table=_aml_opcodes;
	if (ctx->data[ctx->offset]==AML_EXTENDED_OPCODE){
		ctx->offset++;
		table=_aml_extended_opcodes;
	}
	opcode_func_t out=table[ctx->data[ctx->offset]];
	if (out){
		ctx->offset++;
		return out(ctx);
	}
	ERROR("Unknown AML opcode '%s%X'",(table==_aml_extended_opcodes?"5b ":""),ctx->data[ctx->offset]);
	return NULL;
}



static _Bool _execute_aml(aml_reader_context_t* ctx){
	ctx->offset=0;
	while (ctx->offset<ctx->length){
		aml_object_t* value=_execute_aml_single(ctx);
		if (!value){
			return 0;
		}
		aml_object_dealloc(value);
	}
	return 1;
}



static void _print_namespace_recursive(aml_namespace_t* namespace,u32 indent){
	for (u32 i=0;i<indent;i++){
		log(" ");
	}
	log("%s\n",namespace->name);
	for (aml_namespace_t* child=namespace->first_child;child;child=child->next_sibling){
		_print_namespace_recursive(child,indent+2);
	}
}



static _Bool _init(module_t* module){
	if (!acpi_dsdt||!acpi_dsdt->header.length){
		WARN("No AML code present");
		return 0;
	}
	aml_reader_context_t ctx={
		acpi_dsdt->data,
		acpi_dsdt->header.length-sizeof(acpi_dsdt_t),
		NULL
	};
	_execute_aml(&ctx);
	_print_namespace_recursive(aml_namespace_lookup(NULL,"\\",0),0);
	// panic("test");
	// INFO("Registering AML IRQ...");
	// ioapic_redirect_irq(fadt->sci_int,isr_allocate());
	// // PM1x flags
	// #define SLP_TYP_SHIFT 10
	// #define SLP_EN 0x2000
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
