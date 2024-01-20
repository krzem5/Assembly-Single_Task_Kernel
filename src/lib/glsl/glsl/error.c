#include <glsl/ast.h>
#include <glsl/error.h>
#include <glsl/linker.h>
#include <sys/heap/heap.h>
#include <sys/format/format.h>
#include <sys/memory/memory.h>
#include <sys/types.h>



#define ERROR_BUFFER_SIZE 1024



static glsl_error_t _create_error(const char* buffer,u32 length){
	char* out=sys_heap_alloc(NULL,length+1);
	sys_memory_copy(buffer,out,length+1);
	return out;
}



SYS_PUBLIC void glsl_error_delete(glsl_error_t error){
	sys_heap_dealloc(NULL,error);
}



glsl_error_t _glsl_error_create_unimplemented(const char* file,u32 line,const char* func){
	char buffer[ERROR_BUFFER_SIZE];
	return _create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"Unimplemented: %s:%u(%s)",file,line,func));
}



glsl_error_t _glsl_error_create_preprocessor_unknown_directive(const char* directive,u32 directive_length){
	char buffer[ERROR_BUFFER_SIZE];
	return _create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"Unknown preprocessor directive '%.*s'",directive_length,directive));
}



glsl_error_t _glsl_error_create_preprocessor_already_defined(const char* identifier){
	char buffer[ERROR_BUFFER_SIZE];
	return _create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"Preprocessor macro '%s' is already defined",identifier));
}



glsl_error_t _glsl_error_create_preprocessor_wrong_version_placement(void){
	char buffer[ERROR_BUFFER_SIZE];
	return _create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"#version directive must be placed on top of the file"));
}



glsl_error_t _glsl_error_create_preprocessor_invalid_version(u32 version){
	char buffer[ERROR_BUFFER_SIZE];
	return _create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"Unknown GLSL version '%u'",version));
}



glsl_error_t _glsl_error_create_preprocessor_invalid_profile(const char* profile,u32 profile_length){
	char buffer[ERROR_BUFFER_SIZE];
	return _create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"Unsupported GLSL profile '%.*s'",profile_length,profile));
}



glsl_error_t _glsl_error_create_lexer_unexpected_character(char c){
	char buffer[ERROR_BUFFER_SIZE];
	return _create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"Unexpected character '%c'",c));
}



glsl_error_t _glsl_error_create_lexer_digit_expected(char c,u32 base){
	char buffer[ERROR_BUFFER_SIZE];
	return _create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"%s digit expected, got '%c'",(base==8?"Octal":(base==16?"Hexadecimal":"Decimal")),c));
}



glsl_error_t _glsl_error_create_lexer_reserved_keyword(const char* keyword){
	char buffer[ERROR_BUFFER_SIZE];
	return _create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"Reserved keyword: %s",keyword));
}



glsl_error_t _glsl_error_create_parser_deprecated_keyword(const char* keyword){
	char buffer[ERROR_BUFFER_SIZE];
	return _create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"Deprecated keyword: %s",keyword));
}



glsl_error_t _glsl_error_create_parser_expected(const char* type){
	char buffer[ERROR_BUFFER_SIZE];
	return _create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"Expected %s, got ???",type));
}



glsl_error_t _glsl_error_create_parser_already_defined(const char* name){
	char buffer[ERROR_BUFFER_SIZE];
	return _create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"Identifier '%s' is already defined",name));
}



glsl_error_t _glsl_error_create_parser_invalid_storage_type(const char* type,const glsl_ast_var_storage_t* storage){
	char buffer[ERROR_BUFFER_SIZE];
	return _create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"Storage '%s' cannot be applied to a %s","???",type));
}



glsl_error_t _glsl_error_create_parser_unknown_layout_qualifier(const char* qualifier){
	char buffer[ERROR_BUFFER_SIZE];
	return _create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"Unknown layout qualifier '%s'",qualifier));
}



glsl_error_t _glsl_error_create_parser_duplicated_layout_qualifier(const char* qualifier){
	char buffer[ERROR_BUFFER_SIZE];
	return _create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"Duplicated layout qualifier '%s'",qualifier));
}



glsl_error_t _glsl_error_create_parser_expression_stack_overflow(void){
	char buffer[ERROR_BUFFER_SIZE];
	return _create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"Too many nested expressions"));
}



glsl_error_t _glsl_error_create_parser_undefined(const char* name){
	char buffer[ERROR_BUFFER_SIZE];
	return _create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"Undefined identifier '%s'",name));
}



glsl_error_t _glsl_error_create_parser_wrong_application(const char* operator,const char* type){
	char buffer[ERROR_BUFFER_SIZE];
	return _create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"Operator '%s' cannot be applied to a %s",operator,type));
}



glsl_error_t _glsl_error_create_parser_function_constructor(void){
	char buffer[ERROR_BUFFER_SIZE];
	return _create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"A function cannot be used in a constructor"));
}



glsl_error_t _glsl_error_create_parser_constructor_too_long(void){
	char buffer[ERROR_BUFFER_SIZE];
	return _create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"Too many constructor arguments"));
}



glsl_error_t _glsl_error_create_parser_invalid_constructor(const glsl_ast_node_t* node){
	char buffer[ERROR_BUFFER_SIZE];
	char* type_str=glsl_ast_type_to_string(node->value_type);
	u32 length=sys_format_string(buffer,ERROR_BUFFER_SIZE,"Type '%s' cannot be constructed from type%s: ",type_str,(node->arg_count==1?"":"s"));
	sys_heap_dealloc(NULL,type_str);
	for (u32 i=0;i<node->arg_count;i++){
		type_str=glsl_ast_type_to_string(glsl_ast_get_arg(node,i)->value_type);
		length+=sys_format_string(buffer,ERROR_BUFFER_SIZE-length,"%s",type_str);
		sys_heap_dealloc(NULL,type_str);
		if (i<node->arg_count-1){
			length+=sys_format_string(buffer,ERROR_BUFFER_SIZE-length,", ");
		}
	}
	length+=sys_format_string(buffer,ERROR_BUFFER_SIZE-length,".\n");
	return _create_error(buffer,length);
}



glsl_error_t _glsl_error_create_parser_swizzle_too_long(const char* swizzle){
	char buffer[ERROR_BUFFER_SIZE];
	return _create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"Swizzle too long: %s",swizzle));
}



glsl_error_t _glsl_error_create_parser_invalid_swizzle_character(const char* swizzle,char c){
	char buffer[ERROR_BUFFER_SIZE];
	return _create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"Invalid swizzle character in swizzle '%s': %c",swizzle,c));
}



glsl_error_t _glsl_error_create_parser_member_not_found(const glsl_ast_type_t* type,const char* member){
	char buffer[ERROR_BUFFER_SIZE];
	char* type_str=glsl_ast_type_to_string(type);
	glsl_error_t out=_create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"Member '%s' not found in type '%s'",member,type_str));
	sys_heap_dealloc(NULL,type_str);
	return out;
}



glsl_error_t _glsl_error_create_parser_invalid_operator_types(const char* operator,const glsl_ast_type_t* left_type,const glsl_ast_type_t* right_type){
	char buffer[ERROR_BUFFER_SIZE];
	char* type_str_left=glsl_ast_type_to_string(left_type);
	char* type_str_right=glsl_ast_type_to_string(right_type);
	glsl_error_t out=_create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"Operator '%s' cannot be applied to types '%s' and '%s'",operator,type_str_left,type_str_right));
	sys_heap_dealloc(NULL,type_str_left);
	sys_heap_dealloc(NULL,type_str_right);
	return out;
}



glsl_error_t _glsl_error_create_parser_invalid_lvalue(void){
	char buffer[ERROR_BUFFER_SIZE];
	return _create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"Invalid lvalue expression"));
}



glsl_error_t _glsl_error_create_parser_dupliated_swizzle_component(void){
	char buffer[ERROR_BUFFER_SIZE];
	return _create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"Duplicated component in lvalue swizzle"));
}



glsl_error_t _glsl_error_create_parser_disallowed_operation(const glsl_ast_var_storage_t* storage,glsl_ast_var_usage_flags_t usage_flag){
	char buffer[ERROR_BUFFER_SIZE];
	return _create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"%s operation is not allowed on storage class '%s'",((usage_flag&GLSL_AST_VAR_USAGE_FLAG_READ)?"Read":"Write"),"???"));
}



glsl_error_t _glsl_error_create_parser_uninitialized_var(const char* name){
	char buffer[ERROR_BUFFER_SIZE];
	return _create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"Variable '%s' is uninitialized",name));
}



glsl_error_t _glsl_error_create_linker_missing_shader(glsl_shader_type_t shader_type){
	char buffer[ERROR_BUFFER_SIZE];
	return _create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"%s shader missing",(shader_type==GLSL_SHADER_TYPE_VERTEX?"Vertex":"Fragment")));
}



glsl_error_t _glsl_error_create_linker_unlinked_var(const char* name,_Bool is_output){
	char buffer[ERROR_BUFFER_SIZE];
	return _create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"%s variable '%s' is not declared in %s shader stage",(is_output?"Output":"Input"),name,(is_output?"next":"previous")));
}



glsl_error_t _glsl_error_create_linker_wrong_type(const char* name,const glsl_ast_type_t* output_type,const glsl_ast_type_t* input_type){
	char buffer[ERROR_BUFFER_SIZE];
	char* output_type_str=glsl_ast_type_to_string(output_type);
	char* input_type_str=glsl_ast_type_to_string(input_type);
	glsl_error_t out=_create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"Types of variable '%s' do not match: %s and %s",name,output_type_str,input_type_str));
	sys_heap_dealloc(NULL,output_type_str);
	sys_heap_dealloc(NULL,input_type_str);
	return out;
}



glsl_error_t _glsl_error_create_linker_inconsistent_layout(const char* name){
	char buffer[ERROR_BUFFER_SIZE];
	return _create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"Variable '%s' has an inconsistent layout declaration",name));
}



glsl_error_t _glsl_error_create_linker_unallocatable_layout(const char* name){
	char buffer[ERROR_BUFFER_SIZE];
	return _create_error(buffer,sys_format_string(buffer,ERROR_BUFFER_SIZE,"Variable '%s' cannot be allocated",name));
}
