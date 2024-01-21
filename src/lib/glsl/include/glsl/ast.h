#ifndef _GLSL_AST_H_
#define _GLSL_AST_H_ 1
#include <glsl/builtin_types.h>
#include <sys/types.h>



#define GLSL_AST_NODE_INLINE_ARG_COUNT 7

// #define GLSL_AST_NODE_TYPE_NONE 0
// #define GLSL_AST_NODE_TYPE_ADD 1
// #define GLSL_AST_NODE_TYPE_ADD_ASSIGN 2
// #define GLSL_AST_NODE_TYPE_AND 3
// #define GLSL_AST_NODE_TYPE_AND_ASSIGN 4
// #define GLSL_AST_NODE_TYPE_ASSIGN 6
// #define GLSL_AST_NODE_TYPE_BIT_INVERSE 7
// #define GLSL_AST_NODE_TYPE_DIVIDE 11
// #define GLSL_AST_NODE_TYPE_DIVIDE_ASSIGN 12
// #define GLSL_AST_NODE_TYPE_EQUALS 13
// #define GLSL_AST_NODE_TYPE_GREATER_THAN 14
// #define GLSL_AST_NODE_TYPE_LEFT_SHIFT 15
// #define GLSL_AST_NODE_TYPE_LEFT_SHIFT_ASSIGN 16
// #define GLSL_AST_NODE_TYPE_LESS_THAN 17
// #define GLSL_AST_NODE_TYPE_LOGICAL_AND 18
// #define GLSL_AST_NODE_TYPE_LOGICAL_OR 19
// #define GLSL_AST_NODE_TYPE_MODULO 21
// #define GLSL_AST_NODE_TYPE_MODULO_ASSIGN 22
// #define GLSL_AST_NODE_TYPE_MULTIPLY 23
// #define GLSL_AST_NODE_TYPE_MULTIPLY_ASSIGN 24
// #define GLSL_AST_NODE_TYPE_NEGATE 25
// #define GLSL_AST_NODE_TYPE_NOT 26
// #define GLSL_AST_NODE_TYPE_NOT_EQUALS 27
// #define GLSL_AST_NODE_TYPE_NOT_GREATER_THAN 28
// #define GLSL_AST_NODE_TYPE_NOT_LESS_THAN 29
// #define GLSL_AST_NODE_TYPE_OR 30
// #define GLSL_AST_NODE_TYPE_OR_ASSIGN 31
// #define GLSL_AST_NODE_TYPE_RIGHT_SHIFT 32
// #define GLSL_AST_NODE_TYPE_RIGHT_SHIFT_ASSIGN 33
// #define GLSL_AST_NODE_TYPE_SUBTRACT 34
// #define GLSL_AST_NODE_TYPE_SUBTRACT_ASSIGN 35
// #define GLSL_AST_NODE_TYPE_XOR 40
// #define GLSL_AST_NODE_TYPE_XOR_ASSIGN 41

#define new_GLSL_AST_NODE_TYPE_NONE 0
#define new_GLSL_AST_NODE_TYPE_ARRAY_ACCESS 1
#define new_GLSL_AST_NODE_TYPE_BLOCK 2
#define new_GLSL_AST_NODE_TYPE_CALL 3
#define new_GLSL_AST_NODE_TYPE_CONSTRUCTOR 4
#define new_GLSL_AST_NODE_TYPE_INLINE_BLOCK 5
#define new_GLSL_AST_NODE_TYPE_MEMBER_ACCESS 6
#define new_GLSL_AST_NODE_TYPE_OPERATOR 7
#define new_GLSL_AST_NODE_TYPE_SWIZZLE 8
#define new_GLSL_AST_NODE_TYPE_VAR 9
#define new_GLSL_AST_NODE_TYPE_VAR_CONST 10

// #define GLSL_AST_NODE_MAX_TYPE GLSL_AST_NODE_TYPE_SWIZZLE
#define new_GLSL_AST_NODE_MAX_TYPE new_GLSL_AST_NODE_TYPE_VAR_CONST

#define GLSL_AST_NODE_OPERATOR_TYPE_NONE 0
#define GLSL_AST_NODE_OPERATOR_TYPE_ADD 1
#define GLSL_AST_NODE_OPERATOR_TYPE_ADD_ASSIGN 2
#define GLSL_AST_NODE_OPERATOR_TYPE_AND 3
#define GLSL_AST_NODE_OPERATOR_TYPE_AND_ASSIGN 4
#define GLSL_AST_NODE_OPERATOR_TYPE_ASSIGN 5
#define GLSL_AST_NODE_OPERATOR_TYPE_BIT_INVERSE 6
#define GLSL_AST_NODE_OPERATOR_TYPE_DIVIDE 7
#define GLSL_AST_NODE_OPERATOR_TYPE_DIVIDE_ASSIGN 8
#define GLSL_AST_NODE_OPERATOR_TYPE_EQUALS 9
#define GLSL_AST_NODE_OPERATOR_TYPE_GREATER_THAN 10
#define GLSL_AST_NODE_OPERATOR_TYPE_LEFT_SHIFT 11
#define GLSL_AST_NODE_OPERATOR_TYPE_LEFT_SHIFT_ASSIGN 12
#define GLSL_AST_NODE_OPERATOR_TYPE_LESS_THAN 13
#define GLSL_AST_NODE_OPERATOR_TYPE_LOGICAL_AND 14
#define GLSL_AST_NODE_OPERATOR_TYPE_LOGICAL_OR 15
#define GLSL_AST_NODE_OPERATOR_TYPE_MODULO 16
#define GLSL_AST_NODE_OPERATOR_TYPE_MODULO_ASSIGN 17
#define GLSL_AST_NODE_OPERATOR_TYPE_MULTIPLY 18
#define GLSL_AST_NODE_OPERATOR_TYPE_MULTIPLY_ASSIGN 19
#define GLSL_AST_NODE_OPERATOR_TYPE_NEGATE 20
#define GLSL_AST_NODE_OPERATOR_TYPE_NOT 21
#define GLSL_AST_NODE_OPERATOR_TYPE_NOT_EQUALS 22
#define GLSL_AST_NODE_OPERATOR_TYPE_NOT_GREATER_THAN 23
#define GLSL_AST_NODE_OPERATOR_TYPE_NOT_LESS_THAN 24
#define GLSL_AST_NODE_OPERATOR_TYPE_OR 25
#define GLSL_AST_NODE_OPERATOR_TYPE_OR_ASSIGN 26
#define GLSL_AST_NODE_OPERATOR_TYPE_RIGHT_SHIFT 27
#define GLSL_AST_NODE_OPERATOR_TYPE_RIGHT_SHIFT_ASSIGN 28
#define GLSL_AST_NODE_OPERATOR_TYPE_SUBTRACT 29
#define GLSL_AST_NODE_OPERATOR_TYPE_SUBTRACT_ASSIGN 30
#define GLSL_AST_NODE_OPERATOR_TYPE_XOR 31
#define GLSL_AST_NODE_OPERATOR_TYPE_XOR_ASSIGN 32

#define GLSL_AST_TYPE_TYPE_BUILTIN 0
#define GLSL_AST_TYPE_TYPE_STRUCT 1
#define GLSL_AST_TYPE_TYPE_FUNC 2

#define GLSL_AST_TYPE_NO_ARRAY 0xffffffff

#define GLSL_AST_VAR_USAGE_FLAG_READ 1
#define GLSL_AST_VAR_USAGE_FLAG_WRITE 2

#define GLSL_AST_VAR_FLAG_INITIALIZED 1
#define GLSL_AST_VAR_FLAG_LINKED 2
#define GLSL_AST_VAR_FLAG_BUILTIN 4

#define GLSL_AST_VAR_STORAGE_TYPE_DEFAULT 0
#define GLSL_AST_VAR_STORAGE_TYPE_CONST 1
#define GLSL_AST_VAR_STORAGE_TYPE_IN 2
#define GLSL_AST_VAR_STORAGE_TYPE_OUT 3
#define GLSL_AST_VAR_STORAGE_TYPE_UNIFORM 4

#define GLSL_AST_VAR_STORAGE_FLAG_CENTROID 1
#define GLSL_AST_VAR_STORAGE_FLAG_HAS_LAYOUT_LOCATION 2
#define GLSL_AST_VAR_STORAGE_FLAG_BLOCK 4



typedef u8 glsl_ast_var_flags_t;



typedef u8 glsl_ast_var_usage_flags_t;



typedef u8 glsl_ast_swizzle_t;



typedef u32 glsl_ast_scope_t;



typedef u32 glsl_ast_type_type_t;



typedef u32 glsl_ast_node_type_t;



typedef u32 glsl_ast_node_operator_type_t;



typedef struct _GLSL_AST_TYPE{
	glsl_ast_type_type_t type;
	u32 array_length;
	u32 rc;
	union{
		glsl_builtin_type_t builtin_type;
		struct{
			struct _GLSL_AST_TYPE_FIELD* fields;
			u32 field_count;
		} struct_;
		struct{
			struct _GLSL_AST_TYPE** return_and_args;
			u32 arg_count;
		} func;
	};
} glsl_ast_type_t;



typedef struct _GLSL_AST_TYPE_FIELD{
	const char* field;
	glsl_ast_type_t* type;
} glsl_ast_type_field_t;



typedef struct _GLSL_AST_NODE{
	glsl_ast_node_type_t type;
	union{
		glsl_ast_scope_t block_scope;
		glsl_ast_node_operator_type_t operator_type;
	};
	glsl_ast_type_t* value_type;
	union{
		union{
			struct _GLSL_AST_VAR* var;
			_Bool var_bool;
			float var_float;
			s32 var_int;
			float var_vector[4];
			_Bool var_vector_bool[4];
			s32 var_vector_int[4];
			u32 var_vector_uint[4];
			float var_matrix[16];
		};
		struct{
			union{
				struct _GLSL_AST_NODE* args_inline[GLSL_AST_NODE_INLINE_ARG_COUNT];
				struct _GLSL_AST_NODE** args;
			};
			u32 arg_count;
		};
		struct{
			struct _GLSL_AST_NODE* value;
			char* member;
		} member_access;
		struct{
			struct _GLSL_AST_NODE* value;
			glsl_ast_swizzle_t pattern;
			u8 pattern_length;
		} swizzle;
	};
} glsl_ast_node_t;



typedef struct _GLSL_AST_NAMED_TYPE{
	char* name;
	glsl_ast_type_t* type;
} glsl_ast_named_type_t;



typedef struct _GLSL_AST_VAR_STORAGE{
	u8 type;
	u8 flags;
	u16 layout_location;
	struct _GLSL_AST_BLOCK* block;
} glsl_ast_var_storage_t;



typedef struct _GLSL_AST_BLOCK{
	char* name;
	glsl_ast_var_storage_t storage;
} glsl_ast_block_t;



typedef struct _GLSL_AST_VAR{
	char* name;
	glsl_ast_var_usage_flags_t usage_flags;
	glsl_ast_var_usage_flags_t possible_usage_flags;
	glsl_ast_var_flags_t flags;
	u32 link_slot;
	glsl_ast_scope_t scope;
	glsl_ast_var_storage_t storage;
	glsl_ast_type_t* type;
	glsl_ast_node_t* value;
} glsl_ast_var_t;



typedef struct _GLSL_AST{
	u32 block_count;
	u32 named_type_count;
	u32 var_count;
	glsl_ast_block_t** blocks;
	glsl_ast_named_type_t** named_types;
	glsl_ast_var_t** vars;
} glsl_ast_t;



static inline glsl_ast_node_t* glsl_ast_get_arg(const glsl_ast_node_t* node,u32 idx){
	return (node->arg_count>=GLSL_AST_NODE_INLINE_ARG_COUNT?node->args:node->args_inline)[idx];
}



void glsl_ast_delete(glsl_ast_t* ast);



glsl_ast_var_t* glsl_ast_lookup_var(glsl_ast_t* ast,const char* name,glsl_ast_scope_t scope,glsl_ast_type_t* type);



glsl_ast_var_t* glsl_ast_create_var(glsl_ast_t* ast,const char* name,glsl_ast_scope_t scope,glsl_ast_type_t* type,const glsl_ast_var_storage_t* storage);



glsl_ast_type_t* glsl_ast_type_create(glsl_ast_type_type_t type);



void glsl_ast_type_delete(glsl_ast_type_t* type);



glsl_ast_type_t* glsl_ast_type_duplicate(glsl_ast_type_t* type);



char* glsl_ast_type_to_string(const glsl_ast_type_t* type);



_Bool glsl_ast_type_is_equal(const glsl_ast_type_t* a,const glsl_ast_type_t* b);



u32 glsl_ast_type_get_slot_count(const glsl_ast_type_t* type);



glsl_ast_node_t* glsl_ast_node_create(glsl_ast_node_type_t type);



void glsl_ast_node_delete(glsl_ast_node_t* node);



const char* glsl_ast_node_type_to_string(glsl_ast_node_type_t type);



#endif
