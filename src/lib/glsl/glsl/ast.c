#include <glsl/ast.h>
#include <sys/format/format.h>
#include <sys/heap/heap.h>
#include <sys/io/io.h>
#include <sys/memory/memory.h>
#include <sys/string/string.h>
#include <sys/types.h>



#define TYPE_STRING_BUFFER_SIZE 1024



static const char*const _glsl_ast_node_type_to_string[GLSL_AST_NODE_MAX_TYPE+1]={
	[GLSL_AST_NODE_TYPE_NONE]="<none>",
	[GLSL_AST_NODE_TYPE_ADD]="+",
	[GLSL_AST_NODE_TYPE_ADD_ASSIGN]="+=",
	[GLSL_AST_NODE_TYPE_AND]="&",
	[GLSL_AST_NODE_TYPE_AND_ASSIGN]="&=",
	[GLSL_AST_NODE_TYPE_ARRAY_ACCESS]="<array_access>",
	[GLSL_AST_NODE_TYPE_ASSIGN]="=",
	[GLSL_AST_NODE_TYPE_BIT_INVERSE]="~",
	[GLSL_AST_NODE_TYPE_CALL]="<call>",
	[GLSL_AST_NODE_TYPE_CONSTRUCTOR]="<constructor>",
	[GLSL_AST_NODE_TYPE_COMMA]=",",
	[GLSL_AST_NODE_TYPE_DIVIDE]="/",
	[GLSL_AST_NODE_TYPE_DIVIDE_ASSIGN]="/=",
	[GLSL_AST_NODE_TYPE_EQUALS]="==",
	[GLSL_AST_NODE_TYPE_GREATER_THAN]=">",
	[GLSL_AST_NODE_TYPE_LEFT_SHIFT]="<<",
	[GLSL_AST_NODE_TYPE_LEFT_SHIFT_ASSIGN]="<<=",
	[GLSL_AST_NODE_TYPE_LESS_THAN]="<",
	[GLSL_AST_NODE_TYPE_LOGICAL_AND]="&&",
	[GLSL_AST_NODE_TYPE_LOGICAL_OR]="||",
	[GLSL_AST_NODE_TYPE_MEMBER_ACCESS]=".",
	[GLSL_AST_NODE_TYPE_MODULO]="%",
	[GLSL_AST_NODE_TYPE_MODULO_ASSIGN]="%=",
	[GLSL_AST_NODE_TYPE_MULTIPLY]="*",
	[GLSL_AST_NODE_TYPE_MULTIPLY_ASSIGN]="*=",
	[GLSL_AST_NODE_TYPE_NEGATE]="-",
	[GLSL_AST_NODE_TYPE_NOT]="!",
	[GLSL_AST_NODE_TYPE_NOT_EQUALS]="!=",
	[GLSL_AST_NODE_TYPE_NOT_GREATER_THAN]="<=",
	[GLSL_AST_NODE_TYPE_NOT_LESS_THAN]=">=",
	[GLSL_AST_NODE_TYPE_OR]="|",
	[GLSL_AST_NODE_TYPE_OR_ASSIGN]="|=",
	[GLSL_AST_NODE_TYPE_RIGHT_SHIFT]=">>",
	[GLSL_AST_NODE_TYPE_RIGHT_SHIFT_ASSIGN]=">>=",
	[GLSL_AST_NODE_TYPE_SUBTRACT]="-",
	[GLSL_AST_NODE_TYPE_SUBTRACT_ASSIGN]="-=",
	[GLSL_AST_NODE_TYPE_VAR]="<var>",
	[GLSL_AST_NODE_TYPE_VAR_BOOL]="<var_bool>",
	[GLSL_AST_NODE_TYPE_VAR_FLOAT]="<var_float>",
	[GLSL_AST_NODE_TYPE_VAR_INT]="<var_int>",
	[GLSL_AST_NODE_TYPE_XOR]="^",
	[GLSL_AST_NODE_TYPE_XOR_ASSIGN]="^=",
	[GLSL_AST_NODE_TYPE_BLOCK]="<block>",
	[GLSL_AST_NODE_TYPE_SWIZZLE]="<swizzle>",
};



static u32 _type_to_string(const glsl_ast_type_t* type,char* buffer,u32 length){
	if (type->type==GLSL_AST_TYPE_TYPE_BUILTIN){
		length+=sys_format_string(buffer+length,TYPE_STRING_BUFFER_SIZE-length,"%s",glsl_builtin_type_to_string(type->builtin_type));
	}
	else if (type->type==GLSL_AST_TYPE_TYPE_FUNC){
		length=_type_to_string(type->func.return_and_args[0],buffer,length);
		length+=sys_format_string(buffer+length,TYPE_STRING_BUFFER_SIZE-length," (*)(");
		for (u32 i=0;i<type->func.arg_count;i++){
			length=_type_to_string(type->func.return_and_args[i+1],buffer,length);
			if (i<type->func.arg_count-1){
				length+=sys_format_string(buffer+length,TYPE_STRING_BUFFER_SIZE-length," ");
			}
		}
		if (!type->func.arg_count){
			length+=sys_format_string(buffer+length,TYPE_STRING_BUFFER_SIZE-length,"void");
		}
		length+=sys_format_string(buffer+length,TYPE_STRING_BUFFER_SIZE-length,")");
	}
	else{
		length+=sys_format_string(buffer+length,TYPE_STRING_BUFFER_SIZE-length,"<struct>");
	}
	if (!type->array_length){
		length+=sys_format_string(buffer+length,TYPE_STRING_BUFFER_SIZE-length,"[]");
	}
	else if (type->array_length!=GLSL_AST_TYPE_NO_ARRAY){
		length+=sys_format_string(buffer+length,TYPE_STRING_BUFFER_SIZE-length,"[%u]",type->array_length);
	}
	return length;
}



SYS_PUBLIC void glsl_ast_delete(glsl_ast_t* ast){
	for (u32 i=0;i<ast->block_count;i++){
		glsl_ast_block_t* block=ast->blocks[i];
		sys_heap_dealloc(NULL,block->name);
		sys_heap_dealloc(NULL,block);
	}
	sys_heap_dealloc(NULL,ast->blocks);
	for (u32 i=0;i<ast->var_count;i++){
		glsl_ast_var_t* var=ast->vars[i];
		sys_heap_dealloc(NULL,var->name);
		glsl_ast_type_delete(var->type);
		if (var->value){
			glsl_ast_node_delete(var->value);
		}
		sys_heap_dealloc(NULL,var);
	}
	sys_heap_dealloc(NULL,ast->vars);
	ast->block_count=0;
	ast->named_type_count=0;
	ast->var_count=0;
	ast->blocks=NULL;
	ast->named_types=NULL;
	ast->vars=NULL;
}



SYS_PUBLIC glsl_ast_var_t* glsl_ast_lookup_var(glsl_ast_t* ast,const char* name,glsl_ast_scope_t scope,glsl_ast_type_t* type){
	for (u32 i=0;i<ast->var_count;i++){
		glsl_ast_var_t* var=ast->vars[i];
		if (!sys_string_compare(var->name,name)&&var->scope<=scope&&(!type||glsl_ast_type_is_equal(var->type,type))){
			return var;
		}
	}
	return NULL;
}



SYS_PUBLIC glsl_ast_var_t* glsl_ast_create_var(glsl_ast_t* ast,const char* name,glsl_ast_scope_t scope,glsl_ast_type_t* type,const glsl_ast_var_storage_t* storage){
	ast->var_count++;
	ast->vars=sys_heap_realloc(NULL,ast->vars,ast->var_count*sizeof(glsl_ast_var_t*));
	glsl_ast_var_t* out=sys_heap_alloc(NULL,sizeof(glsl_ast_var_t));
	ast->vars[ast->var_count-1]=out;
	out->name=sys_string_duplicate(name);
	out->usage_flags=0;
	out->flags=(!sys_memory_compare(name,"gl_",3)?GLSL_AST_VAR_FLAG_BUILTIN:0);
	out->scope=scope;
	if (storage){
		out->storage=*storage;
		out->possible_usage_flags=GLSL_AST_VAR_USAGE_FLAG_READ;
		if (out->storage.type!=GLSL_AST_VAR_STORAGE_TYPE_CONST&&out->storage.type!=GLSL_AST_VAR_STORAGE_TYPE_UNIFORM){
			out->possible_usage_flags|=GLSL_AST_VAR_USAGE_FLAG_WRITE;
			if (out->storage.type==GLSL_AST_VAR_STORAGE_TYPE_IN){
				out->flags|=GLSL_AST_VAR_FLAG_INITIALIZED;
			}
		}
		else{
			out->flags|=GLSL_AST_VAR_FLAG_INITIALIZED;
		}
	}
	else{
		out->storage.type=GLSL_AST_VAR_STORAGE_TYPE_DEFAULT;
		out->storage.flags=0;
		out->possible_usage_flags=GLSL_AST_VAR_USAGE_FLAG_READ;
		if (type->type!=GLSL_AST_TYPE_TYPE_FUNC){
			out->possible_usage_flags|=GLSL_AST_VAR_USAGE_FLAG_WRITE;
		}
		else{
			out->flags|=GLSL_AST_VAR_FLAG_INITIALIZED;
		}
	}
	out->type=type;
	out->value=NULL;
	return out;
}



SYS_PUBLIC glsl_ast_type_t* glsl_ast_type_create(glsl_ast_type_type_t type){
	glsl_ast_type_t* out=sys_heap_alloc(NULL,sizeof(glsl_ast_type_t));
	out->type=type;
	out->rc=1;
	out->array_length=GLSL_AST_TYPE_NO_ARRAY;
	return out;
}



SYS_PUBLIC void glsl_ast_type_delete(glsl_ast_type_t* type){
	type->rc--;
	if (type->rc){
		return;
	}
	if (type->type==GLSL_AST_TYPE_TYPE_FUNC){
		for (u32 i=0;i<=type->func.arg_count;i++){
			glsl_ast_type_delete(type->func.return_and_args[i]);
		}
		sys_heap_dealloc(NULL,type->func.return_and_args);
	}
	sys_heap_dealloc(NULL,type);
}



SYS_PUBLIC glsl_ast_type_t* glsl_ast_type_duplicate(glsl_ast_type_t* type){
	type->rc++;
	return type;
}



SYS_PUBLIC char* glsl_ast_type_to_string(const glsl_ast_type_t* type){
	char buffer[TYPE_STRING_BUFFER_SIZE];
	u32 length=_type_to_string(type,buffer,0);
	char* out=sys_heap_alloc(NULL,length+1);
	sys_memory_copy(buffer,out,length+1);
	return out;
}



SYS_PUBLIC _Bool glsl_ast_type_is_equal(const glsl_ast_type_t* a,const glsl_ast_type_t* b){
	if (a->type!=b->type){
		return 0;
	}
	if (a->type==GLSL_AST_TYPE_TYPE_BUILTIN){
		return a->builtin_type==b->builtin_type;
	}
	if (a->type==GLSL_AST_TYPE_TYPE_FUNC){
		if (a->func.arg_count!=b->func.arg_count){
			return 0;
		}
		for (u32 i=0;i<=a->func.arg_count;i++){
			if (!glsl_ast_type_is_equal(a->func.return_and_args[i],b->func.return_and_args[i])){
				return 0;
			}
		}
		return 1;
	}
	sys_io_print("~~~ glsl_ast_type_is_equal: structure ~~~\n");
	return 0;
}



SYS_PUBLIC u32 glsl_ast_type_get_slot_count(const glsl_ast_type_t* type){
	if (type->type==GLSL_AST_TYPE_TYPE_BUILTIN){
		return glsl_builtin_type_to_slot_count(type->builtin_type);
	}
	if (type->type==GLSL_AST_TYPE_TYPE_FUNC){
		return 0;
	}
	sys_io_print("~~~ glsl_ast_type_is_equal: structure ~~~\n");
	return 0;
}



SYS_PUBLIC glsl_ast_node_t* glsl_ast_node_create(glsl_ast_node_type_t type){
	glsl_ast_node_t* out=sys_heap_alloc(NULL,sizeof(glsl_ast_node_t));
	out->type=type;
	out->value_type=NULL;
	return out;
}



SYS_PUBLIC void glsl_ast_node_delete(glsl_ast_node_t* node){
	if (node->value_type){
		glsl_ast_type_delete(node->value_type);
	}
	switch (node->type){
		case GLSL_AST_NODE_TYPE_NONE:
			break;
		case GLSL_AST_NODE_TYPE_ADD:
		case GLSL_AST_NODE_TYPE_ADD_ASSIGN:
		case GLSL_AST_NODE_TYPE_AND:
		case GLSL_AST_NODE_TYPE_AND_ASSIGN:
		case GLSL_AST_NODE_TYPE_ASSIGN:
		case GLSL_AST_NODE_TYPE_DIVIDE:
		case GLSL_AST_NODE_TYPE_DIVIDE_ASSIGN:
		case GLSL_AST_NODE_TYPE_EQUALS:
		case GLSL_AST_NODE_TYPE_GREATER_THAN:
		case GLSL_AST_NODE_TYPE_LEFT_SHIFT:
		case GLSL_AST_NODE_TYPE_LEFT_SHIFT_ASSIGN:
		case GLSL_AST_NODE_TYPE_LESS_THAN:
		case GLSL_AST_NODE_TYPE_LOGICAL_AND:
		case GLSL_AST_NODE_TYPE_LOGICAL_OR:
		case GLSL_AST_NODE_TYPE_MODULO:
		case GLSL_AST_NODE_TYPE_MODULO_ASSIGN:
		case GLSL_AST_NODE_TYPE_MULTIPLY:
		case GLSL_AST_NODE_TYPE_MULTIPLY_ASSIGN:
		case GLSL_AST_NODE_TYPE_NOT_EQUALS:
		case GLSL_AST_NODE_TYPE_NOT_GREATER_THAN:
		case GLSL_AST_NODE_TYPE_NOT_LESS_THAN:
		case GLSL_AST_NODE_TYPE_OR:
		case GLSL_AST_NODE_TYPE_OR_ASSIGN:
		case GLSL_AST_NODE_TYPE_RIGHT_SHIFT:
		case GLSL_AST_NODE_TYPE_RIGHT_SHIFT_ASSIGN:
		case GLSL_AST_NODE_TYPE_SUBTRACT:
		case GLSL_AST_NODE_TYPE_SUBTRACT_ASSIGN:
		case GLSL_AST_NODE_TYPE_XOR:
		case GLSL_AST_NODE_TYPE_XOR_ASSIGN:
			glsl_ast_node_delete(node->binary[0]);
			glsl_ast_node_delete(node->binary[1]);
			break;
		case GLSL_AST_NODE_TYPE_ARRAY_ACCESS:
			break;
		case GLSL_AST_NODE_TYPE_BIT_INVERSE:
		case GLSL_AST_NODE_TYPE_NEGATE:
		case GLSL_AST_NODE_TYPE_NOT:
			glsl_ast_node_delete(node->unary);
			break;
		case GLSL_AST_NODE_TYPE_CALL:
		case GLSL_AST_NODE_TYPE_CONSTRUCTOR:
			for (u32 i=0;i<node->arg_count;i++){
				glsl_ast_node_delete(glsl_ast_get_arg(node,i));
			}
			if (node->arg_count>GLSL_AST_NODE_INLINE_ARG_COUNT){
				sys_heap_dealloc(NULL,node->args);
			}
			break;
		case GLSL_AST_NODE_TYPE_COMMA:
			break;
		case GLSL_AST_NODE_TYPE_MEMBER_ACCESS:
			glsl_ast_node_delete(node->member_access.value);
			sys_heap_dealloc(NULL,node->member_access.member);
			break;
		case GLSL_AST_NODE_TYPE_BLOCK:
			for (u32 i=0;i<node->block.length;i++){
				glsl_ast_node_delete(node->block.data[i]);
			}
			sys_heap_dealloc(NULL,node->block.data);
			break;
		case GLSL_AST_NODE_TYPE_SWIZZLE:
			glsl_ast_node_delete(node->swizzle.value);
			break;
	}
	sys_heap_dealloc(NULL,node);
}



SYS_PUBLIC const char* glsl_ast_node_type_to_string(glsl_ast_node_type_t type){
	return _glsl_ast_node_type_to_string[type];
}
