#include <glsl/ast.h>
#include <glsl/error.h>
#include <glsl/linker.h>
#include <sys/format/format.h>
#include <sys/heap/heap.h>
#include <sys/io/io.h>
#include <sys/memory/memory.h>
#include <sys/string/string.h>
#include <sys/types.h>
#include <sys/util/var_arg.h>
#include <tgsicompiler/compiler.h>



#define COMPILATION_OUTPUT_BUFFER_GROWTH_SIZE 256 // power of 2



static void _output_string(tgsi_compilation_output_t* out,const char* str,u32 length){
	if (!length){
		length=sys_string_length(str);
	}
	if (out->_capacity-out->length<length){
		out->_capacity=(out->length+length+COMPILATION_OUTPUT_BUFFER_GROWTH_SIZE)&(-COMPILATION_OUTPUT_BUFFER_GROWTH_SIZE);
		out->data=sys_heap_realloc(NULL,out->data,out->_capacity);
	}
	sys_memory_copy(str,out->data+out->length,length);
	out->length+=length;
	out->data[out->length]=0;
}



static void _output_string_template(tgsi_compilation_output_t* out,const char* template,...){
	char buffer[256];
	sys_var_arg_list_t va;
	sys_var_arg_init(va,template);
	_output_string(out,buffer,sys_format_string_va(buffer,256,template,&va));
	sys_var_arg_deinit(va);
}



glsl_error_t tgsi_compile_shader(const glsl_compilation_output_t* output,glsl_shader_type_t shader_type,tgsi_compilation_output_t* out){
	out->data=NULL;
	out->length=0;
	out->_capacity=0;
	switch (shader_type){
		case GLSL_SHADER_TYPE_VERTEX:
			_output_string(out,"VERT\n",0);
			break;
		case GLSL_SHADER_TYPE_FRAGMENT:
			_output_string(out,"FRAG\n",0);
			break;
	}
	(void)_output_string_template;
	// for (u32 i=0;i<ast->var_count;i++){
	// 	const glsl_ast_var_t* var=ast->vars[i];
	// 	if (var->type->type==GLSL_AST_TYPE_TYPE_FUNC){
	// 		continue;
	// 	}
	// 	_output_string(out,"DCL ",0);
	// 	switch (var->storage.type){
	// 		case GLSL_AST_VAR_STORAGE_TYPE_DEFAULT:
	// 			_output_string(out,"TEMP",0);
	// 			break;
	// 		case GLSL_AST_VAR_STORAGE_TYPE_CONST:
	// 			_output_string(out,"IMM",0);
	// 			break;
	// 		case GLSL_AST_VAR_STORAGE_TYPE_IN:
	// 			_output_string(out,"IN",0);
	// 			break;
	// 		case GLSL_AST_VAR_STORAGE_TYPE_OUT:
	// 			_output_string(out,"OUT",0);
	// 			break;
	// 		case GLSL_AST_VAR_STORAGE_TYPE_UNIFORM:
	// 			_output_string(out,"CONST",0);
	// 			break;
	// 	}
	// 	u32 slot_count=glsl_ast_type_get_slot_count(var->type);
	// 	if (slot_count==1){
	// 		_output_string_template(out,"[%u]\n",var->link_slot);
	// 	}
	// 	else{
	// 		_output_string_template(out,"[%u..%u]\n",var->link_slot,var->link_slot+slot_count-1);
	// 	}
	// }
	sys_io_print("===SHADER===\n%s===SHADER===\n",out->data);
	out->data=sys_heap_realloc(NULL,out->data,out->length);
	out->_capacity=out->length;
	return GLSL_NO_ERROR;
}
