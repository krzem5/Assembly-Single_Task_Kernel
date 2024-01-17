#include <glsl/_internal/error.h>
#include <glsl/_internal/operator_table.h>
#include <glsl/ast.h>
#include <glsl/builtin_types.h>
#include <glsl/error.h>
#include <glsl/lexer.h>
#include <glsl/parser.h>
#include <sys/heap/heap.h>
#include <sys/io/io.h>
#include <sys/memory/memory.h>
#include <sys/string/string.h>
#include <sys/types.h>



#define PARSER_EXPRESSION_STACK_SIZE 256



typedef struct _OPERATOR_STACK_ELEMENT{
	const glsl_operator_t* operator;
	_Bool is_binary;
	u32 precedence;
} operator_stack_element_t;



static _Bool _calculate_swizzle_type(glsl_ast_node_t* node,u32 size,glsl_error_t* error){
	const char* swizzle=node->member_access.member;
	u32 length=sys_string_length(swizzle);
	if (length>4){
		*error=_glsl_error_create_parser_swizzle_too_long(swizzle);
		return 0;
	}
	const char* swizzle_values=NULL;
	if (swizzle[0]=='x'||swizzle[0]=='y'||swizzle[0]=='z'||swizzle[0]=='w'){
		swizzle_values="xyzw";
	}
	else if (swizzle[0]=='r'||swizzle[0]=='g'||swizzle[0]=='b'||swizzle[0]=='a'){
		swizzle_values="rgba";
	}
	else if (swizzle[0]=='s'||swizzle[0]=='t'||swizzle[0]=='p'||swizzle[0]=='q'){
		swizzle_values="stpq";
	}
	else{
		*error=_glsl_error_create_parser_invalid_swizzle_character(swizzle,swizzle[0]);
		return 0;
	}
	glsl_ast_swizzle_t pattern=0;
	for (;swizzle[0];swizzle++){
		u32 i=0;
		for (;i<4&&swizzle_values[i]!=swizzle[0];i++);
		if (i==4){
			*error=_glsl_error_create_parser_invalid_swizzle_character(node->member_access.member,swizzle[0]);
			return 0;
		}
		pattern|=i<<((swizzle-node->member_access.member)<<1);
	}
	node->type=GLSL_AST_NODE_TYPE_SWIZZLE;
	node->value_type=glsl_ast_type_create(GLSL_AST_TYPE_TYPE_BUILTIN);
	node->value_type->builtin_type=glsl_builtin_type_from_base_type_and_length(glsl_builtin_type_to_vector_base_type(node->member_access.value->value_type->builtin_type),length);
	sys_heap_dealloc(NULL,node->member_access.member);
	node->swizzle.value=node->member_access.value;
	node->swizzle.pattern=pattern;
	node->swizzle.pattern_length=length;
	return 1;
}



static _Bool _calculate_member_type(glsl_ast_node_t* node,glsl_error_t* error){
	if (node->member_access.value->value_type->type==GLSL_AST_TYPE_TYPE_FUNC){
		goto _error;
	}
	if (node->member_access.value->value_type->type==GLSL_AST_TYPE_TYPE_STRUCT){
		*error=_glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
		return NULL;
	}
	u32 vector_length=glsl_builtin_type_to_vector_length(node->member_access.value->value_type->builtin_type);
	if (vector_length){
		return _calculate_swizzle_type(node,vector_length,error);
	}
_error:
	*error=_glsl_error_create_parser_member_not_found(node->member_access.value->value_type,node->member_access.member);
	return NULL;
}



static glsl_error_t _check_constructor_type(glsl_ast_node_t* node){
	if (!node->constructor.arg_count||node->value_type->type==GLSL_AST_TYPE_TYPE_FUNC){
		return _glsl_error_create_parser_function_constructor();
	}
	if (node->constructor.arg_count==1&&glsl_ast_type_is_equal(node->value_type,node->constructor.args[0]->value_type)){
		return GLSL_NO_ERROR;
	}
	if (node->value_type->type==GLSL_AST_TYPE_TYPE_STRUCT){
		return _glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
	}
	if (node->value_type->array_length!=GLSL_AST_TYPE_NO_ARRAY){
		return _glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
	}
	u32 remaining_length=glsl_builtin_type_to_size(node->value_type->builtin_type);
	for (u32 i=0;i<node->constructor.arg_count;i++){
		if (!remaining_length){
			return _glsl_error_create_parser_constructor_too_long();
		}
		if (node->constructor.args[i]->value_type->type!=GLSL_AST_TYPE_TYPE_BUILTIN){
			return _glsl_error_create_parser_invalid_constructor(node);
		}
		u32 length=glsl_builtin_type_to_size(node->constructor.args[i]->value_type->builtin_type);
		remaining_length=(length>remaining_length?0:remaining_length-length);
	}
	return GLSL_NO_ERROR;
}



static glsl_ast_type_t* _parse_type(glsl_parser_state_t* parser,_Bool allow_array,glsl_error_t* error){
	if (parser->tokens[parser->index].type==GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE){
		glsl_ast_type_t* out=glsl_ast_type_create(GLSL_AST_TYPE_TYPE_BUILTIN);
		out->builtin_type=parser->tokens[parser->index].builtin_type;
		parser->index++;
		return out;
	}
	if (parser->tokens[parser->index].type==GLSL_LEXER_TOKEN_TYPE_IDENTIFIER){
		// check if type is a struct
	}
	if (parser->tokens[parser->index].type!=GLSL_LEXER_TOKEN_TYPE_STRUCT){
		return NULL;
	}
	*error=_glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
	return NULL;
}



static glsl_error_t _process_operator_stack(operator_stack_element_t* operator_stack,u32* operator_stack_size,glsl_ast_node_t** value_stack,u32* value_stack_size,u32 precedence){
	while (*operator_stack_size&&(operator_stack+(*operator_stack_size)-1)->precedence<precedence){
		(*operator_stack_size)--;
		const operator_stack_element_t* operator=operator_stack+(*operator_stack_size);
		if (*value_stack_size<1+operator->is_binary){
			return _glsl_error_create_parser_expected("expression");
		}
		glsl_error_t error=GLSL_NO_ERROR;
		glsl_ast_node_t* node=NULL;
		if (operator->is_binary){
			if (value_stack[(*value_stack_size)-2]->value_type->type!=GLSL_AST_TYPE_TYPE_BUILTIN||value_stack[(*value_stack_size)-1]->value_type->type!=GLSL_AST_TYPE_TYPE_BUILTIN){
_binary_error:
				return _glsl_error_create_parser_invalid_operator_types(operator->operator->name,value_stack[(*value_stack_size)-2]->value_type,value_stack[(*value_stack_size)-1]->value_type);
			}
			node=operator->operator->binary(value_stack[(*value_stack_size)-2],value_stack[(*value_stack_size)-1],&error);
		}
		else{
			node=operator->operator->unary(value_stack[(*value_stack_size)-1],&error);
		}
		if (!node){
			if (error==GLSL_NO_ERROR){
				goto _binary_error;
			}
			return error;
		}
		if (operator->is_binary){
			(*value_stack_size)--;
		}
		value_stack[(*value_stack_size)-1]=node;
	}
	return GLSL_NO_ERROR;
}



static glsl_ast_node_t* _parse_expression(glsl_parser_state_t* parser,u32 end_glsl_lexer_token_type,_Bool end_on_comma,glsl_error_t* error){
	operator_stack_element_t operator_stack[PARSER_EXPRESSION_STACK_SIZE];
	u32 operator_stack_size=0;
	glsl_ast_node_t* value_stack[PARSER_EXPRESSION_STACK_SIZE];
	u32 value_stack_size=0;
	_Bool last_token_was_value=0;
	while (1){
		if (parser->index==parser->length){
			*error=_glsl_error_create_parser_expected((last_token_was_value?"operator":"expression"));
			goto _cleanup;
		}
		if (parser->tokens[parser->index].type==end_glsl_lexer_token_type||(end_on_comma&&parser->tokens[parser->index].type==GLSL_LEXER_TOKEN_TYPE_COMMA)){
			parser->index++;
			break;
		}
		if (parser->tokens[parser->index].type==GLSL_LEXER_TOKEN_TYPE_LEFT_PAREN){
			parser->index++;
			glsl_ast_node_t* value=_parse_expression(parser,GLSL_LEXER_TOKEN_TYPE_RIGHT_PAREN,0,error);
			if (!value){
				goto _cleanup;
			}
			if (value_stack_size==PARSER_EXPRESSION_STACK_SIZE){
				*error=_glsl_error_create_parser_expression_stack_overflow();
				goto _cleanup;
			}
			value_stack[value_stack_size]=value;
			value_stack_size++;
			continue;
		}
		glsl_ast_type_t* constructor_type=_parse_type(parser,1,error);
		if (!constructor_type&&*error!=GLSL_NO_ERROR){
			goto _cleanup;
		}
		if (constructor_type){
			if (parser->index==parser->length||parser->tokens[parser->index].type!=GLSL_LEXER_TOKEN_TYPE_LEFT_PAREN){
				*error=_glsl_error_create_parser_expected("constructor arguments");
				goto _cleanup;
			}
			parser->index++;
			if (value_stack_size==PARSER_EXPRESSION_STACK_SIZE){
				*error=_glsl_error_create_parser_expression_stack_overflow();
				goto _cleanup;
			}
			glsl_ast_node_t* node=glsl_ast_node_create(GLSL_AST_NODE_TYPE_CONSTRUCTOR);
			value_stack[value_stack_size]=node;
			value_stack_size++;
			node->value_type=constructor_type;
			node->constructor.args=NULL;
			node->constructor.arg_count=0;
			do{
				glsl_ast_node_t* arg=_parse_expression(parser,GLSL_LEXER_TOKEN_TYPE_RIGHT_PAREN,1,error);
				if (!arg){
					goto _cleanup;
				}
				node->constructor.arg_count++;
				node->constructor.args=sys_heap_realloc(NULL,node->constructor.args,node->constructor.arg_count*sizeof(glsl_ast_node_t*));
				node->constructor.args[node->constructor.arg_count-1]=arg;
			} while (parser->tokens[parser->index-1].type!=GLSL_LEXER_TOKEN_TYPE_RIGHT_PAREN);
			*error=_check_constructor_type(node);
			if (*error!=GLSL_NO_ERROR){
				goto _cleanup;
			}
			last_token_was_value=1;
			continue;
		}
		if (parser->tokens[parser->index].type==GLSL_LEXER_TOKEN_TYPE_IDENTIFIER){
			const char* identifier=parser->tokens[parser->index].string;
			parser->index++;
			if (parser->index==parser->length){
				*error=_glsl_error_create_parser_expected("operator");
				goto _cleanup;
			}
			if (parser->tokens[parser->index].type==GLSL_LEXER_TOKEN_TYPE_LEFT_PAREN){
				*error=_glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
				goto _cleanup;
			}
			if (value_stack_size==PARSER_EXPRESSION_STACK_SIZE){
				*error=_glsl_error_create_parser_expression_stack_overflow();
				goto _cleanup;
			}
			glsl_ast_var_t* var=glsl_ast_lookup_var(parser->ast,identifier,1,NULL);
			if (!var){
				*error=_glsl_error_create_parser_undefined(identifier);
				goto _cleanup;
			}
			value_stack[value_stack_size]=glsl_ast_node_create(GLSL_AST_NODE_TYPE_VAR);
			value_stack[value_stack_size]->value_type=glsl_ast_type_duplicate(var->type);
			value_stack[value_stack_size]->var=var;
			value_stack_size++;
			last_token_was_value=1;
			continue;
		}
		if (parser->tokens[parser->index].type==GLSL_LEXER_TOKEN_TYPE_CONST_BOOL){
			_Bool value=parser->tokens[parser->index].bool_;
			parser->index++;
			if (value_stack_size==PARSER_EXPRESSION_STACK_SIZE){
				*error=_glsl_error_create_parser_expression_stack_overflow();
				goto _cleanup;
			}
			value_stack[value_stack_size]=glsl_ast_node_create(GLSL_AST_NODE_TYPE_VAR_BOOL);
			value_stack[value_stack_size]->value_type=glsl_ast_type_create(GLSL_AST_TYPE_TYPE_BUILTIN);
			value_stack[value_stack_size]->value_type->builtin_type=GLSL_BUILTIN_TYPE_BOOL;
			value_stack[value_stack_size]->var_bool=value;
			value_stack_size++;
			last_token_was_value=1;
			continue;
		}
		if (parser->tokens[parser->index].type==GLSL_LEXER_TOKEN_TYPE_CONST_INT){
			u64 value=parser->tokens[parser->index].int_;
			parser->index++;
			if (value_stack_size==PARSER_EXPRESSION_STACK_SIZE){
				*error=_glsl_error_create_parser_expression_stack_overflow();
				goto _cleanup;
			}
			value_stack[value_stack_size]=glsl_ast_node_create(GLSL_AST_NODE_TYPE_VAR_INT);
			value_stack[value_stack_size]->value_type=glsl_ast_type_create(GLSL_AST_TYPE_TYPE_BUILTIN);
			value_stack[value_stack_size]->value_type->builtin_type=GLSL_BUILTIN_TYPE_INT;
			value_stack[value_stack_size]->var_int=value;
			value_stack_size++;
			last_token_was_value=1;
			continue;
		}
		if (parser->tokens[parser->index].type==GLSL_LEXER_TOKEN_TYPE_CONST_FLOAT){
			double value=parser->tokens[parser->index].float_;
			parser->index++;
			if (value_stack_size==PARSER_EXPRESSION_STACK_SIZE){
				*error=_glsl_error_create_parser_expression_stack_overflow();
				goto _cleanup;
			}
			value_stack[value_stack_size]=glsl_ast_node_create(GLSL_AST_NODE_TYPE_VAR_FLOAT);
			value_stack[value_stack_size]->value_type=glsl_ast_type_create(GLSL_AST_TYPE_TYPE_BUILTIN);
			value_stack[value_stack_size]->value_type->builtin_type=GLSL_BUILTIN_TYPE_FLOAT;
			value_stack[value_stack_size]->var_float=value;
			value_stack_size++;
			last_token_was_value=1;
			continue;
		}
		if (parser->tokens[parser->index].type==GLSL_LEXER_TOKEN_TYPE_PERIOD){
			if (!last_token_was_value){
				*error=_glsl_error_create_parser_wrong_application(".","expression");
				goto _cleanup;
			}
			parser->index++;
			if (parser->index==parser->length||parser->tokens[parser->index].type!=GLSL_LEXER_TOKEN_TYPE_IDENTIFIER){
				*error=_glsl_error_create_parser_expected("member field");
				goto _cleanup;
			}
			glsl_ast_node_t* node=glsl_ast_node_create(GLSL_AST_NODE_TYPE_MEMBER_ACCESS);
			node->member_access.value=value_stack[value_stack_size-1];
			node->member_access.member=sys_string_duplicate(parser->tokens[parser->index].string);
			value_stack[value_stack_size-1]=node;
			if (!_calculate_member_type(value_stack[value_stack_size-1],error)){
				goto _cleanup;
			}
			parser->index++;
			continue;
		}
		const glsl_operator_t* operator=_glsl_operator_table+parser->tokens[parser->index].type;
		if (!operator->name){
			*error=_glsl_error_create_parser_expected("operator");
			goto _cleanup;
		}
		if (last_token_was_value&&!operator->binary){
			*error=_glsl_error_create_parser_wrong_application(operator->name,"binary expression");
			goto _cleanup;
		}
		else if (!last_token_was_value&&!operator->unary){
			*error=_glsl_error_create_parser_wrong_application(operator->name,"unary expression");
			goto _cleanup;
		}
		parser->index++;
		u32 precedence=(last_token_was_value?operator->binary_precedence:operator->unary_precedence);
		*error=_process_operator_stack(operator_stack,&operator_stack_size,value_stack,&value_stack_size,precedence);
		if (*error!=GLSL_NO_ERROR){
			goto _cleanup;
		}
		if (operator_stack_size==PARSER_EXPRESSION_STACK_SIZE){
			*error=_glsl_error_create_parser_expression_stack_overflow();
			goto _cleanup;
		}
		(operator_stack+operator_stack_size)->operator=operator;
		(operator_stack+operator_stack_size)->is_binary=last_token_was_value;
		(operator_stack+operator_stack_size)->precedence=precedence;
		operator_stack_size++;
		last_token_was_value=0;
	}
	*error=_process_operator_stack(operator_stack,&operator_stack_size,value_stack,&value_stack_size,0xffffffff);
	if (*error!=GLSL_NO_ERROR){
		goto _cleanup;
	}
	if (value_stack_size!=1||operator_stack_size){
		*error=_glsl_error_create_parser_expected("expression");
		goto _cleanup;
	}
	return value_stack[0];
_cleanup:
	for (u32 i=0;i<value_stack_size;i++){
		glsl_ast_node_delete(value_stack[i]);
	}
	return NULL;
}



static glsl_ast_node_t* _parse_statements(glsl_parser_state_t* parser,u32 end_glsl_lexer_token_type,glsl_ast_scope_t scope,glsl_error_t* error){
	glsl_ast_node_t* out=glsl_ast_node_create(GLSL_AST_NODE_TYPE_BLOCK);
	out->block.scope=scope;
	out->block.data=NULL;
	out->block.length=0;
	while (1){
		if (parser->index==parser->length){
			*error=_glsl_error_create_parser_expected("statement");
			goto _cleanup;
		}
		if (parser->tokens[parser->index].type==end_glsl_lexer_token_type){
			parser->index++;
			return out;
		}
		if (parser->tokens[parser->index].type==GLSL_LEXER_TOKEN_TYPE_RETURN){
			*error=_glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
			goto _cleanup;
		}
		if (parser->tokens[parser->index].type==GLSL_LEXER_TOKEN_TYPE_DISCARD){
			*error=_glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
			goto _cleanup;
		}
		if (parser->tokens[parser->index].type==GLSL_LEXER_TOKEN_TYPE_CONTINUE){
			*error=_glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
			goto _cleanup;
		}
		if (parser->tokens[parser->index].type==GLSL_LEXER_TOKEN_TYPE_BREAK){
			*error=_glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
			goto _cleanup;
		}
		if (parser->tokens[parser->index].type==GLSL_LEXER_TOKEN_TYPE_IF){
			*error=_glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
			goto _cleanup;
		}
		if (parser->tokens[parser->index].type==GLSL_LEXER_TOKEN_TYPE_FOR){
			*error=_glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
			goto _cleanup;
		}
		if (parser->tokens[parser->index].type==GLSL_LEXER_TOKEN_TYPE_SWITCH){
			*error=_glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
			goto _cleanup;
		}
		if (parser->tokens[parser->index].type==GLSL_LEXER_TOKEN_TYPE_DO){
			*error=_glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
			goto _cleanup;
		}
		if (parser->tokens[parser->index].type==GLSL_LEXER_TOKEN_TYPE_WHILE){
			*error=_glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
			goto _cleanup;
		}
		glsl_ast_type_t* base_type=_parse_type(parser,0,error);
		if (!base_type&&*error!=GLSL_NO_ERROR){
			goto _cleanup;
		}
		if (!base_type){
			glsl_ast_node_t* expression=_parse_expression(parser,GLSL_LEXER_TOKEN_TYPE_SEMICOLON,0,error);
			if (!expression){
				goto _cleanup;
			}
			out->block.length++;
			out->block.data=sys_heap_realloc(NULL,out->block.data,out->block.length*sizeof(glsl_ast_node_t*));
			out->block.data[out->block.length-1]=expression;
			continue;
		}
_read_next_type:
		if (parser->index==parser->length||parser->tokens[parser->index].type!=GLSL_LEXER_TOKEN_TYPE_IDENTIFIER){
			*error=_glsl_error_create_parser_expected("variable name");
			goto _cleanup;
		}
		const char* identifier=parser->tokens[parser->index].string;
		if (glsl_ast_lookup_var(parser->ast,identifier,1,NULL)){
			*error=_glsl_error_create_parser_already_defined(identifier);
			goto _cleanup;
		}
		glsl_ast_create_var(parser->ast,identifier,1,base_type,NULL);
		glsl_ast_node_t* expression=_parse_expression(parser,GLSL_LEXER_TOKEN_TYPE_SEMICOLON,1,error);
		if (!expression){
			goto _cleanup;
		}
		out->block.length++;
		out->block.data=sys_heap_realloc(NULL,out->block.data,out->block.length*sizeof(glsl_ast_node_t*));
		out->block.data[out->block.length-1]=expression;
		if (parser->tokens[parser->index-1].type==GLSL_LEXER_TOKEN_TYPE_COMMA){
			goto _read_next_type;
		}
	}
_cleanup:
	glsl_ast_node_delete(out);
	return NULL;
}



static glsl_error_t _parse_storage(glsl_parser_state_t* parser,glsl_ast_var_storage_t* out){
	out->type=GLSL_AST_VAR_STORAGE_TYPE_DEFAULT;
	out->flags=0;
	if (parser->tokens[parser->index].type==GLSL_LEXER_TOKEN_TYPE_LAYOUT){
		parser->index++;
		if (parser->index==parser->length||parser->tokens[parser->index].type!=GLSL_LEXER_TOKEN_TYPE_LEFT_PAREN){
			return _glsl_error_create_parser_expected("layout qualifiers");
		}
		parser->index++;
_parse_next_layout_qualifier:
		if (parser->index==parser->length||parser->tokens[parser->index].type!=GLSL_LEXER_TOKEN_TYPE_IDENTIFIER){
			return _glsl_error_create_parser_expected("layout qualifier");
		}
		const char* layout_qualifier_key=parser->tokens[parser->index].string;
		u64 layout_qualifier_value=0;
		parser->index++;
		if (parser->index==parser->length){
			return _glsl_error_create_parser_expected("end of layout qualifiers");
		}
		if (parser->tokens[parser->index].type==GLSL_LEXER_TOKEN_TYPE_EQUAL){
			parser->index++;
			if (parser->index==parser->length||parser->tokens[parser->index].type!=GLSL_LEXER_TOKEN_TYPE_CONST_INT){
				return _glsl_error_create_parser_expected("integer constant");
			}
			layout_qualifier_value=parser->tokens[parser->index].int_;
			parser->index++;
			if (parser->index==parser->length){
				return _glsl_error_create_parser_expected("end of layout qualifiers");
			}
		}
		if (!sys_string_compare(layout_qualifier_key,"location")){
			if (out->flags&GLSL_AST_VAR_STORAGE_FLAG_HAS_LAYOUT_LOCATION){
				return _glsl_error_create_parser_duplicated_layout_qualifier(layout_qualifier_key);
			}
			out->flags|=GLSL_AST_VAR_STORAGE_FLAG_HAS_LAYOUT_LOCATION;
			out->layout_location=layout_qualifier_value;
		}
		else{
			return _glsl_error_create_parser_unknown_layout_qualifier(layout_qualifier_key);
		}
		if (parser->tokens[parser->index].type==GLSL_LEXER_TOKEN_TYPE_COMMA){
			parser->index++;
			goto _parse_next_layout_qualifier;
		}
		if (parser->tokens[parser->index].type!=GLSL_LEXER_TOKEN_TYPE_RIGHT_PAREN){
			return _glsl_error_create_parser_expected("end of layout qualifiers");
		}
		parser->index++;
		if (parser->index==parser->length){
			return _glsl_error_create_parser_expected("layout qualifier");
		}
	}
	if (parser->tokens[parser->index].type==GLSL_LEXER_TOKEN_TYPE_CENTROID){
		out->flags|=GLSL_AST_VAR_STORAGE_FLAG_CENTROID;
		parser->index++;
	}
	if (parser->index<=parser->length&&(parser->tokens[parser->index].type==GLSL_LEXER_TOKEN_TYPE_ATTRIBUTE||parser->tokens[parser->index].type==GLSL_LEXER_TOKEN_TYPE_CONST||parser->tokens[parser->index].type==GLSL_LEXER_TOKEN_TYPE_VARYING||parser->tokens[parser->index].type==GLSL_LEXER_TOKEN_TYPE_IN||parser->tokens[parser->index].type==GLSL_LEXER_TOKEN_TYPE_OUT||parser->tokens[parser->index].type==GLSL_LEXER_TOKEN_TYPE_UNIFORM)){
		switch (parser->tokens[parser->index].type){
			case GLSL_LEXER_TOKEN_TYPE_ATTRIBUTE:
				return _glsl_error_create_parser_deprecated_keyword("attribute");
			case GLSL_LEXER_TOKEN_TYPE_CONST:
				out->type=GLSL_AST_VAR_STORAGE_TYPE_CONST;
				break;
			case GLSL_LEXER_TOKEN_TYPE_IN:
				out->type=GLSL_AST_VAR_STORAGE_TYPE_IN;
				break;
			case GLSL_LEXER_TOKEN_TYPE_INOUT:
				out->type=GLSL_AST_VAR_STORAGE_TYPE_INOUT;
				break;
			case GLSL_LEXER_TOKEN_TYPE_OUT:
				out->type=GLSL_AST_VAR_STORAGE_TYPE_OUT;
				break;
			case GLSL_LEXER_TOKEN_TYPE_UNIFORM:
				out->type=GLSL_AST_VAR_STORAGE_TYPE_UNIFORM;
				break;
			case GLSL_LEXER_TOKEN_TYPE_VARYING:
				return _glsl_error_create_parser_deprecated_keyword("varying");
		}
		parser->index++;
	}
	return GLSL_NO_ERROR;
}



SYS_PUBLIC glsl_error_t glsl_parser_parse_tokens(const glsl_lexer_token_list_t* token_list,glsl_linker_program_t* program,glsl_shader_type_t shader_type){
	if (program->shader_bitmap&(1<<shader_type)){
		return _glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
	}
	glsl_error_t error=GLSL_NO_ERROR;
	glsl_ast_t* ast=program->shaders+shader_type;
	ast->block_count=0;
	ast->named_type_count=0;
	ast->var_count=0;
	ast->blocks=NULL;
	ast->named_types=NULL;
	ast->vars=NULL;
	glsl_parser_state_t parser={
		shader_type,
		token_list->data,
		0,
		token_list->length,
		ast
	};
	while (parser.index<parser.length){
		if (parser.tokens[parser.index].type==GLSL_LEXER_TOKEN_TYPE_PRECISION){
			parser.index++;
			if (parser.index==parser.length||(parser.tokens[parser.index].type!=GLSL_LEXER_TOKEN_TYPE_LOWP&&parser.tokens[parser.index].type!=GLSL_LEXER_TOKEN_TYPE_MEDIUMP&&parser.tokens[parser.index].type!=GLSL_LEXER_TOKEN_TYPE_HIGHP)){
				error=_glsl_error_create_parser_expected("precision qualifier");
				goto _cleanup;
			}
			u32 precision=parser.tokens[parser.index].type;
			parser.index++;
			if (parser.index==parser.length||parser.tokens[parser.index].type!=GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE){
				error=_glsl_error_create_parser_expected("type");
				goto _cleanup;
			}
			sys_io_print("Precision: %u -> %u\n",parser.tokens[parser.index].builtin_type,precision);
			parser.index++;
			if (parser.index==parser.length||parser.tokens[parser.index].type!=GLSL_LEXER_TOKEN_TYPE_SEMICOLON){
				error=_glsl_error_create_parser_expected("semicolon");
				goto _cleanup;
			}
			parser.index++;
			continue;
		}
		glsl_ast_var_storage_t storage;
		error=_parse_storage(&parser,&storage);
		if (error){
			goto _cleanup;
		}
		if (parser.index==parser.length){
			error=_glsl_error_create_parser_expected("type");
			goto _cleanup;
		}
		glsl_ast_type_t* type=_parse_type(&parser,0,&error);
		if (!type&&error!=GLSL_NO_ERROR){
			goto _cleanup;
		}
		if (!type){
			if (parser.index+1>=parser.length||parser.tokens[parser.index].type!=GLSL_LEXER_TOKEN_TYPE_IDENTIFIER||parser.tokens[parser.index+1].type!=GLSL_LEXER_TOKEN_TYPE_LEFT_BRACE){
				error=_glsl_error_create_parser_expected("type");
				goto _cleanup;
			}
			const char* block_name=parser.tokens[parser.index].string;
			parser.index+=2;
			glsl_ast_block_t* block=sys_heap_alloc(NULL,sizeof(glsl_ast_block_t));
			block->name=sys_string_duplicate(block_name);
			block->storage=storage;
			ast->block_count++;
			ast->blocks=sys_heap_realloc(NULL,ast->blocks,ast->block_count*sizeof(glsl_ast_block_t*));
			ast->blocks[ast->block_count-1]=block;
			while (1){
				if (parser.index==parser.length){
					error=_glsl_error_create_parser_expected("block member type");
					goto _cleanup;
				}
				if (parser.tokens[parser.index].type==GLSL_LEXER_TOKEN_TYPE_RIGHT_BRACE){
					break;
				}
				error=_parse_storage(&parser,&storage);
				if (error){
					goto _cleanup;
				}
				storage.flags|=GLSL_AST_VAR_STORAGE_FLAG_BLOCK;
				storage.block=block;
				if (parser.index==parser.length){
					error=_glsl_error_create_parser_expected("block member type");
					goto _cleanup;
				}
				type=_parse_type(&parser,0,&error);
				if (!type&&error!=GLSL_NO_ERROR){
					goto _cleanup;
				}
				if (!type){
					error=_glsl_error_create_parser_expected("block member type");
					goto _cleanup;
				}
				if (parser.index==parser.length||parser.tokens[parser.index].type!=GLSL_LEXER_TOKEN_TYPE_IDENTIFIER){
					error=_glsl_error_create_parser_expected("block member name");
					goto _cleanup;
				}
				const char* identifier=parser.tokens[parser.index].string;
				if (glsl_ast_lookup_var(ast,identifier,0,NULL)){
					error=_glsl_error_create_parser_already_defined(identifier);
					goto _cleanup;
				}
				parser.index++;
				if (parser.index==parser.length){
					error=_glsl_error_create_parser_expected("semicolon");
					goto _cleanup;
				}
				if (parser.tokens[parser.index].type==GLSL_LEXER_TOKEN_TYPE_LEFT_BRACKET){
					parser.index++;
					if (parser.index==parser.length){
						error=_glsl_error_create_parser_expected("right bracket");
						goto _cleanup;
					}
					if (parser.tokens[parser.index].type==GLSL_LEXER_TOKEN_TYPE_RIGHT_BRACKET){
						type->array_length=0;
					}
					else{
						error=_glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
						goto _cleanup;
					}
					parser.index++;
					if (parser.index==parser.length){
						error=_glsl_error_create_parser_expected("semicolon");
						goto _cleanup;
					}
				}
				if (parser.tokens[parser.index].type!=GLSL_LEXER_TOKEN_TYPE_SEMICOLON){
					error=_glsl_error_create_parser_expected("semicolon");
					goto _cleanup;
				}
				parser.index++;
				glsl_ast_create_var(ast,identifier,0,type,&storage);
			}
			parser.index++;
			if (parser.index==parser.length){
				error=_glsl_error_create_parser_expected("semicolon");
				goto _cleanup;
			}
			if (parser.tokens[parser.index].type==GLSL_LEXER_TOKEN_TYPE_SEMICOLON){
				parser.index++;
				goto _skip_block_instance_name;
			}
			error=_glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
			goto _cleanup;
_skip_block_instance_name:
			continue;
		}
		if (parser.index==parser.length||parser.tokens[parser.index].type!=GLSL_LEXER_TOKEN_TYPE_IDENTIFIER){
			error=_glsl_error_create_parser_expected("variable or function name");
			goto _cleanup;
		}
		const char* identifier=parser.tokens[parser.index].string;
		parser.index++;
		if (parser.index==parser.length){
			error=_glsl_error_create_parser_expected("semicolon");
			goto _cleanup;
		}
		if (parser.tokens[parser.index].type==GLSL_LEXER_TOKEN_TYPE_LEFT_BRACKET){
			parser.index++;
			if (parser.index==parser.length){
				error=_glsl_error_create_parser_expected("right bracket");
				goto _cleanup;
			}
			if (parser.tokens[parser.index].type==GLSL_LEXER_TOKEN_TYPE_RIGHT_BRACKET){
				parser.index++;
				type->array_length=0;
			}
			else{
				error=_glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
				goto _cleanup;
			}
			if (parser.index==parser.length){
				error=_glsl_error_create_parser_expected("semicolon");
				goto _cleanup;
			}
		}
		if (parser.tokens[parser.index].type==GLSL_LEXER_TOKEN_TYPE_SEMICOLON){
			if (glsl_ast_lookup_var(ast,identifier,0,NULL)){
				error=_glsl_error_create_parser_already_defined(identifier);
				goto _cleanup;
			}
			parser.index++;
			glsl_ast_create_var(ast,identifier,0,type,&storage);
			continue;
		}
		if (parser.tokens[parser.index].type==GLSL_LEXER_TOKEN_TYPE_EQUAL){
			error=_glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
			goto _cleanup;
		}
		if (parser.tokens[parser.index].type!=GLSL_LEXER_TOKEN_TYPE_LEFT_PAREN){
			error=_glsl_error_create_parser_expected("argument list");
			goto _cleanup;
		}
		if (storage.type!=GLSL_AST_VAR_STORAGE_TYPE_DEFAULT||storage.flags){
			error=_glsl_error_create_parser_invalid_storage_type("function",&storage);
			goto _cleanup;
		}
		parser.index++;
		if (parser.index==parser.length){
			error=_glsl_error_create_parser_expected("argument list");
			goto _cleanup;
		}
		glsl_ast_type_t* func_type=glsl_ast_type_create(GLSL_AST_TYPE_TYPE_FUNC);
		func_type->func.return_and_args=sys_heap_alloc(NULL,sizeof(glsl_ast_type_t*));
		func_type->func.return_and_args[0]=type;
		func_type->func.arg_count=0;
		if (parser.tokens[parser.index].type==GLSL_LEXER_TOKEN_TYPE_RIGHT_PAREN){
			parser.index++;
			goto _skip_argument_list;
		}
		if (parser.tokens[parser.index].type==GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE&&parser.tokens[parser.index].builtin_type==GLSL_BUILTIN_TYPE_VOID){
			parser.index++;
			if (parser.index==parser.length||parser.tokens[parser.index].type!=GLSL_LEXER_TOKEN_TYPE_RIGHT_PAREN){
				error=_glsl_error_create_parser_expected("end of argument list");
				glsl_ast_type_delete(func_type);
				goto _cleanup;
			}
			parser.index++;
			goto _skip_argument_list;
		}
		error=_glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
		glsl_ast_type_delete(func_type);
		goto _cleanup;
_skip_argument_list:
		if (parser.index==parser.length){
			error=_glsl_error_create_parser_expected("function definition or declaration");
			glsl_ast_type_delete(func_type);
			goto _cleanup;
		}
		glsl_ast_var_t* var=glsl_ast_lookup_var(ast,identifier,0,func_type);
		if (var&&var->value){
			error=_glsl_error_create_parser_already_defined(identifier);
			glsl_ast_type_delete(func_type);
			goto _cleanup;
		}
		if (!var){
			var=glsl_ast_create_var(ast,identifier,0,func_type,NULL);
		}
		if (parser.tokens[parser.index].type==GLSL_LEXER_TOKEN_TYPE_SEMICOLON){
			parser.index++;
			continue;
		}
		if (parser.tokens[parser.index].type!=GLSL_LEXER_TOKEN_TYPE_LEFT_BRACE){
			error=_glsl_error_create_parser_expected("function definition");
			goto _cleanup;
		}
		parser.index++;
		var->value=_parse_statements(&parser,GLSL_LEXER_TOKEN_TYPE_RIGHT_BRACE,1,&error);
		if (!var->value){
			goto _cleanup;
		}
	}
	program->shader_bitmap|=1<<shader_type;
	return GLSL_NO_ERROR;
_cleanup:
	glsl_ast_delete(ast);
	return error;
}
