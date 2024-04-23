#ifndef _GLSL__INTERNAL_ERROR_H_
#define _GLSL__INTERNAL_ERROR_H_ 1
#include <glsl/ast.h>
#include <glsl/error.h>
#include <glsl/linker.h>
#include <sys/types.h>



glsl_error_t _glsl_error_create_unimplemented(const char* file,u32 line,const char* func);



glsl_error_t _glsl_error_create_preprocessor_unknown_directive(const char* directive,u32 directive_length);



glsl_error_t _glsl_error_create_preprocessor_already_defined(const char* identifier);



glsl_error_t _glsl_error_create_preprocessor_wrong_version_placement(void);



glsl_error_t _glsl_error_create_preprocessor_invalid_version(u32 version);



glsl_error_t _glsl_error_create_preprocessor_invalid_profile(const char* profile,u32 profile_length);



glsl_error_t _glsl_error_create_lexer_unexpected_character(char c);



glsl_error_t _glsl_error_create_lexer_digit_expected(char c,u32 base);



glsl_error_t _glsl_error_create_lexer_reserved_keyword(const char* keyword);



glsl_error_t _glsl_error_create_parser_deprecated_keyword(const char* keyword);



glsl_error_t _glsl_error_create_parser_expected(const char* type);



glsl_error_t _glsl_error_create_parser_already_defined(const char* name);



glsl_error_t _glsl_error_create_parser_invalid_storage_type(const char* type,const glsl_ast_var_storage_t* storage);



glsl_error_t _glsl_error_create_parser_unknown_layout_qualifier(const char* qualifier);



glsl_error_t _glsl_error_create_parser_duplicated_layout_qualifier(const char* qualifier);



glsl_error_t _glsl_error_create_parser_expression_stack_overflow(void);



glsl_error_t _glsl_error_create_parser_undefined(const char* name);



glsl_error_t _glsl_error_create_parser_wrong_application(const char* operator,const char* type);



glsl_error_t _glsl_error_create_parser_function_constructor(void);



glsl_error_t _glsl_error_create_parser_constructor_too_long(void);



glsl_error_t _glsl_error_create_parser_invalid_constructor(const glsl_ast_node_t* node);



glsl_error_t _glsl_error_create_parser_swizzle_too_long(const char* swizzle);



glsl_error_t _glsl_error_create_parser_invalid_swizzle_character(const char* swizzle,char c);



glsl_error_t _glsl_error_create_parser_member_not_found(const glsl_ast_type_t* type,const char* member);



glsl_error_t _glsl_error_create_parser_invalid_operator_types(const char* operator,const glsl_ast_type_t* left_type,const glsl_ast_type_t* right_type);



glsl_error_t _glsl_error_create_parser_invalid_lvalue(void);



glsl_error_t _glsl_error_create_parser_dupliated_swizzle_component(void);



glsl_error_t _glsl_error_create_parser_disallowed_operation(const glsl_ast_var_storage_t* storage,glsl_ast_var_usage_flags_t usage_flag);



glsl_error_t _glsl_error_create_parser_uninitialized_var(const char* name);



glsl_error_t _glsl_error_create_parser_non_constant_initializer(void);



glsl_error_t _glsl_error_create_parser_too_many_arguments(void);



glsl_error_t _glsl_error_create_linker_missing_shader(glsl_shader_type_t shader_type);



glsl_error_t _glsl_error_create_linker_unlinked_var(const char* name,bool is_output);



glsl_error_t _glsl_error_create_linker_wrong_type(const char* name,u32 output_slot_count,u32 input_slot_count);



glsl_error_t _glsl_error_create_linker_inconsistent_layout(const char* name);



glsl_error_t _glsl_error_create_linker_unallocatable_layout(const char* name);



#endif
