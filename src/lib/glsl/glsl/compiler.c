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
#define CONST_LIST_GROWTH_SIZE 4



typedef struct _COMPILER_STATE{
	glsl_compilation_output_t* output;
	glsl_interface_allocator_t const_variable_allocator;
	glsl_interface_allocator_t local_variable_allocator;
} compiler_state_t;



typedef struct _REGISTER_STATE{
	_Bool is_initialized;
	glsl_builtin_type_t builtin_type;
	u16 base;
	u16 offset;
	u8 flags;
	u8 pattern_length;
	u8 pattern;
} register_state_t;



static const glsl_compilation_output_var_type_t _glsl_ast_storage_type_to_output_var_type[GLSL_AST_VAR_STORAGE_MAX_TYPE+1]={
	[GLSL_AST_VAR_STORAGE_TYPE_IN]=GLSL_COMPILATION_OUTPUT_VAR_TYPE_INPUT,
	[GLSL_AST_VAR_STORAGE_TYPE_OUT]=GLSL_COMPILATION_OUTPUT_VAR_TYPE_OUTPUT,
	[GLSL_AST_VAR_STORAGE_TYPE_UNIFORM]=GLSL_COMPILATION_OUTPUT_VAR_TYPE_UNIFORM,
};



static u16 _push_var(glsl_compilation_output_t* output,const char* name,u32 slot,glsl_compilation_output_var_type_t type,u16 slot_count){
	if (output->var_count==output->_var_capacity){
		output->_var_capacity+=VAR_LIST_GROWTH_SIZE;
		output->vars=sys_heap_realloc(NULL,output->vars,output->_var_capacity*sizeof(glsl_compilation_output_var_t));
	}
	glsl_compilation_output_var_t* out=output->vars+output->var_count;
	output->var_count++;
	out->type=type;
	out->slot_count=slot_count;
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



static void _push_consts(glsl_compilation_output_t* output,u32 offset,const float* values,u8 count){
	if (offset+count>=output->const_count){
		u32 const_count=(offset+count+CONST_LIST_GROWTH_SIZE-1)&(-CONST_LIST_GROWTH_SIZE);
		output->consts=sys_heap_realloc(NULL,output->consts,const_count*sizeof(float));
		do{
			output->consts[output->const_count]=0;
			output->const_count++;
		} while (output->const_count<const_count);
	}
	sys_memory_copy(values,output->consts+offset,count*sizeof(float));
}



static _Bool _is_var_used(const glsl_ast_var_t* var){
	return ((var->storage.type==GLSL_AST_VAR_STORAGE_TYPE_DEFAULT&&var->usage_flags==(GLSL_AST_VAR_USAGE_FLAG_READ|GLSL_AST_VAR_USAGE_FLAG_WRITE))
		||(var->storage.type!=GLSL_AST_VAR_STORAGE_TYPE_DEFAULT&&var->usage_flags))&&var->type->type!=GLSL_AST_TYPE_TYPE_FUNC;
}



static void _calculate_output_target(compiler_state_t* state,const glsl_ast_node_t* node,glsl_builtin_type_t builtin_type,register_state_t* out){
	out->is_initialized=1;
	out->builtin_type=builtin_type;
	out->offset=0;
	out->flags=0;
	out->pattern_length=0;
	out->pattern=0;
	if (!node){
		u32 slot=0xffffffff;
		_glsl_interface_allocator_reserve(&(state->local_variable_allocator),&slot,glsl_builtin_type_to_slot_count(builtin_type),1);
		out->base=slot;
		out->flags|=GLSL_INSTRUCTION_ARG_FLAG_LOCAL;
		u32 end=slot+glsl_builtin_type_to_slot_count(builtin_type);
		if (end>state->output->local_count){
			state->output->local_count=end;
		}
		return;
	}
	if (node->type==GLSL_AST_NODE_TYPE_VAR_CONST){
		u32 slot=0xffffffff;
		_glsl_interface_allocator_reserve(&(state->const_variable_allocator),&slot,glsl_builtin_type_to_size(builtin_type),glsl_builtin_type_to_vector_length(builtin_type));
		out->base=slot>>2;
		if (glsl_builtin_type_to_size(builtin_type)==1){
			out->pattern_length=1;
			out->pattern=slot&3;
		}
		_push_consts(state->output,slot,node->var_matrix,glsl_builtin_type_to_size(builtin_type));
		out->flags|=GLSL_INSTRUCTION_ARG_FLAG_CONST;
		return;
	}
	if (node->type==GLSL_AST_NODE_TYPE_SWIZZLE){
		out->pattern_length=node->swizzle.pattern_length;
		out->pattern=node->swizzle.pattern;
		node=node->swizzle.value;
	}
	if (node->type==GLSL_AST_NODE_TYPE_VAR){
		if (node->var->storage.type==GLSL_AST_VAR_STORAGE_TYPE_DEFAULT){
			out->base=node->var->_compiler_data;
			out->flags|=GLSL_INSTRUCTION_ARG_FLAG_LOCAL;
		}
		else{
			out->base=node->var->_compiler_data;
		}
		return;
	}
	sys_io_print("_calculate_output_target: GLSL_AST_NODE_TYPE_VAR_CONST swizzle\n");
}



static void _release_local_regs(compiler_state_t* state,const register_state_t* reg){
	if (reg->flags&GLSL_INSTRUCTION_ARG_FLAG_LOCAL){
		_glsl_interface_allocator_release(&(state->local_variable_allocator),reg->base,glsl_builtin_type_to_slot_count(reg->builtin_type));
	}
}



static void _generate_instruction_arg(const register_state_t* reg,glsl_instruction_arg_t* arg,u8 max_size){
	arg->index=reg->base;
	if (reg->pattern_length){
		arg->pattern_length_and_flags=reg->pattern_length-1;
		arg->pattern=reg->pattern;
	}
	else{
		arg->pattern_length_and_flags=glsl_builtin_type_to_vector_length(reg->builtin_type)-1;
		arg->pattern=0b11100100;
	}
	if (arg->pattern_length_and_flags>max_size){
		arg->pattern_length_and_flags=max_size;
	}
	arg->pattern+=0b01010101*(reg->offset&3);
	arg->pattern_length_and_flags|=(reg->offset&0xfc)|(reg->flags&(GLSL_INSTRUCTION_ARG_FLAG_CONST|GLSL_INSTRUCTION_ARG_FLAG_LOCAL));
}



static void _generate_move(compiler_state_t* state,const register_state_t* src,const register_state_t* dst){
	u8 src_size=glsl_builtin_type_to_size(src->builtin_type);
	u8 dst_size=glsl_builtin_type_to_size(dst->builtin_type);
	register_state_t tmp_src=*src;
	register_state_t tmp_dst=*dst;
	while (tmp_src.offset<src_size&&tmp_dst.offset<dst_size){
		u8 count=src_size-tmp_src.offset;
		if (dst_size-tmp_dst.offset<count){
			count=dst_size-tmp_dst.offset;
		}
		if (4-(tmp_src.offset&3)<count){
			count=4-(tmp_src.offset&3);
		}
		if (4-(tmp_dst.offset&3)<count){
			count=4-(tmp_dst.offset&3);
		}
		glsl_instruction_t* instruction=_push_instruction(state->output,GLSL_INSTRUCTION_TYPE_MOV,2);
		_generate_instruction_arg(&tmp_dst,instruction->args,count-1);
		_generate_instruction_arg(&tmp_src,instruction->args+1,count-1);
		tmp_src.offset+=count;
		tmp_dst.offset+=count;
	}
}



static u8 _calculate_max_output_size(const glsl_instruction_t* instruction){
	u8 out=0xff;
	for (u32 i=1;i<instruction->arg_count;i++){
		u8 size=(instruction->args+i)->pattern_length_and_flags&3;
		if (size<out){
			out=size;
		}
	}
	return out;
}



static void _generate_add(compiler_state_t* state,const register_state_t* a,const register_state_t* b,const register_state_t* out){
	glsl_instruction_t* instruction=_push_instruction(state->output,GLSL_INSTRUCTION_TYPE_ADD,3);
	_generate_instruction_arg(a,instruction->args+1,4);
	_generate_instruction_arg(b,instruction->args+2,4);
	_generate_instruction_arg(out,instruction->args,_calculate_max_output_size(instruction));
}



static void _generate_multiply(compiler_state_t* state,const register_state_t* a,const register_state_t* b,const register_state_t* out){
	glsl_instruction_t* instruction=_push_instruction(state->output,GLSL_INSTRUCTION_TYPE_MUL,3);
	_generate_instruction_arg(a,instruction->args+1,4);
	_generate_instruction_arg(b,instruction->args+2,4);
	_generate_instruction_arg(out,instruction->args,_calculate_max_output_size(instruction));
}



static void _generate_subtract(compiler_state_t* state,const register_state_t* a,const register_state_t* b,const register_state_t* out){
	glsl_instruction_t* instruction=_push_instruction(state->output,GLSL_INSTRUCTION_TYPE_SUB,3);
	_generate_instruction_arg(a,instruction->args+1,4);
	_generate_instruction_arg(b,instruction->args+2,4);
	_generate_instruction_arg(out,instruction->args,_calculate_max_output_size(instruction));
}



static void _generate_dot_product_3d(compiler_state_t* state,const register_state_t* a,const register_state_t* b,const register_state_t* out){
	glsl_instruction_t* instruction=_push_instruction(state->output,GLSL_INSTRUCTION_TYPE_DP3,3);
	_generate_instruction_arg(a,instruction->args+1,4);
	_generate_instruction_arg(b,instruction->args+2,4);
	_generate_instruction_arg(out,instruction->args,_calculate_max_output_size(instruction));
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
				_Bool base_type_is_float=glsl_builtin_type_to_vector_base_type(node->value_type->builtin_type)==GLSL_BUILTIN_TYPE_FLOAT;
				register_state_t tmp=*output_register;
				u16 mask=0;
				float const_values[16];
				u32* const_values_as_int=(u32*)const_values;
				for (u32 i=0;i<node->args.count;i++){
					u32 size=glsl_builtin_type_to_size(node->args.data[i]->value_type->builtin_type);
					if (node->args.data[i]->type!=GLSL_AST_NODE_TYPE_VAR_CONST){
						goto _continue_not_const;
					}
					if ((glsl_builtin_type_to_vector_base_type(node->args.data[i]->value_type->builtin_type)==GLSL_BUILTIN_TYPE_FLOAT)!=base_type_is_float){
						if (base_type_is_float){
							for (u16 i=0;i<size;i++){
								const_values[tmp.offset+i]=node->args.data[i]->var_int_matrix[i];
							}
						}
						else{
							for (u16 i=0;i<size;i++){
								const_values_as_int[tmp.offset+i]=node->args.data[i]->var_matrix[i];
							}
						}
					}
					else{
						sys_memory_copy(node->args.data[i]->var_matrix,const_values+tmp.offset,size*sizeof(float));
					}
					mask|=((1<<size)-1)<<tmp.offset;
_continue_not_const:
					tmp.offset+=size;
				}
				u32 row_bit_count=glsl_builtin_type_to_size(node->value_type->builtin_type)/glsl_builtin_type_to_vector_count(node->value_type->builtin_type);
				for (u32 i=0;i<glsl_builtin_type_to_vector_count(node->value_type->builtin_type);i++){
					u8 row=(mask>>(i*row_bit_count))&0xf;
					if (!row){
						continue;
					}
					float const_vector[4];
					u8 pattern_length=0;
					u8 pattern=0;
					do{
						u8 j=__builtin_ffs(row)-1;
						row&=row-1;
						const_vector[pattern_length]=const_values[i*row_bit_count+j];
						pattern|=j<<(pattern_length<<1);
						pattern_length++;
					} while (row);
					u32 slot=0xffffffff;
					_glsl_interface_allocator_reserve(&(state->const_variable_allocator),&slot,pattern_length,(pattern_length==1?1:(pattern_length==2?2:4)));
					_push_consts(state->output,slot,const_vector,pattern_length);
					glsl_instruction_t* instruction=_push_instruction(state->output,GLSL_INSTRUCTION_TYPE_MOV,2);
					instruction->args->index=output_register->base;
					instruction->args->pattern_length_and_flags=(pattern_length-1)|(i<<2)|output_register->flags;
					instruction->args->pattern=pattern;
					(instruction->args+1)->index=slot>>2;
					(instruction->args+1)->pattern_length_and_flags=(pattern_length-1)|GLSL_INSTRUCTION_ARG_FLAG_CONST;
					(instruction->args+1)->pattern=0b11100100+0b01010101*(slot&3);
				}
				tmp.offset=0;
				for (u32 i=0;i<node->args.count;i++){
					if (node->args.data[i]->type==GLSL_AST_NODE_TYPE_VAR_CONST){
						goto _continue_const;
					}
					glsl_error_t error=_visit_node(node->args.data[i],state,&tmp);
					if (error!=GLSL_NO_ERROR){
						return error;
					}
_continue_const:
					tmp.offset+=glsl_builtin_type_to_size(node->args.data[i]->value_type->builtin_type);
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
				_calculate_output_target(state,NULL,GLSL_BUILTIN_TYPE_FLOAT,&tmp);
				_generate_dot_product_3d(state,regs,regs+1,&tmp);
				_generate_move(state,&tmp,output_register);
				regs->offset+=4;
				output_register->offset++;
				_generate_dot_product_3d(state,regs,regs+1,&tmp);
				_generate_move(state,&tmp,output_register);
				regs->offset+=4;
				output_register->offset++;
				_generate_dot_product_3d(state,regs,regs+1,&tmp);
				_generate_move(state,&tmp,output_register);
				output_register->offset-=2;
				_release_local_regs(state,&tmp);
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
	for (u32 i=0;i<node->args.count;i++){
		_release_local_regs(state,regs+i);
	}
	return GLSL_NO_ERROR;
}



static const glsl_ast_var_t* _allocate_vars(const glsl_ast_t* ast,compiler_state_t* state,glsl_error_t* error){
	const glsl_ast_var_t* ret=NULL;
	for (u32 i=0;i<ast->var_count;i++){
		glsl_ast_var_t* var=ast->vars[i];
		if (var->storage.type!=GLSL_AST_VAR_STORAGE_TYPE_DEFAULT&&var->storage.type!=GLSL_AST_VAR_STORAGE_TYPE_CONST&&_is_var_used(var)){
			glsl_compilation_output_var_type_t type=_glsl_ast_storage_type_to_output_var_type[var->storage.type];
			if (var->flags&GLSL_AST_VAR_FLAG_BUILTIN){
				if (!sys_string_compare(var->name,"gl_Position")){
					type=GLSL_COMPILATION_OUTPUT_VAR_TYPE_BUILTIN_POSITION;
				}
				else{
					sys_io_print("ERROR: Unknown builtin variale '%s'\n",var->name);
					*error=_glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
					return NULL;
				}
			}
			var->_compiler_data=_push_var(state->output,var->name,((var->storage.flags&GLSL_AST_VAR_STORAGE_FLAG_HAS_LAYOUT_LOCATION)?var->storage.layout_location:0xffffffff),type,glsl_ast_type_get_slot_count(var->type));
		}
		else if (var->type->type==GLSL_AST_TYPE_TYPE_FUNC&&!sys_string_compare(var->name,"main")){
			ret=var;
		}
		else if (var->storage.type==GLSL_AST_VAR_STORAGE_TYPE_DEFAULT&&_is_var_used(var)){
			u32 slot;
			_glsl_interface_allocator_reserve(&(state->local_variable_allocator),&slot,glsl_ast_type_get_slot_count(var->type),1);
			var->_compiler_data=slot;
		}
	}
	return ret;
}



SYS_PUBLIC glsl_error_t glsl_compiler_compile(const glsl_ast_t* ast,glsl_compilation_output_t* out){
	out->shader_type=ast->shader_type;
	out->var_count=0;
	out->instruction_count=0;
	out->const_count=0;
	out->local_count=0;
	out->_var_capacity=0;
	out->_instruction_capacity=0;
	out->vars=NULL;
	out->instructions=NULL;
	out->consts=NULL;
	compiler_state_t state={
		out
	};
	_glsl_interface_allocator_init(4096,&(state.const_variable_allocator));
	_glsl_interface_allocator_init(1024,&(state.local_variable_allocator));
	glsl_error_t error=GLSL_NO_ERROR;
	const glsl_ast_var_t* main_function=_allocate_vars(ast,&state,&error);
	if (!main_function||!main_function->value){
		if (error==GLSL_NO_ERROR){
			sys_io_print("Main function not found (error)\n");
			error=_glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
		}
		goto _cleanup;
	}
	register_state_t tmp={
		0
	};
	error=_visit_node(main_function->value,&state,&tmp);
	if (error!=GLSL_NO_ERROR){
		goto _cleanup;
	}
_cleanup:
	_glsl_interface_allocator_deinit(&(state.const_variable_allocator));
	_glsl_interface_allocator_deinit(&(state.local_variable_allocator));
	if (error!=GLSL_NO_ERROR){
		glsl_compiler_compilation_output_delete(out);
	}
	return error;
}



SYS_PUBLIC void glsl_compiler_compilation_output_delete(glsl_compilation_output_t* output){
	for (u32 i=0;i<output->var_count;i++){
		sys_heap_dealloc(NULL,(output->vars+i)->name);
	}
	sys_heap_dealloc(NULL,output->vars);
	for (u32 i=0;i<output->instruction_count;i++){
		sys_heap_dealloc(NULL,output->instructions[i]);
	}
	sys_heap_dealloc(NULL,output->instructions);
	sys_heap_dealloc(NULL,output->consts);
	output->var_count=0;
	output->instruction_count=0;
	output->const_count=0;
	output->local_count=0;
	output->_var_capacity=0;
	output->_instruction_capacity=0;
	output->vars=NULL;
	output->instructions=NULL;
	output->consts=NULL;
}
