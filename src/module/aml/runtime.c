#include <aml/field.h>
#include <aml/namespace.h>
#include <aml/object.h>
#include <aml/opcode.h>
#include <aml/runtime.h>
#include <kernel/apic/ioapic.h>
#include <kernel/isr/isr.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/smm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "aml_runtime"



static u32 _get_pkglength(aml_runtime_context_t* ctx){
	u32 out=ctx->data[ctx->offset]&0x3f;
	for (u8 i=0;i<(ctx->data[ctx->offset]>>6);i++){
		out|=ctx->data[ctx->offset+i+1]<<(4+(i<<3));
	}
	ctx->offset+=(ctx->data[ctx->offset]>>6)+1;
	return out;
}



static string_t* _get_name(aml_runtime_context_t* ctx){
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



static u8 _get_uint8(aml_runtime_context_t* ctx){
	u8 out=ctx->data[ctx->offset];
	ctx->offset++;
	return out;
}



static u16 _get_uint16(aml_runtime_context_t* ctx){
	u16 out=(ctx->data[ctx->offset+1]<<8)|ctx->data[ctx->offset];
	ctx->offset+=2;
	return out;
}



static u32 _get_uint32(aml_runtime_context_t* ctx){
	u32 out=(ctx->data[ctx->offset+3]<<24)|(ctx->data[ctx->offset+2]<<16)|(ctx->data[ctx->offset+1]<<8)|ctx->data[ctx->offset];
	ctx->offset+=4;
	return out;
}



static _Bool _store_value(aml_object_t* value,aml_runtime_context_t* ctx){
	u8 type=ctx->data[ctx->offset];
	if (!type){
		ctx->offset++;
		aml_object_dealloc(value);
		return 1;
	}
	if (AML_OPCODE_LOCAL0<=type&&type<=AML_OPCODE_LOCAL7){
		ctx->offset++;
		aml_object_dealloc(ctx->vars->locals[type-AML_OPCODE_LOCAL0]);
		ctx->vars->locals[type-AML_OPCODE_LOCAL0]=value;
		return 1;
	}
	if (type=='\\'||type=='^'||type=='.'||type=='/'||(type>47&&type<58)||type=='_'||(type>64&&type<91)){
		string_t* name=_get_name(ctx);
		aml_namespace_t* container=aml_namespace_lookup(ctx->namespace,name->data,0);
		if (!container||!container->value){
			ERROR("_store_value: object '%s' not found",name->data);
			smm_dealloc(name);
			return 0;
		}
		smm_dealloc(name);
		if (container->value->type==AML_OBJECT_TYPE_FIELD_UNIT){
			return aml_field_write(container->value,value);
		}
		ERROR("_store_value: container of type %u",container->value->type);
		return 1;
	}
	if (type==AML_OPCODE_INDEX){
		ctx->offset++;
		aml_object_t* container=aml_runtime_execute_single(ctx);
		if (!container){
			return 0;
		}
		aml_object_t* index=aml_runtime_execute_single(ctx);
		if (!index){
			return 0;
		}
		if (index->type!=AML_OBJECT_TYPE_INTEGER){
			ERROR("_store_value: index is not an integer");
			goto _cleanup_index_store;
		}
		if (container->type==AML_OBJECT_TYPE_BUFFER){
			panic("AML_OBJECT_TYPE_BUFFER");
			return 1;
		}
		if (container->type==AML_OBJECT_TYPE_STRING){
			panic("AML_OBJECT_TYPE_STRING");
			return 1;
		}
		if (container->type==AML_OBJECT_TYPE_PACKAGE){
			if (index->integer>=container->package.length){
				ERROR("_store_value: index of out range");
				goto _cleanup_index_store;
			}
			aml_object_dealloc(container->package.data[index->integer]);
			container->package.data[index->integer]=value;
			return 1;
		}
		ERROR("_store_value: container is not a container");
_cleanup_index_store:
		aml_object_dealloc(container);
		aml_object_dealloc(index);
		return 0;
	}
	ERROR("_store_value: unable to store value to opcode [%X %X]",ctx->data[ctx->offset],ctx->data[ctx->offset+1]);
	return 0;
}



static aml_object_t* _exec_opcode_zero(aml_runtime_context_t* ctx){
	return aml_object_alloc_integer(0);
}



static aml_object_t* _exec_opcode_one(aml_runtime_context_t* ctx){
	return aml_object_alloc_integer(1);
}



static aml_object_t* _exec_opcode_alias(aml_runtime_context_t* ctx){
	ERROR("alias");
	return NULL;
}



static aml_object_t* _exec_opcode_name(aml_runtime_context_t* ctx){
	string_t* name=_get_name(ctx);
	aml_object_t* value=aml_runtime_execute_single(ctx);
	if (!value){
		return NULL;
	}
	if (value->type==AML_OBJECT_TYPE_NONE){
		ERROR("_exec_opcode_name: value is not an object");
		return NULL;
	}
	aml_namespace_lookup(ctx->namespace,name->data,AML_NAMESPACE_LOOKUP_FLAG_CREATE|AML_NAMESPACE_LOOKUP_FLAG_CLEAR)->value=value;
	return aml_object_alloc_none();
}



static aml_object_t* _exec_opcode_byte_prefix(aml_runtime_context_t* ctx){
	return aml_object_alloc_integer(_get_uint8(ctx));
}



static aml_object_t* _exec_opcode_word_prefix(aml_runtime_context_t* ctx){
	return aml_object_alloc_integer(_get_uint16(ctx));
}



static aml_object_t* _exec_opcode_dword_prefix(aml_runtime_context_t* ctx){
	return aml_object_alloc_integer(_get_uint32(ctx));
}



static aml_object_t* _exec_opcode_string_prefix(aml_runtime_context_t* ctx){
	u32 length=0;
	for (;ctx->data[ctx->offset+length];length++);
	aml_object_t* out=aml_object_alloc_string(smm_alloc((const char*)(ctx->data+ctx->offset),length));
	ctx->offset+=length+1;
	return out;
}



static aml_object_t* _exec_opcode_qword_prefix(aml_runtime_context_t* ctx){
	ERROR("qword_prefix");
	return NULL;
}



static aml_object_t* _exec_opcode_scope(aml_runtime_context_t* ctx){
	u64 end_offset=ctx->offset+_get_pkglength(ctx);
	string_t* name=_get_name(ctx);
	aml_runtime_context_t child_ctx={
		ctx->data+ctx->offset,
		end_offset-ctx->offset,
		aml_namespace_lookup(ctx->namespace,name->data,AML_NAMESPACE_LOOKUP_FLAG_CREATE),
		NULL
	};
	smm_dealloc(name);
	if (!aml_runtime_execute(&child_ctx)){
		return NULL;
	}
	ctx->offset=end_offset;
	return aml_object_alloc_none();
}



static aml_object_t* _exec_opcode_buffer(aml_runtime_context_t* ctx){
	u64 end_offset=ctx->offset+_get_pkglength(ctx);
	aml_object_t* buffer_size=aml_runtime_execute_single(ctx);
	if (!buffer_size){
		return NULL;
	}
	if (buffer_size->type!=AML_OBJECT_TYPE_INTEGER){
		ERROR("_exec_opcode_name: buffer_size is not an integer");
		return NULL;
	}
	u8* buffer=amm_alloc(buffer_size->integer);
	u32 size=end_offset-ctx->offset;
	if (size>buffer_size->integer){
		size=buffer_size->integer;
	}
	memcpy(buffer,ctx->data+ctx->offset,size);
	ctx->offset=end_offset;
	return aml_object_alloc_buffer(size,buffer);
}



static aml_object_t* _exec_opcode_package(aml_runtime_context_t* ctx){
	u64 end_offset=ctx->offset+_get_pkglength(ctx);
	u8 num_elements=_get_uint8(ctx);
	aml_runtime_context_t child_ctx={
		ctx->data+ctx->offset,
		end_offset-ctx->offset,
		ctx->namespace,
		NULL,
		0
	};
	aml_object_t* out=aml_object_alloc_package(num_elements);
	u8 i=0;
	while (child_ctx.offset<child_ctx.length){
		aml_object_t* value=aml_runtime_execute_single(&child_ctx);
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



static aml_object_t* _exec_opcode_var_package(aml_runtime_context_t* ctx){
	ERROR("var_package");
	return NULL;
}



static aml_object_t* _exec_opcode_method(aml_runtime_context_t* ctx){
	u64 end_offset=ctx->offset+_get_pkglength(ctx);
	string_t* name=_get_name(ctx);
	u8 flags=_get_uint8(ctx);
	aml_namespace_t* namespace=aml_namespace_lookup(ctx->namespace,name->data,AML_NAMESPACE_LOOKUP_FLAG_CREATE|AML_NAMESPACE_LOOKUP_FLAG_CLEAR);
	namespace->value=aml_object_alloc_method(flags,ctx->data+ctx->offset,end_offset-ctx->offset,namespace);
	smm_dealloc(name);
	ctx->offset=end_offset;
	return aml_object_alloc_none();
}



static aml_object_t* _exec_opcode_name_reference(aml_runtime_context_t* ctx){
	ctx->offset--;
	string_t* name=_get_name(ctx);
	aml_namespace_t* value=aml_namespace_lookup(ctx->namespace,name->data,0);
	if (!value||!value->value){
		ERROR("_exec_opcode_name_reference: object '%s' not found",name->data);
		for (;;);
		smm_dealloc(name);
		return NULL;
	}
	smm_dealloc(name);
	if (value->value->type==AML_OBJECT_TYPE_FIELD_UNIT){
		return aml_field_read(value->value);
	}
	if (value->value->type!=AML_OBJECT_TYPE_METHOD){
		value->value->rc++;
		return value->value;
	}
	u8 arg_count=value->value->method.flags&7;
	aml_object_t* args[7]={NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	for (u8 i=0;i<arg_count;i++){
		args[i]=aml_runtime_execute_single(ctx);
		if (args[i]){
			continue;
		}
		while (i){
			i--;
			aml_object_dealloc(args[i]);
		}
		return NULL;
	}
	aml_object_t* new_value=aml_runtime_execute_method(value->value,arg_count,args);
	for (u8 i=0;i<arg_count;i++){
		// aml_object_dealloc(args[i]);
	}
	return new_value;
}



static aml_object_t* _exec_opcode_local0(aml_runtime_context_t* ctx){
	if (!ctx->vars){
		ERROR("_exec_opcode_local0: local0 outside of method");
		return NULL;
	}
	ctx->vars->locals[0]->rc++;
	return ctx->vars->locals[0];
}



static aml_object_t* _exec_opcode_local1(aml_runtime_context_t* ctx){
	if (!ctx->vars){
		ERROR("_exec_opcode_local1: local1 outside of method");
		return NULL;
	}
	ctx->vars->locals[1]->rc++;
	return ctx->vars->locals[1];
}



static aml_object_t* _exec_opcode_local2(aml_runtime_context_t* ctx){
	if (!ctx->vars){
		ERROR("_exec_opcode_local2: local2 outside of method");
		return NULL;
	}
	ctx->vars->locals[2]->rc++;
	return ctx->vars->locals[2];
}



static aml_object_t* _exec_opcode_local3(aml_runtime_context_t* ctx){
	if (!ctx->vars){
		ERROR("_exec_opcode_local3: local3 outside of method");
		return NULL;
	}
	ctx->vars->locals[3]->rc++;
	return ctx->vars->locals[3];
}



static aml_object_t* _exec_opcode_local4(aml_runtime_context_t* ctx){
	if (!ctx->vars){
		ERROR("_exec_opcode_local4: local4 outside of method");
		return NULL;
	}
	ctx->vars->locals[4]->rc++;
	return ctx->vars->locals[4];
}



static aml_object_t* _exec_opcode_local5(aml_runtime_context_t* ctx){
	if (!ctx->vars){
		ERROR("_exec_opcode_local5: local5 outside of method");
		return NULL;
	}
	ctx->vars->locals[5]->rc++;
	return ctx->vars->locals[5];
}



static aml_object_t* _exec_opcode_local6(aml_runtime_context_t* ctx){
	if (!ctx->vars){
		ERROR("_exec_opcode_local6: local6 outside of method");
		return NULL;
	}
	ctx->vars->locals[6]->rc++;
	return ctx->vars->locals[6];
}



static aml_object_t* _exec_opcode_local7(aml_runtime_context_t* ctx){
	if (!ctx->vars){
		ERROR("_exec_opcode_local7: local7 outside of method");
		return NULL;
	}
	ctx->vars->locals[7]->rc++;
	return ctx->vars->locals[7];
}



static aml_object_t* _exec_opcode_arg0(aml_runtime_context_t* ctx){
	if (!ctx->vars){
		ERROR("_exec_opcode_arg0: arg0 outside of method");
		return NULL;
	}
	ctx->vars->args[0]->rc++;
	return ctx->vars->args[0];
}



static aml_object_t* _exec_opcode_arg1(aml_runtime_context_t* ctx){
	if (!ctx->vars){
		ERROR("_exec_opcode_arg1: arg1 outside of method");
		return NULL;
	}
	ctx->vars->args[1]->rc++;
	return ctx->vars->args[1];
}



static aml_object_t* _exec_opcode_arg2(aml_runtime_context_t* ctx){
	if (!ctx->vars){
		ERROR("_exec_opcode_arg2: arg2 outside of method");
		return NULL;
	}
	ctx->vars->args[2]->rc++;
	return ctx->vars->args[2];
}



static aml_object_t* _exec_opcode_arg3(aml_runtime_context_t* ctx){
	if (!ctx->vars){
		ERROR("_exec_opcode_arg3: arg3 outside of method");
		return NULL;
	}
	ctx->vars->args[3]->rc++;
	return ctx->vars->args[3];
}



static aml_object_t* _exec_opcode_arg4(aml_runtime_context_t* ctx){
	if (!ctx->vars){
		ERROR("_exec_opcode_arg4: arg4 outside of method");
		return NULL;
	}
	ctx->vars->args[4]->rc++;
	return ctx->vars->args[4];
}



static aml_object_t* _exec_opcode_arg5(aml_runtime_context_t* ctx){
	if (!ctx->vars){
		ERROR("_exec_opcode_arg5: arg5 outside of method");
		return NULL;
	}
	ctx->vars->args[5]->rc++;
	return ctx->vars->args[5];
}



static aml_object_t* _exec_opcode_arg6(aml_runtime_context_t* ctx){
	if (!ctx->vars){
		ERROR("_exec_opcode_arg6: arg6 outside of method");
		return NULL;
	}
	ctx->vars->args[6]->rc++;
	return ctx->vars->args[6];
}



static aml_object_t* _exec_opcode_store(aml_runtime_context_t* ctx){
	if (!ctx->vars){
		ERROR("_exec_opcode_store: store outside of method");
		return NULL;
	}
	aml_object_t* value=aml_runtime_execute_single(ctx);
	if (!value){
		return NULL;
	}
	return (_store_value(value,ctx)?aml_object_alloc_none():NULL);
}



static aml_object_t* _exec_opcode_ref_of(aml_runtime_context_t* ctx){
	ERROR("ref_of");
	return NULL;
}



static aml_object_t* _exec_opcode_add(aml_runtime_context_t* ctx){
	aml_object_t* left=aml_runtime_execute_single(ctx);
	if (!left){
		return NULL;
	}
	aml_object_t* right=aml_runtime_execute_single(ctx);
	if (!right){
		aml_object_dealloc(left);
		return NULL;
	}
	if (left->type!=AML_OBJECT_TYPE_INTEGER||right->type!=AML_OBJECT_TYPE_INTEGER){
		ERROR("_exec_opcode_add: left or right is not and integer");
		return NULL;
	}
	aml_object_t* out=aml_object_alloc_integer(left->integer+right->integer);
	aml_object_dealloc(left);
	aml_object_dealloc(right);
	out->rc++;
	if (_store_value(out,ctx)){
		return out;
	}
	aml_object_dealloc(out);
	aml_object_dealloc(out);
	return NULL;
}



static aml_object_t* _exec_opcode_concat(aml_runtime_context_t* ctx){
	ERROR("concat");
	return NULL;
}



static aml_object_t* _exec_opcode_subtract(aml_runtime_context_t* ctx){
	aml_object_t* left=aml_runtime_execute_single(ctx);
	if (!left){
		return NULL;
	}
	aml_object_t* right=aml_runtime_execute_single(ctx);
	if (!right){
		aml_object_dealloc(left);
		return NULL;
	}
	if (left->type!=AML_OBJECT_TYPE_INTEGER||right->type!=AML_OBJECT_TYPE_INTEGER){
		ERROR("_exec_opcode_subtract: left or right is not and integer");
		return NULL;
	}
	aml_object_t* out=aml_object_alloc_integer(left->integer-right->integer);
	aml_object_dealloc(left);
	aml_object_dealloc(right);
	out->rc++;
	if (_store_value(out,ctx)){
		return out;
	}
	aml_object_dealloc(out);
	aml_object_dealloc(out);
	return NULL;
}



static aml_object_t* _exec_opcode_increment(aml_runtime_context_t* ctx){
	u64 start_offset=ctx->offset;
	aml_object_t* value=aml_runtime_execute_single(ctx);
	if (!value){
		return NULL;
	}
	if (value->type!=AML_OBJECT_TYPE_INTEGER){
		ERROR("_exec_opcode_increment: value is not an integer");
		return NULL;
	}
	if (value->rc>1){
		value->integer++;
		aml_object_dealloc(value);
		return aml_object_alloc_none();
	}
	ctx->offset=start_offset;
	aml_object_t* out=aml_object_alloc_integer(value->integer+1);
	aml_object_dealloc(value);
	if (_store_value(out,ctx)){
		return aml_object_alloc_none();
	}
	aml_object_dealloc(out);
	return NULL;
}



static aml_object_t* _exec_opcode_decrement(aml_runtime_context_t* ctx){
	u64 start_offset=ctx->offset;
	aml_object_t* value=aml_runtime_execute_single(ctx);
	if (!value){
		return NULL;
	}
	if (value->type!=AML_OBJECT_TYPE_INTEGER){
		ERROR("_exec_opcode_decrement: value is not an integer");
		return NULL;
	}
	if (value->rc>1){
		value->integer--;
		aml_object_dealloc(value);
		return aml_object_alloc_none();
	}
	ctx->offset=start_offset;
	aml_object_t* out=aml_object_alloc_integer(value->integer-1);
	aml_object_dealloc(value);
	if (_store_value(out,ctx)){
		return aml_object_alloc_none();
	}
	aml_object_dealloc(out);
	return NULL;
}



static aml_object_t* _exec_opcode_multiply(aml_runtime_context_t* ctx){
	aml_object_t* left=aml_runtime_execute_single(ctx);
	if (!left){
		return NULL;
	}
	aml_object_t* right=aml_runtime_execute_single(ctx);
	if (!right){
		aml_object_dealloc(left);
		return NULL;
	}
	if (left->type!=AML_OBJECT_TYPE_INTEGER||right->type!=AML_OBJECT_TYPE_INTEGER){
		ERROR("_exec_opcode_multiply: left or right is not and integer");
		return NULL;
	}
	aml_object_t* out=aml_object_alloc_integer(left->integer*right->integer);
	aml_object_dealloc(left);
	aml_object_dealloc(right);
	out->rc++;
	if (_store_value(out,ctx)){
		return out;
	}
	aml_object_dealloc(out);
	aml_object_dealloc(out);
	return NULL;
}



static aml_object_t* _exec_opcode_divide(aml_runtime_context_t* ctx){
	ERROR("divide");
	return NULL;
}



static aml_object_t* _exec_opcode_shift_left(aml_runtime_context_t* ctx){
	aml_object_t* value=aml_runtime_execute_single(ctx);
	if (!value){
		return NULL;
	}
	aml_object_t* shift=aml_runtime_execute_single(ctx);
	if (!shift){
		aml_object_dealloc(value);
		return NULL;
	}
	if (value->type!=AML_OBJECT_TYPE_INTEGER||shift->type!=AML_OBJECT_TYPE_INTEGER){
		ERROR("_exec_opcode_shift_left: value or shift is not and integer");
		return NULL;
	}
	aml_object_t* out=aml_object_alloc_integer(value->integer<<shift->integer);
	aml_object_dealloc(value);
	aml_object_dealloc(shift);
	out->rc++;
	if (_store_value(out,ctx)){
		return out;
	}
	aml_object_dealloc(out);
	aml_object_dealloc(out);
	return NULL;
}



static aml_object_t* _exec_opcode_shift_right(aml_runtime_context_t* ctx){
	aml_object_t* value=aml_runtime_execute_single(ctx);
	if (!value){
		return NULL;
	}
	aml_object_t* shift=aml_runtime_execute_single(ctx);
	if (!shift){
		aml_object_dealloc(value);
		return NULL;
	}
	if (value->type!=AML_OBJECT_TYPE_INTEGER||shift->type!=AML_OBJECT_TYPE_INTEGER){
		ERROR("_exec_opcode_shift_right: value or shift is not and integer");
		return NULL;
	}
	aml_object_t* out=aml_object_alloc_integer(value->integer>>shift->integer);
	aml_object_dealloc(value);
	aml_object_dealloc(shift);
	out->rc++;
	if (_store_value(out,ctx)){
		return out;
	}
	aml_object_dealloc(out);
	aml_object_dealloc(out);
	return NULL;
}



static aml_object_t* _exec_opcode_and(aml_runtime_context_t* ctx){
	aml_object_t* left=aml_runtime_execute_single(ctx);
	if (!left){
		return NULL;
	}
	aml_object_t* right=aml_runtime_execute_single(ctx);
	if (!right){
		aml_object_dealloc(left);
		return NULL;
	}
	if (left->type!=AML_OBJECT_TYPE_INTEGER||right->type!=AML_OBJECT_TYPE_INTEGER){
		ERROR("_exec_opcode_and: left or right is not and integer");
		return NULL;
	}
	aml_object_t* out=aml_object_alloc_integer(left->integer&right->integer);
	aml_object_dealloc(left);
	aml_object_dealloc(right);
	out->rc++;
	if (_store_value(out,ctx)){
		return out;
	}
	aml_object_dealloc(out);
	aml_object_dealloc(out);
	return NULL;
}



static aml_object_t* _exec_opcode_nand(aml_runtime_context_t* ctx){
	aml_object_t* left=aml_runtime_execute_single(ctx);
	if (!left){
		return NULL;
	}
	aml_object_t* right=aml_runtime_execute_single(ctx);
	if (!right){
		aml_object_dealloc(left);
		return NULL;
	}
	if (left->type!=AML_OBJECT_TYPE_INTEGER||right->type!=AML_OBJECT_TYPE_INTEGER){
		ERROR("_exec_opcode_nand: left or right is not and integer");
		return NULL;
	}
	aml_object_t* out=aml_object_alloc_integer(~(left->integer&right->integer));
	aml_object_dealloc(left);
	aml_object_dealloc(right);
	out->rc++;
	if (_store_value(out,ctx)){
		return out;
	}
	aml_object_dealloc(out);
	aml_object_dealloc(out);
	return NULL;
}



static aml_object_t* _exec_opcode_or(aml_runtime_context_t* ctx){
	aml_object_t* left=aml_runtime_execute_single(ctx);
	if (!left){
		return NULL;
	}
	aml_object_t* right=aml_runtime_execute_single(ctx);
	if (!right){
		aml_object_dealloc(left);
		return NULL;
	}
	if (left->type!=AML_OBJECT_TYPE_INTEGER||right->type!=AML_OBJECT_TYPE_INTEGER){
		ERROR("_exec_opcode_or: left or right is not and integer");
		return NULL;
	}
	aml_object_t* out=aml_object_alloc_integer(left->integer|right->integer);
	aml_object_dealloc(left);
	aml_object_dealloc(right);
	out->rc++;
	if (_store_value(out,ctx)){
		return out;
	}
	aml_object_dealloc(out);
	aml_object_dealloc(out);
	return NULL;
}



static aml_object_t* _exec_opcode_nor(aml_runtime_context_t* ctx){
	aml_object_t* left=aml_runtime_execute_single(ctx);
	if (!left){
		return NULL;
	}
	aml_object_t* right=aml_runtime_execute_single(ctx);
	if (!right){
		aml_object_dealloc(left);
		return NULL;
	}
	if (left->type!=AML_OBJECT_TYPE_INTEGER||right->type!=AML_OBJECT_TYPE_INTEGER){
		ERROR("_exec_opcode_nor: left or right is not and integer");
		return NULL;
	}
	aml_object_t* out=aml_object_alloc_integer(~(left->integer|right->integer));
	aml_object_dealloc(left);
	aml_object_dealloc(right);
	out->rc++;
	if (_store_value(out,ctx)){
		return out;
	}
	aml_object_dealloc(out);
	aml_object_dealloc(out);
	return NULL;
}



static aml_object_t* _exec_opcode_xor(aml_runtime_context_t* ctx){
	aml_object_t* left=aml_runtime_execute_single(ctx);
	if (!left){
		return NULL;
	}
	aml_object_t* right=aml_runtime_execute_single(ctx);
	if (!right){
		aml_object_dealloc(left);
		return NULL;
	}
	if (left->type!=AML_OBJECT_TYPE_INTEGER||right->type!=AML_OBJECT_TYPE_INTEGER){
		ERROR("_exec_opcode_xor: left or right is not and integer");
		return NULL;
	}
	aml_object_t* out=aml_object_alloc_integer(left->integer^right->integer);
	aml_object_dealloc(left);
	aml_object_dealloc(right);
	out->rc++;
	if (_store_value(out,ctx)){
		return out;
	}
	aml_object_dealloc(out);
	aml_object_dealloc(out);
	return NULL;
}



static aml_object_t* _exec_opcode_not(aml_runtime_context_t* ctx){
	ERROR("not");
	return NULL;
}



static aml_object_t* _exec_opcode_find_set_left_bit(aml_runtime_context_t* ctx){
	ERROR("find_set_left_bit");
	return NULL;
}



static aml_object_t* _exec_opcode_find_set_right_bit(aml_runtime_context_t* ctx){
	ERROR("find_set_right_bit");
	return NULL;
}



static aml_object_t* _exec_opcode_deref_of(aml_runtime_context_t* ctx){
	ERROR("deref_of");
	return NULL;
}



static aml_object_t* _exec_opcode_concat_res(aml_runtime_context_t* ctx){
	ERROR("concat_res");
	return NULL;
}



static aml_object_t* _exec_opcode_mod(aml_runtime_context_t* ctx){
	ERROR("mod");
	return NULL;
}



static aml_object_t* _exec_opcode_notify(aml_runtime_context_t* ctx){
	ERROR("notify");
	return NULL;
}



static aml_object_t* _exec_opcode_size_of(aml_runtime_context_t* ctx){
	ERROR("size_of");
	return NULL;
}



static aml_object_t* _exec_opcode_index(aml_runtime_context_t* ctx){
	ERROR("index");
	return NULL;
}



static aml_object_t* _exec_opcode_match(aml_runtime_context_t* ctx){
	ERROR("match");
	return NULL;
}



static aml_object_t* _exec_opcode_create_dword_field(aml_runtime_context_t* ctx){
	aml_object_t* buffer=aml_runtime_execute_single(ctx);
	if (!buffer){
		return NULL;
	}
	if (buffer->type!=AML_OBJECT_TYPE_BUFFER){
		ERROR("_exec_opcode_create_dword_field: buffer is not a buffer");
		return NULL;
	}
	aml_object_t* index=aml_runtime_execute_single(ctx);
	if (!index){
		aml_object_dealloc(buffer);
		return NULL;
	}
	if (index->type!=AML_OBJECT_TYPE_INTEGER){
		ERROR("_exec_opcode_create_dword_field: index is not an integer");
		return NULL;
	}
	string_t* name=_get_name(ctx);
	aml_namespace_lookup(ctx->namespace,name->data,AML_NAMESPACE_LOOKUP_FLAG_CREATE|AML_NAMESPACE_LOOKUP_FLAG_CLEAR)->value=aml_object_alloc_field_unit(0x00,0x03,(u64)(buffer->buffer.data),index->integer<<3,32);
	smm_dealloc(name);
	aml_object_dealloc(buffer);
	aml_object_dealloc(index);
	return aml_object_alloc_none();
}



static aml_object_t* _exec_opcode_create_word_field(aml_runtime_context_t* ctx){
	ERROR("create_word_field");
	return NULL;
}



static aml_object_t* _exec_opcode_create_byte_field(aml_runtime_context_t* ctx){
	ERROR("create_byte_field");
	return NULL;
}



static aml_object_t* _exec_opcode_create_bit_field(aml_runtime_context_t* ctx){
	ERROR("create_bit_field");
	return NULL;
}



static aml_object_t* _exec_opcode_object_type(aml_runtime_context_t* ctx){
	ERROR("object_type");
	return NULL;
}



static aml_object_t* _exec_opcode_create_qword_field(aml_runtime_context_t* ctx){
	ERROR("create_qword_field");
	return NULL;
}



static aml_object_t* _exec_opcode_l_and(aml_runtime_context_t* ctx){
	ERROR("l_and");
	return NULL;
}



static aml_object_t* _exec_opcode_l_or(aml_runtime_context_t* ctx){
	aml_object_t* left=aml_runtime_execute_single(ctx);
	if (!left){
		return NULL;
	}
	if (left->type!=AML_OBJECT_TYPE_INTEGER){
		ERROR("_exec_opcode_l_or: left is not an integer");
		return NULL;
	}
	if (left->integer){
		return left;
	}
	aml_object_dealloc(left);
	aml_object_t* right=aml_runtime_execute_single(ctx);
	if (!right){
		return NULL;
	}
	if (right->type!=AML_OBJECT_TYPE_INTEGER){
		ERROR("_exec_opcode_l_or: right is not an integer");
		return NULL;
	}
	return right;
}



static aml_object_t* _exec_opcode_l_not(aml_runtime_context_t* ctx){
	ERROR("l_not");
	return NULL;
}



static aml_object_t* _exec_opcode_l_equal(aml_runtime_context_t* ctx){
	aml_object_t* left=aml_runtime_execute_single(ctx);
	if (!left){
		return NULL;
	}
	aml_object_t* right=aml_runtime_execute_single(ctx);
	if (!right){
		aml_object_dealloc(left);
		return NULL;
	}
	_Bool out=0;
	if (left->type==AML_OBJECT_TYPE_INTEGER&&right->type==AML_OBJECT_TYPE_INTEGER){
		out=(left->integer==right->integer);
	}
	else if ((left->type==AML_OBJECT_TYPE_BUFFER&&right->type==AML_OBJECT_TYPE_BUFFER)||(left->type==AML_OBJECT_TYPE_STRING&&right->type==AML_OBJECT_TYPE_STRING)){
		if (left->buffer.size==right->buffer.size){
			for (u32 i=0;i<left->buffer.size;i++){
				if (left->buffer.data[i]!=right->buffer.data[i]){
					goto _unequal_data;
				}
			}
			out=1;
_unequal_data:
		}
	}
	else{
		ERROR("_exec_opcode_l_equal: unable to compare types");
		aml_object_dealloc(left);
		aml_object_dealloc(right);
		return NULL;
	}
	aml_object_dealloc(left);
	aml_object_dealloc(right);
	return aml_object_alloc_integer(out);
}



static aml_object_t* _exec_opcode_l_greater(aml_runtime_context_t* ctx){
	aml_object_t* left=aml_runtime_execute_single(ctx);
	if (!left){
		return NULL;
	}
	aml_object_t* right=aml_runtime_execute_single(ctx);
	if (!right){
		aml_object_dealloc(left);
		return NULL;
	}
	_Bool out=0;
	if (left->type==AML_OBJECT_TYPE_INTEGER&&right->type==AML_OBJECT_TYPE_INTEGER){
		out=(left->integer>right->integer);
	}
	else{
		ERROR("_exec_opcode_l_greater: unable to compare types");
		return NULL;
	}
	aml_object_dealloc(left);
	aml_object_dealloc(right);
	return aml_object_alloc_integer(out);
}



static aml_object_t* _exec_opcode_l_less(aml_runtime_context_t* ctx){
	aml_object_t* left=aml_runtime_execute_single(ctx);
	if (!left){
		return NULL;
	}
	aml_object_t* right=aml_runtime_execute_single(ctx);
	if (!right){
		aml_object_dealloc(left);
		return NULL;
	}
	_Bool out=0;
	if (left->type==AML_OBJECT_TYPE_INTEGER&&right->type==AML_OBJECT_TYPE_INTEGER){
		out=(left->integer<right->integer);
	}
	else{
		ERROR("_exec_opcode_l_less: unable to compare types");
		return NULL;
	}
	aml_object_dealloc(left);
	aml_object_dealloc(right);
	return aml_object_alloc_integer(out);
}



static aml_object_t* _exec_opcode_to_buffer(aml_runtime_context_t* ctx){
	ERROR("to_buffer");
	return NULL;
}



static aml_object_t* _exec_opcode_to_decimal_string(aml_runtime_context_t* ctx){
	ERROR("to_decimal_string");
	return NULL;
}



static aml_object_t* _exec_opcode_to_hex_string(aml_runtime_context_t* ctx){
	ERROR("to_hex_string");
	return NULL;
}



static aml_object_t* _exec_opcode_to_integer(aml_runtime_context_t* ctx){
	aml_object_t* value=aml_runtime_execute_single(ctx);
	if (!value){
		return NULL;
	}
	aml_object_t* ret=NULL;
	if (value->type==AML_OBJECT_TYPE_INTEGER){
		ret=value;
	}
	else if (value->type==AML_OBJECT_TYPE_STRING){
		u64 out=0;
		if (!value->string->length){
			goto _skip_string_parse;
		}
		if (value->string->length==1){
			out=value->string->data[0]-48;
			goto _skip_string_parse;
		}
		if (value->string->data[0]=='0'&&value->string->data[1]=='x'){
			for (u32 i=2;i<value->string->length;i++){
				out=(out<<4)+value->string->data[i]-((75<=value->string->data[i]&&value->string->data[i]<=90)||(97<=value->string->data[i]&&value->string->data[i]<=122)?87:48);
			}
			goto _skip_string_parse;
		}
		for (u32 i=2;i<value->string->length;i++){
			out=out*10+value->string->data[i]-48;
		}
_skip_string_parse:
		aml_object_dealloc(value);
		ret=aml_object_alloc_integer(out);
	}
	else if (value->type==AML_OBJECT_TYPE_BUFFER){
		ERROR("to_integer: buffer -> integer");
		return NULL;
	}
	else{
		aml_object_print(value);
		ERROR("_exec_opcode_to_integer: unable to convert object to integer");
		return NULL;
	}
	ret->rc++;
	if (_store_value(ret,ctx)){
		return ret;
	}
	aml_object_dealloc(ret);
	aml_object_dealloc(ret);
	return NULL;
}



static aml_object_t* _exec_opcode_to_string(aml_runtime_context_t* ctx){
	ERROR("to_string");
	return NULL;
}



static aml_object_t* _exec_opcode_copy_object(aml_runtime_context_t* ctx){
	ERROR("copy_object");
	return NULL;
}



static aml_object_t* _exec_opcode_mid(aml_runtime_context_t* ctx){
	ERROR("mid");
	return NULL;
}



static aml_object_t* _exec_opcode_continue(aml_runtime_context_t* ctx){
	ERROR("continue");
	return NULL;
}



static aml_object_t* _exec_opcode_if(aml_runtime_context_t* ctx){
	u64 end_offset=ctx->offset+_get_pkglength(ctx);
	aml_object_t* predicate=aml_runtime_execute_single(ctx);
	if (!predicate){
		return NULL;
	}
	if (predicate->type!=AML_OBJECT_TYPE_INTEGER){
		ERROR("_exec_opcode_if: predicate is not an integer");
		return NULL;
	}
	_Bool execute_branch=!!predicate->integer;
	aml_object_dealloc(predicate);
	if (execute_branch){
		aml_runtime_context_t child_ctx={
			ctx->data+ctx->offset,
			end_offset-ctx->offset,
			ctx->namespace,
			ctx->vars
		};
		if (!aml_runtime_execute(&child_ctx)){
			return NULL;
		}
	}
	ctx->offset=end_offset;
	if (ctx->offset>=ctx->length||ctx->data[ctx->offset]!=AML_OPCODE_ELSE){
		return aml_object_alloc_none();
	}
	ctx->offset++;
	end_offset=ctx->offset+_get_pkglength(ctx);
	if (!execute_branch){
		aml_runtime_context_t child_ctx={
			ctx->data+ctx->offset,
			end_offset-ctx->offset,
			ctx->namespace,
			ctx->vars
		};
		if (!aml_runtime_execute(&child_ctx)){
			return NULL;
		}
	}
	ctx->offset=end_offset;
	return aml_object_alloc_none();
}



static aml_object_t* _exec_opcode_else(aml_runtime_context_t* ctx){
	ERROR("_exec_opcode_else: else without an if");
	return NULL;
}



static aml_object_t* _exec_opcode_while(aml_runtime_context_t* ctx){
	u64 end_offset=ctx->offset+_get_pkglength(ctx);
	u64 start_offset=ctx->offset;
	while (1){
		aml_object_t* predicate=aml_runtime_execute_single(ctx);
		if (!predicate){
			return NULL;
		}
		if (predicate->type!=AML_OBJECT_TYPE_INTEGER){
			ERROR("_exec_opcode_while: predicate is not an integer");
			return NULL;
		}
		_Bool execute_loop=!!predicate->integer;
		aml_object_dealloc(predicate);
		if (!execute_loop){
			break;
		}
		aml_runtime_context_t child_ctx={
			ctx->data+ctx->offset,
			end_offset-ctx->offset,
			ctx->namespace,
			ctx->vars
		};
		if (!aml_runtime_execute(&child_ctx)){
			return NULL;
		}
		ctx->offset=start_offset;
	}
	ctx->offset=end_offset;
	return aml_object_alloc_none();
}



static aml_object_t* _exec_opcode_noop(aml_runtime_context_t* ctx){
	return aml_object_alloc_none();
}



static aml_object_t* _exec_opcode_return(aml_runtime_context_t* ctx){
	if (!ctx->vars){
		ERROR("_exec_opcode_return: return outside of method");
		return NULL;
	}
	aml_object_t* value=aml_runtime_execute_single(ctx);
	if (!value){
		return NULL;
	}
	ctx->vars->ret=value;
	return aml_object_alloc_none();
}



static aml_object_t* _exec_opcode_break(aml_runtime_context_t* ctx){
	ERROR("break");
	return NULL;
}



static aml_object_t* _exec_opcode_break_point(aml_runtime_context_t* ctx){
	ERROR("break_point");
	return NULL;
}



static aml_object_t* _exec_opcode_ones(aml_runtime_context_t* ctx){
	return aml_object_alloc_integer(0xffffffffffffffffull);
}



static aml_object_t* _exec_opcode_ext_mutex(aml_runtime_context_t* ctx){
	string_t* name=_get_name(ctx);
	u8 sync_flags=_get_uint8(ctx);
	aml_namespace_lookup(ctx->namespace,name->data,AML_NAMESPACE_LOOKUP_FLAG_CREATE|AML_NAMESPACE_LOOKUP_FLAG_CLEAR)->value=aml_object_alloc_mutex(sync_flags);
	return aml_object_alloc_none();
}



static aml_object_t* _exec_opcode_ext_event(aml_runtime_context_t* ctx){
	ERROR("ext_event");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_cond_ref_of(aml_runtime_context_t* ctx){
	ERROR("ext_cond_ref_of");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_create_field(aml_runtime_context_t* ctx){
	ERROR("ext_create_field");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_load_table(aml_runtime_context_t* ctx){
	ERROR("ext_load_table");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_load(aml_runtime_context_t* ctx){
	ERROR("ext_load");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_stall(aml_runtime_context_t* ctx){
	ERROR("ext_stall");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_sleep(aml_runtime_context_t* ctx){
	ERROR("ext_sleep");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_acquire(aml_runtime_context_t* ctx){
	aml_object_t* mutex=aml_runtime_execute_single(ctx);
	if (!mutex){
		return NULL;
	}
	aml_object_dealloc(mutex);
	u16 timeout=_get_uint16(ctx);
	(void)timeout;
	return aml_object_alloc_none();
}



static aml_object_t* _exec_opcode_ext_signal(aml_runtime_context_t* ctx){
	ERROR("ext_signal");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_wait(aml_runtime_context_t* ctx){
	ERROR("ext_wait");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_reset(aml_runtime_context_t* ctx){
	ERROR("ext_reset");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_release(aml_runtime_context_t* ctx){
	aml_object_t* mutex=aml_runtime_execute_single(ctx);
	if (!mutex){
		return NULL;
	}
	aml_object_dealloc(mutex);
	return aml_object_alloc_none();
}



static aml_object_t* _exec_opcode_ext_from_bcd(aml_runtime_context_t* ctx){
	ERROR("ext_from_bcd");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_to_bcd(aml_runtime_context_t* ctx){
	ERROR("ext_to_bcd");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_unload(aml_runtime_context_t* ctx){
	ERROR("ext_unload");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_revision(aml_runtime_context_t* ctx){
	ERROR("ext_revision");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_debug(aml_runtime_context_t* ctx){
	ERROR("ext_debug");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_fatal(aml_runtime_context_t* ctx){
	ERROR("ext_fatal");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_timer(aml_runtime_context_t* ctx){
	ERROR("ext_timer");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_region(aml_runtime_context_t* ctx){
	string_t* name=_get_name(ctx);
	u8 region_space=_get_uint8(ctx);
	aml_object_t* region_offset=aml_runtime_execute_single(ctx);
	if (!region_offset){
		return NULL;
	}
	if (region_offset->type!=AML_OBJECT_TYPE_INTEGER){
		ERROR("_exec_opcode_ext_region: region_offset is not an integer");
		return NULL;
	}
	aml_object_t* region_length=aml_runtime_execute_single(ctx);
	if (!region_length){
		return NULL;
	}
	if (region_length->type!=AML_OBJECT_TYPE_INTEGER){
		ERROR("_exec_opcode_ext_region: region_length is not an integer");
		return NULL;
	}
	if (!region_space){
		region_offset->integer=vmm_identity_map(region_offset->integer,region_length->integer>>3);
	}
	aml_namespace_lookup(ctx->namespace,name->data,AML_NAMESPACE_LOOKUP_FLAG_CREATE|AML_NAMESPACE_LOOKUP_FLAG_CLEAR)->value=aml_object_alloc_region(region_space,region_offset->integer,region_length->integer);
	smm_dealloc(name);
	return aml_object_alloc_none();
}



static aml_object_t* _exec_opcode_ext_field(aml_runtime_context_t* ctx){
	u64 end_offset=ctx->offset+_get_pkglength(ctx);
	string_t* name=_get_name(ctx);
	u8 field_flags=_get_uint8(ctx);
	aml_namespace_t* namespace=aml_namespace_lookup(ctx->namespace,name->data,0);
	if (!namespace){
		ERROR("_exec_opcode_ext_field: namespace '%s' not found",name->data);
		smm_dealloc(name);
		return NULL;
	}
	smm_dealloc(name);
	if (!namespace->value||namespace->value->type!=AML_OBJECT_TYPE_REGION){
		ERROR("_exec_opcode_ext_field: namespace is not a region");
		return NULL;
	}
	u64 offset=0;
	while (ctx->offset<end_offset){
		if (!ctx->data[ctx->offset]){
			ctx->offset++;
			offset+=_get_pkglength(ctx);
		}
		else if (ctx->data[ctx->offset]==1){
			ERROR("_exec_opcode_ext_field: access field");
			return NULL;
		}
		else if (ctx->data[ctx->offset]==2){
			ERROR("_exec_opcode_ext_field: connect field");
			return NULL;
		}
		else if (ctx->data[ctx->offset]==3){
			ERROR("_exec_opcode_ext_field: extended access field");
			return NULL;
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



static aml_object_t* _exec_opcode_ext_device(aml_runtime_context_t* ctx){
	u64 end_offset=ctx->offset+_get_pkglength(ctx);
	string_t* name=_get_name(ctx);
	aml_runtime_context_t child_ctx={
		ctx->data+ctx->offset,
		end_offset-ctx->offset,
		aml_namespace_lookup(ctx->namespace,name->data,AML_NAMESPACE_LOOKUP_FLAG_CREATE|AML_NAMESPACE_LOOKUP_FLAG_CLEAR),
		NULL
	};
	child_ctx.namespace->value=aml_object_alloc_device();
	smm_dealloc(name);
	if (!aml_runtime_execute(&child_ctx)){
		return NULL;
	}
	ctx->offset=end_offset;
	return aml_object_alloc_none();
}



static aml_object_t* _exec_opcode_ext_processor(aml_runtime_context_t* ctx){
	u64 end_offset=ctx->offset+_get_pkglength(ctx);
	string_t* name=_get_name(ctx);
	u8 processor_id=_get_uint8(ctx);
	u32 processor_block_address=_get_uint32(ctx);
	u8 processor_block_length=_get_uint8(ctx);
	aml_runtime_context_t child_ctx={
		ctx->data+ctx->offset,
		end_offset-ctx->offset,
		aml_namespace_lookup(ctx->namespace,name->data,AML_NAMESPACE_LOOKUP_FLAG_CREATE|AML_NAMESPACE_LOOKUP_FLAG_CLEAR),
		NULL
	};
	child_ctx.namespace->value=aml_object_alloc_processor(processor_id,processor_block_address,processor_block_length);
	smm_dealloc(name);
	if (!aml_runtime_execute(&child_ctx)){
		return NULL;
	}
	ctx->offset=end_offset;
	return aml_object_alloc_none();
}



static aml_object_t* _exec_opcode_ext_power_res(aml_runtime_context_t* ctx){
	ERROR("ext_power_res");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_thermal_zone(aml_runtime_context_t* ctx){
	ERROR("ext_thermal_zone");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_index_field(aml_runtime_context_t* ctx){
	ERROR("ext_index_field");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_bank_field(aml_runtime_context_t* ctx){
	ERROR("ext_bank_field");
	return NULL;
}



static aml_object_t* _exec_opcode_ext_data_region(aml_runtime_context_t* ctx){
	ERROR("ext_data_region");
	return NULL;
}



static aml_object_t*(*const _aml_opcode_table[512])(aml_runtime_context_t*)={
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
	[AML_OPCODE_ONES]=_exec_opcode_ones,
	[AML_OPCODE_EXT_MUTEX|0x100]=_exec_opcode_ext_mutex,
	[AML_OPCODE_EXT_EVENT|0x100]=_exec_opcode_ext_event,
	[AML_OPCODE_EXT_COND_REF_OF|0x100]=_exec_opcode_ext_cond_ref_of,
	[AML_OPCODE_EXT_CREATE_FIELD|0x100]=_exec_opcode_ext_create_field,
	[AML_OPCODE_EXT_LOAD_TABLE|0x100]=_exec_opcode_ext_load_table,
	[AML_OPCODE_EXT_LOAD|0x100]=_exec_opcode_ext_load,
	[AML_OPCODE_EXT_STALL|0x100]=_exec_opcode_ext_stall,
	[AML_OPCODE_EXT_SLEEP|0x100]=_exec_opcode_ext_sleep,
	[AML_OPCODE_EXT_ACQUIRE|0x100]=_exec_opcode_ext_acquire,
	[AML_OPCODE_EXT_SIGNAL|0x100]=_exec_opcode_ext_signal,
	[AML_OPCODE_EXT_WAIT|0x100]=_exec_opcode_ext_wait,
	[AML_OPCODE_EXT_RESET|0x100]=_exec_opcode_ext_reset,
	[AML_OPCODE_EXT_RELEASE|0x100]=_exec_opcode_ext_release,
	[AML_OPCODE_EXT_FROM_BCD|0x100]=_exec_opcode_ext_from_bcd,
	[AML_OPCODE_EXT_TO_BCD|0x100]=_exec_opcode_ext_to_bcd,
	[AML_OPCODE_EXT_UNLOAD|0x100]=_exec_opcode_ext_unload,
	[AML_OPCODE_EXT_REVISION|0x100]=_exec_opcode_ext_revision,
	[AML_OPCODE_EXT_DEBUG|0x100]=_exec_opcode_ext_debug,
	[AML_OPCODE_EXT_FATAL|0x100]=_exec_opcode_ext_fatal,
	[AML_OPCODE_EXT_TIMER|0x100]=_exec_opcode_ext_timer,
	[AML_OPCODE_EXT_REGION|0x100]=_exec_opcode_ext_region,
	[AML_OPCODE_EXT_FIELD|0x100]=_exec_opcode_ext_field,
	[AML_OPCODE_EXT_DEVICE|0x100]=_exec_opcode_ext_device,
	[AML_OPCODE_EXT_PROCESSOR|0x100]=_exec_opcode_ext_processor,
	[AML_OPCODE_EXT_POWER_RES|0x100]=_exec_opcode_ext_power_res,
	[AML_OPCODE_EXT_THERMAL_ZONE|0x100]=_exec_opcode_ext_thermal_zone,
	[AML_OPCODE_EXT_INDEX_FIELD|0x100]=_exec_opcode_ext_index_field,
	[AML_OPCODE_EXT_BANK_FIELD|0x100]=_exec_opcode_ext_bank_field,
	[AML_OPCODE_EXT_DATA_REGION|0x100]=_exec_opcode_ext_data_region
};



KERNEL_PUBLIC aml_object_t* aml_runtime_execute_single(aml_runtime_context_t* ctx){
	u16 key=ctx->data[ctx->offset];
	ctx->offset++;
	if (key==AML_EXTENDED_OPCODE){
		key=ctx->data[ctx->offset]|0x100;
		ctx->offset++;
	}
	if (_aml_opcode_table[key]){
		return _aml_opcode_table[key](ctx);
	}
	ERROR("Unknown AML opcode '%s%X'",((key>>8)?"5b ":""),ctx->data[ctx->offset-1]);
	return NULL;
}



KERNEL_PUBLIC _Bool aml_runtime_execute(aml_runtime_context_t* ctx){
	ctx->offset=0;
	while (ctx->offset<ctx->length){
		aml_object_t* value=aml_runtime_execute_single(ctx);
		if (!value){
			return 0;
		}
		aml_object_dealloc(value);
		if (ctx->vars&&ctx->vars->ret){
			return 1;
		}
	}
	return 1;
}



KERNEL_PUBLIC aml_object_t* aml_runtime_execute_method(aml_object_t* method,u8 arg_count,aml_object_t*const* args){
	if (method->type!=AML_OBJECT_TYPE_METHOD){
		method->rc++;
		return method;
	}
	if (arg_count>(method->method.flags&7)){
		arg_count=method->method.flags&7;
	}
	aml_runtime_vars_t vars;
	for (u8 i=0;i<8;i++){
		if (i<7){
			vars.args[i]=aml_object_alloc_none();
		}
		vars.locals[i]=aml_object_alloc_none();
	}
	vars.ret=NULL;
	for (u8 i=0;i<arg_count;i++){
		aml_object_dealloc(vars.args[i]);
		args[i]->rc++;
		vars.args[i]=args[i];
	}
	aml_runtime_context_t ctx={
		method->method.code,
		method->method.code_length,
		method->method.namespace,
		&vars
	};
	_Bool ret=aml_runtime_execute(&ctx);
	for (u8 i=0;i<8;i++){
		if (i<7){
			aml_object_dealloc(vars.args[i]);
		}
		aml_object_dealloc(vars.locals[i]);
	}
	return (ret?vars.ret:NULL);
}



void aml_runtime_register_irq(u8 irq){
	ioapic_redirect_irq(irq,isr_allocate());
}
