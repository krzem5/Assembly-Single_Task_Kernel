#include <glsl/ast.h>
#include <glsl/internal_functions.h>
#include <sys/string/string.h>
#include <sys/io/io.h>
#include <sys/types.h>



#define MATCH_FUNC_NAME(func) \
	if (!sys_string_compare(name,"__gl_"#func)){ \
		return _match_##func(node,arg_types); \
	}

#define MATCH_RESOLVE(func,ret,args) \
	node->internal_function_type=GLSL_INTERNAL_FUNCTION_TYPE_##func##_##ret##args; \
	node->value_type=glsl_ast_type_create(GLSL_AST_TYPE_TYPE_BUILTIN); \
	node->value_type->builtin_type=GLSL_BUILTIN_TYPE_##ret; \
	return 1;
#define MATCH_FUNC_ARGS2(func,ret,arg0,arg1) \
	if (node->args.count==2&&arg_types[0]==GLSL_BUILTIN_TYPE_##arg0&&arg_types[1]==GLSL_BUILTIN_TYPE_##arg1){ \
		MATCH_RESOLVE(func,ret,_##arg0##_##arg1); \
	}



static _Bool _match_texture(glsl_ast_node_t* node,const glsl_builtin_type_t* arg_types){
	MATCH_FUNC_ARGS2(TEXTURE,VEC4,SAMPLER_2D,VEC2);
	return 0;
}



_Bool _glsl_internal_function_table_find(const char* name,glsl_ast_node_t* node){
	glsl_builtin_type_t arg_types[GLSL_AST_NODE_MAX_ARG_COUNT];
	for (u32 i=0;i<node->args.count;i++){
		if (node->args.data[i]->value_type->type!=GLSL_AST_TYPE_TYPE_BUILTIN){
			return 0;
		}
		arg_types[i]=node->args.data[i]->value_type->builtin_type;
	}
	MATCH_FUNC_NAME(texture);
	return 0;
}
