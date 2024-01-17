#include <glsl/ast.h>
#include <glsl/builtin_types.h>
#include <glsl/lexer.h>
#include <glsl/linker.h>
#include <sys/heap/heap.h>
#include <sys/io/io.h>
#include <sys/types.h>



static void _print_indentation(u32 indentation){
	for (;indentation;indentation--){
		sys_io_print(" ");
	}
}



static void _print_ast_node(const glsl_ast_node_t* node,u32 indentation){
	if (node->type==GLSL_AST_NODE_TYPE_BLOCK){
		if (node->block.scope){
			sys_io_print("(%u)",node->block.scope);
		}
		if (!node->block.length){
			sys_io_print("[]");
			return;
		}
		sys_io_print("[\n");
		for (u32 i=0;i<node->block.length;i++){
			_print_indentation(indentation+2);
			_print_ast_node(node->block.data[i],indentation+2);
			if (i<node->block.length-1){
				sys_io_print(",");
			}
			sys_io_print("\n");
		}
		_print_indentation(indentation);
		sys_io_print("]");
		return;
	}
	char* type_str=glsl_ast_type_to_string(node->value_type);
	switch (node->type){
		case GLSL_AST_NODE_TYPE_NONE:
			sys_io_print("<invalid ast node>");
			goto _cleanup;
		case GLSL_AST_NODE_TYPE_ARRAY_ACCESS:
			sys_io_print("GLSL_AST_NODE_TYPE_ARRAY_ACCESS");
			goto _cleanup;
		case GLSL_AST_NODE_TYPE_BIT_INVERSE:
		case GLSL_AST_NODE_TYPE_NEGATE:
		case GLSL_AST_NODE_TYPE_NOT:
			goto _unary_op;
		case GLSL_AST_NODE_TYPE_CALL:
			sys_io_print("GLSL_AST_NODE_TYPE_CALL");
			goto _cleanup;
		case GLSL_AST_NODE_TYPE_CONSTRUCTOR:
			sys_io_print("{\n");
			_print_indentation(indentation+2);
			sys_io_print("op: constructor,\n");
			_print_indentation(indentation+2);
			sys_io_print("type: %s,\n",type_str);
			_print_indentation(indentation+2);
			sys_io_print("args: [");
			for (u32 i=0;i<node->constructor.arg_count;i++){
				sys_io_print("\n");
				_print_indentation(indentation+4);
				_print_ast_node(node->constructor.args[i],indentation+4);
				if (i<node->constructor.arg_count-1){
					sys_io_print(",");
				}
			}
			if (node->constructor.arg_count){
				sys_io_print("\n");
				_print_indentation(indentation+2);
			}
			sys_io_print("]\n");
			_print_indentation(indentation);
			sys_io_print("}");
			goto _cleanup;
		case GLSL_AST_NODE_TYPE_COMMA:
			sys_io_print("GLSL_AST_NODE_TYPE_COMMA");
			goto _cleanup;
		case GLSL_AST_NODE_TYPE_MEMBER_ACCESS:
			sys_io_print("{\n");
			_print_indentation(indentation+2);
			sys_io_print("op: member_access,\n");
			_print_indentation(indentation+2);
			sys_io_print("type: %s,\n",type_str);
			_print_indentation(indentation+2);
			sys_io_print("member: %s,\n",node->member_access.member);
			_print_indentation(indentation+2);
			sys_io_print("value: ");
			_print_ast_node(node->member_access.value,indentation+2);
			sys_io_print("\n");
			_print_indentation(indentation);
			sys_io_print("}");
			goto _cleanup;
		case GLSL_AST_NODE_TYPE_VAR:
			sys_io_print("%s",node->var->name);
			goto _cleanup;
		case GLSL_AST_NODE_TYPE_VAR_BOOL:
			sys_io_print("%s",(node->var_bool?"true":"false"));
			goto _cleanup;
		case GLSL_AST_NODE_TYPE_VAR_FLOAT:
			sys_io_print("%f",node->var_float);
			goto _cleanup;
		case GLSL_AST_NODE_TYPE_VAR_INT:
			sys_io_print("%ld",node->var_int);
			goto _cleanup;
		case GLSL_AST_NODE_TYPE_SWIZZLE:
			sys_io_print("{\n");
			_print_indentation(indentation+2);
			sys_io_print("op: swizzle,\n");
			_print_indentation(indentation+2);
			sys_io_print("type: %s,\n",type_str);
			_print_indentation(indentation+2);
			sys_io_print("pattern: ");
			for (u8 i=0;i<node->swizzle.pattern_length;i++){
				sys_io_print("%c","xyzw"[(node->swizzle.pattern>>(i<<1))&3]);
			}
			sys_io_print(",\n");
			_print_indentation(indentation+2);
			sys_io_print("value: ");
			_print_ast_node(node->swizzle.value,indentation+2);
			sys_io_print("\n");
			_print_indentation(indentation);
			sys_io_print("}");
			goto _cleanup;
	}
	sys_io_print("{\n");
	_print_indentation(indentation+2);
	sys_io_print("op: %s,\n",glsl_ast_node_type_to_string(node->type));
	_print_indentation(indentation+2);
	sys_io_print("type: %s,\n",type_str);
	_print_indentation(indentation+2);
	sys_io_print("left: ");
	_print_ast_node(node->binary[0],indentation+2);
	sys_io_print(",\n");
	_print_indentation(indentation+2);
	sys_io_print("right: ");
	_print_ast_node(node->binary[1],indentation+2);
	sys_io_print("\n");
	_print_indentation(indentation);
	sys_io_print("}");
	goto _cleanup;
_unary_op:
	sys_io_print("{\n");
	_print_indentation(indentation+2);
	sys_io_print("op: %s,\n",glsl_ast_node_type_to_string(node->type));
	_print_indentation(indentation+2);
	sys_io_print("type: %s,\n",type_str);
	_print_indentation(indentation+2);
	sys_io_print("value: ");
	_print_ast_node(node->binary[1],indentation+2);
	sys_io_print("\n");
	_print_indentation(indentation);
	sys_io_print("}");
_cleanup:
	sys_heap_dealloc(NULL,type_str);
}



static void _print_ast_storage(const glsl_ast_var_storage_t* storage){
	if (storage->flags&GLSL_AST_VAR_STORAGE_FLAG_CENTROID){
		sys_io_print("centroid ");
	}
	switch (storage->type){
		case GLSL_AST_VAR_STORAGE_TYPE_DEFAULT:
			sys_io_print("default");
			break;
		case GLSL_AST_VAR_STORAGE_TYPE_CONST:
			sys_io_print("const");
			break;
		case GLSL_AST_VAR_STORAGE_TYPE_IN:
			sys_io_print("in");
			break;
		case GLSL_AST_VAR_STORAGE_TYPE_INOUT:
			sys_io_print("inout");
			break;
		case GLSL_AST_VAR_STORAGE_TYPE_OUT:
			sys_io_print("out");
			break;
		case GLSL_AST_VAR_STORAGE_TYPE_UNIFORM:
			sys_io_print("uniform");
			break;
	}
	if (storage->flags&GLSL_AST_VAR_STORAGE_FLAG_HAS_LAYOUT_LOCATION){
		sys_io_print(", layout_location=%u",storage->layout_location);
	}
	if (storage->flags&GLSL_AST_VAR_STORAGE_FLAG_BLOCK){
		sys_io_print(", block=%s",storage->block->name);
	}
}



SYS_PUBLIC void glsl_debug_print_token_list(const glsl_lexer_token_list_t* token_list){
	for (u32 i=0;i<token_list->length;i++){
		const glsl_lexer_token_t* token=token_list->data+i;
		switch (token->type){
			case GLSL_LEXER_TOKEN_TYPE_ADD:
				sys_io_print("ADD\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_ADD_ASSIGN:
				sys_io_print("ADD_ASSIGN\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_AND:
				sys_io_print("AND\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_ATTRIBUTE:
				sys_io_print("ATTRIBUTE\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_BOOL:
				sys_io_print("BOOL\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_BREAK:
				sys_io_print("BREAK\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE:
				sys_io_print("BUILTIN_TYPE (%s)\n",glsl_builtin_type_to_string(token->builtin_type));
				break;
			case GLSL_LEXER_TOKEN_TYPE_CASE:
				sys_io_print("CASE\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_CENTROID:
				sys_io_print("CENTROID\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_COLON:
				sys_io_print("COLON\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_COMMA:
				sys_io_print("COMMA\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_CONST:
				sys_io_print("CONST\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_CONST_BOOL:
				sys_io_print("CONST_BOOL (%s)\n",(token->bool_?"true":"false"));
				break;
			case GLSL_LEXER_TOKEN_TYPE_CONST_FLOAT:
				sys_io_print("CONST_FLOAT (%lf)\n",token->float_);
				break;
			case GLSL_LEXER_TOKEN_TYPE_CONST_INT:
				sys_io_print("CONST_INT (%lu)\n",token->int_);
				break;
			case GLSL_LEXER_TOKEN_TYPE_CONTINUE:
				sys_io_print("CONTINUE\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_DEC:
				sys_io_print("DEC\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_DEFAULT:
				sys_io_print("DEFAULT\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_DISCARD:
				sys_io_print("DISCARD\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_DIV:
				sys_io_print("DIV\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_DIV_ASSIGN:
				sys_io_print("DIV_ASSIGN\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_DO:
				sys_io_print("DO\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_ELSE:
				sys_io_print("ELSE\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_EQU:
				sys_io_print("EQU\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_EQUAL:
				sys_io_print("EQUAL\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_FLAT:
				sys_io_print("FLAT\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_FLOAT:
				sys_io_print("FLOAT\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_FOR:
				sys_io_print("FOR\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_GEQ:
				sys_io_print("GEQ\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_HIGHP:
				sys_io_print("HIGHP\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_IDENTIFIER:
				sys_io_print("IDENTIFIER (%s)\n",token->string);
				break;
			case GLSL_LEXER_TOKEN_TYPE_IF:
				sys_io_print("IF\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_IN:
				sys_io_print("IN\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_INC:
				sys_io_print("INC\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_INOUT:
				sys_io_print("INOUT\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_INT:
				sys_io_print("INT\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_INV:
				sys_io_print("INV\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_INVARIANT:
				sys_io_print("INVARIANT\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_LAND:
				sys_io_print("LAND\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_LAYOUT:
				sys_io_print("LAYOUT\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_LEFT_BRACE:
				sys_io_print("LEFT_BRACE\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_LEFT_BRACKET:
				sys_io_print("LEFT_BRACKET\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_LEFT_PAREN:
				sys_io_print("LEFT_PAREN\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_LEQ:
				sys_io_print("LEQ\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_LESS:
				sys_io_print("LESS\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_LOR:
				sys_io_print("LOR\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_LOWP:
				sys_io_print("LOWP\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_LSH:
				sys_io_print("LSH\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_LSH_ASSIGN:
				sys_io_print("LSH_ASSIGN\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_MEDIUMP:
				sys_io_print("MEDIUMP\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_MOD:
				sys_io_print("MOD\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_MOD_ASSIGN:
				sys_io_print("MOD_ASSIGN\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_MORE:
				sys_io_print("MORE\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_MUL:
				sys_io_print("MUL\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_MUL_ASSIGN:
				sys_io_print("MUL_ASSIGN\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_NEQ:
				sys_io_print("NEQ\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_NOPERSPECTIVE:
				sys_io_print("NOPERSPECTIVE\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_NOT:
				sys_io_print("NOT\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_OR:
				sys_io_print("OR\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_OUT:
				sys_io_print("OUT\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_PERIOD:
				sys_io_print("PERIOD\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_PRECISION:
				sys_io_print("PRECISION\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_QUESTION_MARK:
				sys_io_print("QUESTION_MARK\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_RETURN:
				sys_io_print("RETURN\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_RIGHT_BRACE:
				sys_io_print("RIGHT_BRACE\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_RIGHT_BRACKET:
				sys_io_print("RIGHT_BRACKET\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_RIGHT_PAREN:
				sys_io_print("RIGHT_PAREN\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_RSH:
				sys_io_print("RSH\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_RSH_ASSIGN:
				sys_io_print("RSH_ASSIGN\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_SEMICOLON:
				sys_io_print("SEMICOLON\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_SMOOTH:
				sys_io_print("SMOOTH\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_STRUCT:
				sys_io_print("STRUCT\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_SUB:
				sys_io_print("SUB\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_SUB_ASSIGN:
				sys_io_print("SUB_ASSIGN\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_SWITCH:
				sys_io_print("SWITCH\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_UNIFORM:
				sys_io_print("UNIFORM\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_VARYING:
				sys_io_print("VARYING\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_VOID:
				sys_io_print("VOID\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_WHILE:
				sys_io_print("WHILE\n");
				break;
			case GLSL_LEXER_TOKEN_TYPE_XOR:
				sys_io_print("XOR\n");
				break;
		}
	}
}



SYS_PUBLIC void glsl_debug_print_ast(const glsl_ast_t* ast){
	sys_io_print("{\n  blocks: [");
	for (u32 i=0;i<ast->block_count;i++){
		const glsl_ast_block_t* block=ast->blocks[i];
		sys_io_print("\n    {\n      name: '%s',\n      storage: ",block->name);
		_print_ast_storage(&(block->storage));
		sys_io_print("\n    }%s",(i==ast->block_count-1?"":","));
	}
	if (ast->block_count){
		sys_io_print("\n  ");
	}
	sys_io_print("],\n  named_types: [");
	if (ast->named_type_count){
		sys_io_print("\n  ");
	}
	sys_io_print("],\n  vars: [");
	for (u32 i=0;i<ast->var_count;i++){
		const glsl_ast_var_t* var=ast->vars[i];
		sys_io_print("\n    {\n      name: '%s',\n      usage:",var->name);
		if (var->usage_flags&GLSL_AST_VAR_USAGE_FLAG_READ){
			sys_io_print(" read");
		}
		if (var->usage_flags&GLSL_AST_VAR_USAGE_FLAG_WRITE){
			sys_io_print(" write");
		}
		if (!var->usage_flags){
			sys_io_print(" none");
		}
		sys_io_print(",\n      scope: ");
		if (!var->scope){
			sys_io_print("global");
		}
		else{
			sys_io_print("local (%u)",var->scope);
		}
		sys_io_print(",\n      storage: ");
		_print_ast_storage(&(var->storage));
		char* type_str=glsl_ast_type_to_string(var->type);
		sys_io_print(",\n      type: %s,\n      value: ",type_str);
		sys_heap_dealloc(NULL,type_str);
		if (var->value){
			_print_ast_node(var->value,8);
		}
		else{
			sys_io_print("<null>");
		}
		sys_io_print("\n    }%s",(i==ast->var_count-1?"":","));
	}
	if (ast->var_count){
		sys_io_print("\n  ");
	}
	sys_io_print("]\n}\n");
}



SYS_PUBLIC void glsl_debug_print_linked_program(const glsl_linker_linked_program_t* linked_program){
	sys_io_print("{\n  uniforms: [");
	for (u32 i=0;i<linked_program->uniform_count;i++){
		const glsl_linker_linked_program_uniform_t* uniform=linked_program->uniforms+i;
		sys_io_print("\n    {\n      name: '%s',\n      slot: %u,\n      size: %u\n    }%s",uniform->name,uniform->slot,uniform->size,(i==linked_program->uniform_count-1?"":","));
	}
	if (linked_program->uniform_count){
		sys_io_print("\n  ");
	}
	sys_io_print("]\n}\n");
}
