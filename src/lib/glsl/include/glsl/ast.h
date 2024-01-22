#ifndef _GLSL_AST_H_
#define _GLSL_AST_H_ 1
#include <glsl/builtin_types.h>
#include <glsl/shader.h>
#include <sys/types.h>



#define GLSL_AST_NODE_MAX_ARG_COUNT 7

#define GLSL_AST_NODE_TYPE_NONE 0
#define GLSL_AST_NODE_TYPE_ARRAY_ACCESS 1
#define GLSL_AST_NODE_TYPE_BLOCK 2
#define GLSL_AST_NODE_TYPE_CALL 3
#define GLSL_AST_NODE_TYPE_CONSTRUCTOR 4
#define GLSL_AST_NODE_TYPE_INLINE_BLOCK 5
#define GLSL_AST_NODE_TYPE_MEMBER_ACCESS 6
#define GLSL_AST_NODE_TYPE_OPERATOR 7
#define GLSL_AST_NODE_TYPE_SWIZZLE 8
#define GLSL_AST_NODE_TYPE_VAR 9
#define GLSL_AST_NODE_TYPE_VAR_CONST 10

#define GLSL_AST_NODE_MAX_TYPE GLSL_AST_NODE_TYPE_VAR_CONST

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

#define GLSL_AST_VAR_STORAGE_MAX_TYPE GLSL_AST_VAR_STORAGE_TYPE_UNIFORM

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
		struct{
			struct _GLSL_AST_NODE* data[GLSL_AST_NODE_MAX_ARG_COUNT];
			u32 count;
		} args;
		struct{
			struct _GLSL_AST_NODE** data;
			u32 length;
		} block;
		struct{
			struct _GLSL_AST_NODE* value;
			char* member;
		} member_access;
		struct{
			struct _GLSL_AST_NODE* value;
			glsl_ast_swizzle_t pattern;
			u8 pattern_length;
		} swizzle;
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
			u32 var_int_matrix[16];
		};
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
	u32 link_slot; // to delete
	u64 _compiler_data;
	glsl_ast_scope_t scope;
	glsl_ast_var_storage_t storage;
	glsl_ast_type_t* type;
	glsl_ast_node_t* value;
} glsl_ast_var_t;



typedef struct _GLSL_AST{
	glsl_shader_type_t shader_type;
	u32 block_count;
	u32 named_type_count;
	u32 var_count;
	glsl_ast_block_t** blocks;
	glsl_ast_named_type_t** named_types;
	glsl_ast_var_t** vars;
} glsl_ast_t;



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
