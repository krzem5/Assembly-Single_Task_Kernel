#include <glsl/_internal/error.h>
#include <glsl/_internal/interface_allocator.h>
#include <glsl/ast.h>
#include <glsl/compiler.h>
#include <glsl/error.h>
#include <glsl/operators.h>
#include <sys/heap/heap.h>
#include <sys/io/io.h>
#include <sys/memory/memory.h>
#include <sys/string/string.h>
#include <sys/types.h>



#define VAR_LIST_GROWTH_SIZE 8
#define INSTRUCTION_LIST_GROWTH_SIZE 16
#define CONST_LIST_GROWTH_SIZE 16



typedef struct _COMPILER_STATE{
	glsl_compilation_output_t* output;
	glsl_interface_allocator_t temporary_variable_allocator;
} compiler_state_t;



typedef struct _REGISTER_STATE{
	_Bool is_initialized;
	u8 offset;
	glsl_builtin_type_t builtin_type;
	glsl_instruction_arg_t arg;
} register_state_t;



static const glsl_compilation_output_var_type_t _glsl_ast_storage_type_to_output_var_type[GLSL_AST_VAR_STORAGE_MAX_TYPE+1]={
	[GLSL_AST_VAR_STORAGE_TYPE_IN]=GLSL_COMPILATION_OUTPUT_VAR_TYPE_INPUT,
	[GLSL_AST_VAR_STORAGE_TYPE_OUT]=GLSL_COMPILATION_OUTPUT_VAR_TYPE_OUTPUT,
	[GLSL_AST_VAR_STORAGE_TYPE_UNIFORM]=GLSL_COMPILATION_OUTPUT_VAR_TYPE_UNIFORM,
};



static u16 _push_var(glsl_compilation_output_t* output,const char* name,glsl_compilation_output_var_type_t type,u32 slot){
	if (output->var_count==output->_var_capacity){
		output->_var_capacity+=VAR_LIST_GROWTH_SIZE;
		output->vars=sys_heap_realloc(NULL,output->vars,output->_var_capacity*sizeof(glsl_compilation_output_var_t));
	}
	glsl_compilation_output_var_t* out=output->vars+output->var_count;
	output->var_count++;
	out->type=type;
	out->slot=slot;
	out->name=sys_string_duplicate(name);
	return output->var_count-1;
}



static glsl_instruction_t* _push_instruction(glsl_compilation_output_t* output,glsl_instruction_type_t type,u32 arg_count){
	if (output->instruction_count==output->_instruction_capacity){
		output->_instruction_capacity+=INSTRUCTION_LIST_GROWTH_SIZE;
		output->instructions=sys_heap_realloc(NULL,output->instructions,output->_instruction_capacity*sizeof(glsl_instruction_t*));
	}
	glsl_instruction_t* out=sys_heap_alloc(NULL,sizeof(glsl_instruction_t));
	output->instructions[output->instruction_count]=out;
	output->instruction_count++;
	out->type=type;
	out->arg_count=arg_count;
	return out;
}



static u16 _push_consts(glsl_compilation_output_t* output,const float* values,u8 count){
	if (output->const_count+count>=output->_const_capacity){
		output->_const_capacity+=CONST_LIST_GROWTH_SIZE;
		output->consts=sys_heap_realloc(NULL,output->consts,output->_const_capacity*sizeof(float));
	}
	u16 out=output->const_count;
	sys_memory_copy(values,output->consts+output->const_count,count*sizeof(float));
	while (count&3){
		output->consts[output->const_count+count]=0;
		count++;
	}
	output->const_count+=count;
	return out;
}



static _Bool _is_var_used(const glsl_ast_var_t* var){
	return ((var->storage.type==GLSL_AST_VAR_STORAGE_TYPE_DEFAULT&&var->usage_flags==(GLSL_AST_VAR_USAGE_FLAG_READ|GLSL_AST_VAR_USAGE_FLAG_WRITE))||(var->storage.type!=GLSL_AST_VAR_STORAGE_TYPE_DEFAULT&&var->usage_flags))&&var->type->type!=GLSL_AST_TYPE_TYPE_FUNC;
}



// static _Bool _is_arg(const glsl_ast_node_t* node){
// 	return node->type==GLSL_AST_NODE_TYPE_VAR||node->type==GLSL_AST_NODE_TYPE_VAR_CONST||(node->type==GLSL_AST_NODE_TYPE_SWIZZLE&&(node->swizzle.value->type==GLSL_AST_NODE_TYPE_VAR||node->swizzle.value->type==GLSL_AST_NODE_TYPE_VAR_CONST));
// }



static void _calculate_output_target(compiler_state_t* state,const glsl_ast_node_t* node,glsl_builtin_type_t builtin_type,register_state_t* out){
	out->is_initialized=1;
	out->offset=0;
	out->builtin_type=builtin_type;
	if (!node){
		u32 slot;
		_glsl_interface_allocator_reserve(&(state->temporary_variable_allocator),&slot,glsl_builtin_type_to_slot_count(builtin_type));
		out->arg.index=slot;
		out->arg.pattern_length_and_flags=glsl_builtin_type_to_vector_length(builtin_type);
		out->arg.pattern=0b11100100&(1<<(out->arg.pattern_length_and_flags<<1));
		out->arg.pattern_length_and_flags|=GLSL_INSTRUCTION_ARG_FLAG_LOCAL;
		return;
	}
	if (node->type==GLSL_AST_NODE_TYPE_VAR_CONST){
		out->arg.index=_push_consts(state->output,node->var_matrix,glsl_builtin_type_to_size(builtin_type));
		out->arg.pattern_length_and_flags=glsl_builtin_type_to_vector_length(builtin_type);
		out->arg.pattern=0b11100100&(1<<(out->arg.pattern_length_and_flags<<1));
		out->arg.pattern_length_and_flags|=GLSL_INSTRUCTION_ARG_FLAG_CONST;
		return;
	}
	if (node->type!=GLSL_AST_NODE_TYPE_SWIZZLE){
		out->arg.pattern_length_and_flags=glsl_builtin_type_to_vector_length(node->value_type->builtin_type);
		out->arg.pattern=0b11100100&(1<<(out->arg.pattern_length_and_flags<<1));
	}
	else{
		out->arg.pattern_length_and_flags=node->swizzle.pattern_length;
		out->arg.pattern=node->swizzle.pattern;
		node=node->swizzle.value;
	}
	if (node->type==GLSL_AST_NODE_TYPE_VAR){
		if (node->var->storage.type==GLSL_AST_VAR_STORAGE_TYPE_DEFAULT){
			out->arg.index=node->var->_compiler_data;
			out->arg.pattern_length_and_flags|=GLSL_INSTRUCTION_ARG_FLAG_LOCAL;
		}
		else{
			out->arg.index=node->var->_compiler_data;
		}
		return;
	}
	sys_io_print("_calculate_output_target: GLSL_AST_NODE_TYPE_VAR_CONST swizzle\n");
}



static void _generate_move(compiler_state_t* state,const register_state_t* src,const register_state_t* dst){
	sys_io_print("_generate_move\n");
}



static void _generate_add(compiler_state_t* state,const register_state_t* a,const register_state_t* b,const register_state_t* out){
	glsl_instruction_t* instruction=_push_instruction(state->output,GLSL_INSTRUCTION_TYPE_ADD,3);
	instruction->args[0]=out->arg;
	instruction->args[1]=a->arg;
	instruction->args[2]=b->arg;
}



static void _generate_multiply(compiler_state_t* state,const register_state_t* a,const register_state_t* b,const register_state_t* out){
	glsl_instruction_t* instruction=_push_instruction(state->output,GLSL_INSTRUCTION_TYPE_MUL,3);
	instruction->args[0]=out->arg;
	instruction->args[1]=a->arg;
	instruction->args[2]=b->arg;
}



static void _generate_subtract(compiler_state_t* state,const register_state_t* a,const register_state_t* b,const register_state_t* out){
	glsl_instruction_t* instruction=_push_instruction(state->output,GLSL_INSTRUCTION_TYPE_SUB,3);
	instruction->args[0]=out->arg;
	instruction->args[1]=a->arg;
	instruction->args[2]=b->arg;
}



static void _generate_dot_product_3d(compiler_state_t* state,const register_state_t* a,const register_state_t* b,const register_state_t* out){
	glsl_instruction_t* instruction=_push_instruction(state->output,GLSL_INSTRUCTION_TYPE_DP3,3);
	instruction->args[0]=out->arg;
	instruction->args[1]=a->arg;
	instruction->args[2]=b->arg;
}



static glsl_error_t _visit_node(const glsl_ast_node_t* node,compiler_state_t* state,register_state_t* output_register){
	switch (node->type){
		default:
		case GLSL_AST_NODE_TYPE_NONE:
			return GLSL_NO_ERROR;
		case GLSL_AST_NODE_TYPE_ARRAY_ACCESS:
			sys_io_print("GLSL_AST_NODE_TYPE_ARRAY_ACCESS\n");
			return GLSL_NO_ERROR;
		case GLSL_AST_NODE_TYPE_BLOCK:
			for (u32 i=0;i<node->block.length;i++){
				register_state_t tmp={
					0
				};
				glsl_error_t error=_visit_node(node->block.data[i],state,&tmp);
				if (error!=GLSL_NO_ERROR){
					return error;
				}
			}
			return GLSL_NO_ERROR;
		case GLSL_AST_NODE_TYPE_CALL:
			sys_io_print("GLSL_AST_NODE_TYPE_CALL\n");
			return GLSL_NO_ERROR;
		case GLSL_AST_NODE_TYPE_CONSTRUCTOR:
			{
				if (!output_register->is_initialized){
					_calculate_output_target(state,NULL,node->value_type->builtin_type,output_register);
				}
				if (output_register->offset){
					sys_io_print("GLSL_AST_NODE_TYPE_CONSTRUCTOR: nested constructor // copy not implemented\n");
					return GLSL_NO_ERROR;
				}
				register_state_t tmp=*output_register;
				for (u32 i=0;i<node->args.count;i++){
					glsl_error_t error=_visit_node(node->args.data[i],state,&tmp);
					tmp.offset+=glsl_builtin_type_to_size(node->args.data[i]->value_type->builtin_type);
					if (error!=GLSL_NO_ERROR){
						return error;
					}
				}
				return GLSL_NO_ERROR;
			}
		case GLSL_AST_NODE_TYPE_INLINE_BLOCK:
			sys_io_print("GLSL_AST_NODE_TYPE_INLINE_BLOCK\n");
			return GLSL_NO_ERROR;
		case GLSL_AST_NODE_TYPE_MEMBER_ACCESS:
			sys_io_print("GLSL_AST_NODE_TYPE_MEMBER_ACCESS\n");
			return GLSL_NO_ERROR;
		case GLSL_AST_NODE_TYPE_OPERATOR:
			break;
		case GLSL_AST_NODE_TYPE_SWIZZLE:
		case GLSL_AST_NODE_TYPE_VAR:
		case GLSL_AST_NODE_TYPE_VAR_CONST:
			if (!output_register->is_initialized){
				_calculate_output_target(state,node,node->value_type->builtin_type,output_register);
			}
			else{
				register_state_t tmp;
				_calculate_output_target(state,node,node->value_type->builtin_type,&tmp);
				_generate_move(state,&tmp,output_register);
			}
			return GLSL_NO_ERROR;
	}
	if (node->operator_type>=GLSL_AST_NODE_OPERATOR_TYPE_ASSIGN_BOOL_BOOL&&node->operator_type<=GLSL_AST_NODE_OPERATOR_TYPE_ASSIGN_MAT44_MAT44){
		register_state_t reg;
		_calculate_output_target(state,node->args.data[0],node->value_type->builtin_type,&reg);
		glsl_error_t error=_visit_node(node->args.data[1],state,&reg);
		if (error!=GLSL_NO_ERROR){
			return error;
		}
		if (output_register->is_initialized){
			_generate_move(state,&reg,output_register);
		}
		else{
			*output_register=reg;
		}
		return GLSL_NO_ERROR;
	}
	register_state_t regs[7];
	for (u32 i=0;i<node->args.count;i++){
		(regs+i)->is_initialized=0;
		glsl_error_t error=_visit_node(node->args.data[i],state,regs+i);
		if (error!=GLSL_NO_ERROR){
			return error;
		}
	}
	// _Bool is_output_used_before_last_arg=0;
	if (!output_register->is_initialized){
		_calculate_output_target(state,NULL,node->value_type->builtin_type,output_register);
	}
	// else{
	// 	for (u32 i=0;i<node->args.count-1;i++){
	// 		if ((regs+i)->arg.index==output_register->arg.index&&!(((regs+i)->arg.pattern_length_and_flags^output_register->arg.pattern_length_and_flags)&GLSL_INSTRUCTION_ARG_FLAG_LOCAL)){
	// 			is_output_used_before_last_arg=1;
	// 			break;
	// 		}
	// 	}
	// }
	switch (node->operator_type){
		case GLSL_AST_NODE_OPERATOR_TYPE_ADD_FLOAT_FLOAT:
		case GLSL_AST_NODE_OPERATOR_TYPE_ADD_VEC2_VEC2:
		case GLSL_AST_NODE_OPERATOR_TYPE_ADD_VEC3_VEC3:
		case GLSL_AST_NODE_OPERATOR_TYPE_ADD_VEC4_VEC4:
			_generate_add(state,regs,regs+1,output_register);
			break;
		case GLSL_AST_NODE_OPERATOR_TYPE_MULTIPLY_FLOAT_FLOAT:
		case GLSL_AST_NODE_OPERATOR_TYPE_MULTIPLY_VEC2_VEC2:
		case GLSL_AST_NODE_OPERATOR_TYPE_MULTIPLY_VEC3_VEC3:
		case GLSL_AST_NODE_OPERATOR_TYPE_MULTIPLY_VEC4_VEC4:
			_generate_multiply(state,regs,regs+1,output_register);
			break;
		case GLSL_AST_NODE_OPERATOR_TYPE_MULTIPLY_MAT33_VEC3:
			{
				register_state_t tmp;
				_calculate_output_target(state,NULL,GLSL_BUILTIN_TYPE_VEC3,&tmp);
				_generate_dot_product_3d(state,regs,regs+1,&tmp);
				tmp.arg.pattern_length_and_flags=1|GLSL_INSTRUCTION_ARG_FLAG_LOCAL;
				tmp.arg.pattern=0b00;
				_generate_move(state,&tmp,output_register);
				regs->offset+=4;
				output_register->offset++;
				_generate_dot_product_3d(state,regs,regs+1,&tmp);
				tmp.arg.pattern_length_and_flags=1|GLSL_INSTRUCTION_ARG_FLAG_LOCAL;
				tmp.arg.pattern=0b00;
				_generate_move(state,&tmp,output_register);
				regs->offset+=4;
				output_register->offset++;
				_generate_dot_product_3d(state,regs,regs+1,&tmp);
				tmp.arg.pattern_length_and_flags=1|GLSL_INSTRUCTION_ARG_FLAG_LOCAL;
				tmp.arg.pattern=0b00;
				_generate_move(state,&tmp,output_register);
				output_register->offset-=2;
				break;
			}
		case GLSL_AST_NODE_OPERATOR_TYPE_SUBTRACT_FLOAT_FLOAT:
		case GLSL_AST_NODE_OPERATOR_TYPE_SUBTRACT_VEC2_VEC2:
		case GLSL_AST_NODE_OPERATOR_TYPE_SUBTRACT_VEC3_VEC3:
		case GLSL_AST_NODE_OPERATOR_TYPE_SUBTRACT_VEC4_VEC4:
			_generate_subtract(state,regs,regs+1,output_register);
			break;
		default:
			sys_io_print("<%s>\n",glsl_operator_type_to_string(node->operator_type));
			break;
	}
	return GLSL_NO_ERROR;
}



static const glsl_ast_var_t* _allocate_vars(const glsl_ast_t* ast,compiler_state_t* state){
	const glsl_ast_var_t* ret=NULL;
	for (u32 i=0;i<ast->var_count;i++){
		glsl_ast_var_t* var=ast->vars[i];
		if (var->storage.type!=GLSL_AST_VAR_STORAGE_TYPE_DEFAULT&&var->storage.type!=GLSL_AST_VAR_STORAGE_TYPE_CONST&&_is_var_used(var)){
			var->_compiler_data=_push_var(state->output,var->name,((var->storage.flags&GLSL_AST_VAR_STORAGE_FLAG_HAS_LAYOUT_LOCATION)?var->storage.layout_location:0xffffffff),_glsl_ast_storage_type_to_output_var_type[var->storage.type]);
		}
		else if (var->type->type==GLSL_AST_TYPE_TYPE_FUNC&&!sys_string_compare(var->name,"main")){
			ret=var;
		}
		else if (var->storage.type==GLSL_AST_VAR_STORAGE_TYPE_DEFAULT&&_is_var_used(var)){
			u32 slot;
			_glsl_interface_allocator_reserve(&(state->temporary_variable_allocator),&slot,glsl_ast_type_get_slot_count(var->type));
			var->_compiler_data=slot;
		}
	}
	return ret;
}



SYS_PUBLIC glsl_error_t glsl_compiler_compile(const glsl_ast_t* ast,glsl_compilation_output_t* out){
	out->var_count=0;
	out->instruction_count=0;
	out->const_count=0;
	out->_var_capacity=0;
	out->_instruction_capacity=0;
	out->_const_capacity=0;
	out->vars=NULL;
	out->instructions=NULL;
	out->consts=NULL;
	for (glsl_compilation_output_var_type_t i=0;i<=GLSL_COMPILATION_OUTPUT_VAR_MAX_TYPE;i++){
		out->slot_counts[i]=0;
	}
	compiler_state_t state={
		out
	};
	_glsl_interface_allocator_init(1024,&(state.temporary_variable_allocator));
	const glsl_ast_var_t* main_function=_allocate_vars(ast,&state);
	glsl_error_t error=GLSL_NO_ERROR;
	if (!main_function||!main_function->value){
		sys_io_print("Main function not found (error)\n");
		goto _cleanup;
	}
	register_state_t tmp={
		0
	};
	error=_visit_node(main_function->value,&state,&tmp);
	if (error!=GLSL_NO_ERROR){
		goto _cleanup;
	}
	return GLSL_NO_ERROR;
_cleanup:
	_glsl_interface_allocator_deinit(&(state.temporary_variable_allocator));
	glsl_compiler_compilation_output_delete(out);
	return error;
}



SYS_PUBLIC void glsl_compiler_compilation_output_delete(glsl_compilation_output_t* output){
	return;
}
