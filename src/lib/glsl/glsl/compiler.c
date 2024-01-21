#include <glsl/_internal/error.h>
#include <glsl/_internal/interface_allocator.h>
#include <glsl/ast.h>
#include <glsl/compiler.h>
#include <glsl/error.h>
#include <sys/io/io.h>
#include <sys/types.h>



static const u32 _glsl_ast_output_var_allocator_limits[GLSL_COMPILATION_OUTPUT_VAR_MAX_TYPE+1]={
	[GLSL_COMPILATION_OUTPUT_VAR_TYPE_LOCAL]=64,
	[GLSL_COMPILATION_OUTPUT_VAR_TYPE_INPUT]=64,
	[GLSL_COMPILATION_OUTPUT_VAR_TYPE_OUTPUT]=64,
	[GLSL_COMPILATION_OUTPUT_VAR_TYPE_UNIFORM]=64
};



static const glsl_compilation_output_var_type_t _glsl_ast_storage_type_to_output_var_type[GLSL_AST_VAR_STORAGE_MAX_TYPE+1]={
	[GLSL_AST_VAR_STORAGE_TYPE_DEFAULT]=GLSL_COMPILATION_OUTPUT_VAR_TYPE_LOCAL,
	[GLSL_AST_VAR_STORAGE_TYPE_IN]=GLSL_COMPILATION_OUTPUT_VAR_TYPE_INPUT,
	[GLSL_AST_VAR_STORAGE_TYPE_OUT]=GLSL_COMPILATION_OUTPUT_VAR_TYPE_OUTPUT,
	[GLSL_AST_VAR_STORAGE_TYPE_UNIFORM]=GLSL_COMPILATION_OUTPUT_VAR_TYPE_UNIFORM,
};



static _Bool _is_var_used(const glsl_ast_var_t* var){
	return ((var->storage.type==GLSL_AST_VAR_STORAGE_TYPE_DEFAULT&&var->usage_flags==(GLSL_AST_VAR_USAGE_FLAG_READ|GLSL_AST_VAR_USAGE_FLAG_WRITE))||(var->storage.type!=GLSL_AST_VAR_STORAGE_TYPE_DEFAULT&&var->usage_flags))&&var->type->type!=GLSL_AST_TYPE_TYPE_FUNC;
}



static glsl_error_t _allocate_slots(const glsl_ast_t* ast,glsl_compilation_output_t* out){
	glsl_error_t error=GLSL_NO_ERROR;
	glsl_interface_allocator_t allocators[GLSL_COMPILATION_OUTPUT_VAR_MAX_TYPE+1];
	for (glsl_compilation_output_var_type_t i=0;i<=GLSL_COMPILATION_OUTPUT_VAR_MAX_TYPE;i++){
		_glsl_interface_allocator_init(_glsl_ast_output_var_allocator_limits[i],allocators+i);
	}
	u8 mask_value=GLSL_AST_VAR_STORAGE_FLAG_HAS_LAYOUT_LOCATION;
_second_pass:
	for (u32 i=0;i<ast->var_count;i++){
		const glsl_ast_var_t* var=ast->vars[i];
		if (var->storage.type!=GLSL_AST_VAR_STORAGE_TYPE_CONST&&_is_var_used(var)&&(var->storage.flags&GLSL_AST_VAR_STORAGE_FLAG_HAS_LAYOUT_LOCATION)==mask_value){
			glsl_compilation_output_var_type_t output_var_type=_glsl_ast_storage_type_to_output_var_type[var->storage.type];
			u32 slot=(mask_value?var->storage.layout_location:0xffffffff);
			if (!_glsl_interface_allocator_reserve(allocators+output_var_type,&slot,glsl_ast_type_get_slot_count(var->type))){
				error=_glsl_error_create_linker_unallocatable_layout(var->name);
				goto _error;
			}
			sys_io_print("%s => %u [%u]\n",var->name,slot,output_var_type);
		}
	}
	if (mask_value){
		mask_value=0;
		goto _second_pass;
	}
_error:
	for (glsl_compilation_output_var_type_t i=0;i<=GLSL_COMPILATION_OUTPUT_VAR_MAX_TYPE;i++){
		_glsl_interface_allocator_deinit(allocators+i);
	}
	return error;
}



SYS_PUBLIC glsl_error_t glsl_compiler_compile(const glsl_ast_t* ast,glsl_compilation_output_t* out){
	out->var_count=0;
	out->instruction_count=0;
	out->_var_capacity=0;
	out->_instruction_capacity=0;
	out->vars=NULL;
	out->instructions=NULL;
	for (glsl_compilation_output_var_type_t i=0;i<=GLSL_COMPILATION_OUTPUT_VAR_MAX_TYPE;i++){
		out->slot_counts[i]=0;
	}
	glsl_error_t error=_allocate_slots(ast,out);
	if (error!=GLSL_NO_ERROR){
		goto _cleanup;
	}
	return GLSL_NO_ERROR;
_cleanup:
	glsl_compiler_compilation_output_delete(out);
	return error;
}



SYS_PUBLIC void glsl_compiler_compilation_output_delete(glsl_compilation_output_t* output){
	return;
}
