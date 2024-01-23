#include <glsl/ast.h>
#include <glsl/builtin_types.h>
#include <glsl/compiler.h>
#include <glsl/lexer.h>
#include <glsl/linker.h>
#include <glsl/operators.h>
#include <glsl/shader.h>
#include <sys/heap/heap.h>
#include <sys/io/io.h>
#include <sys/types.h>



#define LINKED_SHADER_BYTES_PER_LINE 16



static const char*const _glsl_instruction_type_to_string[]={
	[GLSL_INSTRUCTION_TYPE_NOP]="NOP",
	[GLSL_INSTRUCTION_TYPE_MOV]="MOV",
	[GLSL_INSTRUCTION_TYPE_ADD]="ADD",
	[GLSL_INSTRUCTION_TYPE_SUB]="SUB",
	[GLSL_INSTRUCTION_TYPE_MUL]="MUL",
	[GLSL_INSTRUCTION_TYPE_DIV]="DIV",
	[GLSL_INSTRUCTION_TYPE_MOD]="MOD",
	[GLSL_INSTRUCTION_TYPE_AND]="AND",
	[GLSL_INSTRUCTION_TYPE_IOR]="IOR",
	[GLSL_INSTRUCTION_TYPE_XOR]="XOR",
	[GLSL_INSTRUCTION_TYPE_DP2]="DP2",
	[GLSL_INSTRUCTION_TYPE_DP3]="DP3",
	[GLSL_INSTRUCTION_TYPE_DP4]="DP4",
	[GLSL_INSTRUCTION_TYPE_EXP]="EXP",
	[GLSL_INSTRUCTION_TYPE_LOG]="LOG",
	[GLSL_INSTRUCTION_TYPE_POW]="POW",
	[GLSL_INSTRUCTION_TYPE_SRT]="SRT",
	[GLSL_INSTRUCTION_TYPE_RCP]="RCP",
	[GLSL_INSTRUCTION_TYPE_MAD]="MAD",
	[GLSL_INSTRUCTION_TYPE_FLR]="FLR",
	[GLSL_INSTRUCTION_TYPE_CEL]="CEL",
	[GLSL_INSTRUCTION_TYPE_RND]="RND",
	[GLSL_INSTRUCTION_TYPE_FRC]="FRC",
	[GLSL_INSTRUCTION_TYPE_SIN]="SIN",
	[GLSL_INSTRUCTION_TYPE_COS]="COS",
	[GLSL_INSTRUCTION_TYPE_IGN]="IGN",
	[GLSL_INSTRUCTION_TYPE_EMV]="EMV",
	[GLSL_INSTRUCTION_TYPE_EMP]="EMP",
};

static const char*const _glsl_compilation_output_var_type_to_string[GLSL_COMPILATION_OUTPUT_VAR_MAX_TYPE+1]={
	[GLSL_COMPILATION_OUTPUT_VAR_TYPE_INPUT]="input",
	[GLSL_COMPILATION_OUTPUT_VAR_TYPE_OUTPUT]="output",
	[GLSL_COMPILATION_OUTPUT_VAR_TYPE_UNIFORM]="uniform",
	[GLSL_COMPILATION_OUTPUT_VAR_TYPE_BUILTIN_POSITION]="<position>",
};

static const char*const _glsl_shader_type_to_string[GLSL_SHADER_MAX_TYPE+1]={
	[GLSL_SHADER_TYPE_VERTEX]="vertex",
	[GLSL_SHADER_TYPE_FRAGMENT]="fragment",
};



static void _print_indentation(u32 indentation){
	for (;indentation;indentation--){
		sys_io_print(" ");
	}
}



static void _print_ast_node(const glsl_ast_node_t* node,u32 indentation){
	if (node->type==GLSL_AST_NODE_TYPE_BLOCK){
		if (node->block_scope){
			sys_io_print("(%u)",node->block_scope);
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
			break;
		case GLSL_AST_NODE_TYPE_ARRAY_ACCESS:
			sys_io_print("GLSL_AST_NODE_TYPE_ARRAY_ACCESS");
			break;
		case GLSL_AST_NODE_TYPE_CALL:
			sys_io_print("GLSL_AST_NODE_TYPE_CALL");
			break;
		case GLSL_AST_NODE_TYPE_CONSTRUCTOR:
			sys_io_print("{\n");
			_print_indentation(indentation+2);
			sys_io_print("op: constructor,\n");
			_print_indentation(indentation+2);
			sys_io_print("type: %s,\n",type_str);
			_print_indentation(indentation+2);
			sys_io_print("args: [");
			for (u32 i=0;i<node->args.count;i++){
				sys_io_print("\n");
				_print_indentation(indentation+4);
				_print_ast_node(node->args.data[i],indentation+4);
				if (i<node->args.count-1){
					sys_io_print(",");
				}
			}
			if (node->args.count){
				sys_io_print("\n");
				_print_indentation(indentation+2);
			}
			sys_io_print("]\n");
			_print_indentation(indentation);
			sys_io_print("}");
			break;
		case GLSL_AST_NODE_TYPE_INLINE_BLOCK:
			sys_io_print("<GLSL_AST_NODE_TYPE_INLINE_BLOCK>");
			break;
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
			break;
		case GLSL_AST_NODE_TYPE_OPERATOR:
			sys_io_print("{\n");
			_print_indentation(indentation+2);
			sys_io_print("op: %s,\n",glsl_operator_type_to_string(node->operator_type));
			_print_indentation(indentation+2);
			sys_io_print("type: %s,\n",type_str);
			_print_indentation(indentation+2);
			sys_io_print("args: [");
			for (u32 i=0;i<node->args.count;i++){
				sys_io_print("\n");
				_print_indentation(indentation+4);
				_print_ast_node(node->args.data[i],indentation+4);
				if (i<node->args.count-1){
					sys_io_print(",");
				}
			}
			if (node->args.count){
				sys_io_print("\n");
				_print_indentation(indentation+2);
			}
			sys_io_print("]\n");
			_print_indentation(indentation);
			sys_io_print("}");
			break;
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
			break;
		case GLSL_AST_NODE_TYPE_VAR:
			sys_io_print("%s",node->var->name);
			break;
		case GLSL_AST_NODE_TYPE_VAR_CONST:
			switch (node->value_type->builtin_type){
				default:
				case GLSL_BUILTIN_TYPE_INT:
					sys_io_print("%d",node->var_int);
					break;
				case GLSL_BUILTIN_TYPE_BOOL:
					sys_io_print("%s",(node->var_bool?"true":"false"));
					break;
				case GLSL_BUILTIN_TYPE_FLOAT:
					sys_io_print("%f",node->var_float);
					break;
				case GLSL_BUILTIN_TYPE_MAT22:
					sys_io_print("<GLSL_BUILTIN_TYPE_MAT22>");
					break;
				case GLSL_BUILTIN_TYPE_MAT33:
					sys_io_print("<GLSL_BUILTIN_TYPE_MAT33>");
					break;
				case GLSL_BUILTIN_TYPE_MAT44:
					sys_io_print("<GLSL_BUILTIN_TYPE_MAT44>");
					break;
				case GLSL_BUILTIN_TYPE_MAT23:
					sys_io_print("<GLSL_BUILTIN_TYPE_MAT23>");
					break;
				case GLSL_BUILTIN_TYPE_MAT24:
					sys_io_print("<GLSL_BUILTIN_TYPE_MAT24>");
					break;
				case GLSL_BUILTIN_TYPE_MAT32:
					sys_io_print("<GLSL_BUILTIN_TYPE_MAT32>");
					break;
				case GLSL_BUILTIN_TYPE_MAT34:
					sys_io_print("<GLSL_BUILTIN_TYPE_MAT34>");
					break;
				case GLSL_BUILTIN_TYPE_MAT42:
					sys_io_print("<GLSL_BUILTIN_TYPE_MAT42>");
					break;
				case GLSL_BUILTIN_TYPE_MAT43:
					sys_io_print("<GLSL_BUILTIN_TYPE_MAT43>");
					break;
				case GLSL_BUILTIN_TYPE_VEC2:
					sys_io_print("<GLSL_BUILTIN_TYPE_VEC2>");
					break;
				case GLSL_BUILTIN_TYPE_VEC3:
					sys_io_print("<GLSL_BUILTIN_TYPE_VEC3>");
					break;
				case GLSL_BUILTIN_TYPE_VEC4:
					sys_io_print("<GLSL_BUILTIN_TYPE_VEC4>");
					break;
				case GLSL_BUILTIN_TYPE_IVEC2:
					sys_io_print("<GLSL_BUILTIN_TYPE_IVEC2>");
					break;
				case GLSL_BUILTIN_TYPE_IVEC3:
					sys_io_print("<GLSL_BUILTIN_TYPE_IVEC3>");
					break;
				case GLSL_BUILTIN_TYPE_IVEC4:
					sys_io_print("<GLSL_BUILTIN_TYPE_IVEC4>");
					break;
				case GLSL_BUILTIN_TYPE_BVEC2:
					sys_io_print("<GLSL_BUILTIN_TYPE_BVEC2>");
					break;
				case GLSL_BUILTIN_TYPE_BVEC3:
					sys_io_print("<GLSL_BUILTIN_TYPE_BVEC3>");
					break;
				case GLSL_BUILTIN_TYPE_BVEC4:
					sys_io_print("<GLSL_BUILTIN_TYPE_BVEC4>");
					break;
				case GLSL_BUILTIN_TYPE_UINT:
					sys_io_print("<GLSL_BUILTIN_TYPE_UINT>");
					break;
				case GLSL_BUILTIN_TYPE_UVEC2:
					sys_io_print("<GLSL_BUILTIN_TYPE_UVEC2>");
					break;
				case GLSL_BUILTIN_TYPE_UVEC3:
					sys_io_print("<GLSL_BUILTIN_TYPE_UVEC3>");
					break;
				case GLSL_BUILTIN_TYPE_UVEC4:
					sys_io_print("<GLSL_BUILTIN_TYPE_UVEC4>");
					break;
			}
			break;
	}
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
			_print_ast_node(var->value,6);
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



SYS_PUBLIC void glsl_debug_print_compilation_output(const glsl_compilation_output_t* output){
	sys_io_print("{\n  locals: %u\n  constants: [",output->local_count);
	for (u32 i=0;i<output->const_count;i+=4){
		sys_io_print("\n    %f, %f, %f, %f%s",output->consts[i],output->consts[i+1],output->consts[i+2],output->consts[i+3],(i==output->const_count-1?"":","));
	}
	if (output->const_count){
		sys_io_print("\n  ");
	}
	sys_io_print("],\n  vars: [");
	for (u32 i=0;i<output->var_count;i++){
		const glsl_compilation_output_var_t* var=output->vars+i;
		sys_io_print("\n    {\n      name: '%s',\n      type: %s,\n      slot: %d\n    }%s",var->name,_glsl_compilation_output_var_type_to_string[var->type],var->slot,(i==output->var_count-1?"":","));
	}
	if (output->var_count){
		sys_io_print("\n  ");
	}
	sys_io_print("],\n  instructions: [");
	for (u32 i=0;i<output->instruction_count;i++){
		const glsl_instruction_t* instruction=output->instructions[i];
		sys_io_print("\n    %s",_glsl_instruction_type_to_string[instruction->type]);
		for (u32 j=0;j<instruction->arg_count;j++){
			const glsl_instruction_arg_t* arg=instruction->args+j;
			if (!j){
				sys_io_print(" ");
			}
			else{
				sys_io_print(", ");
			}
			if (arg->pattern_length_and_flags&GLSL_INSTRUCTION_ARG_FLAG_CONST){
				sys_io_print("const[%u]",arg->index);
			}
			else if (arg->pattern_length_and_flags&GLSL_INSTRUCTION_ARG_FLAG_LOCAL){
				sys_io_print("local[%u]",arg->index);
			}
			else{
				sys_io_print("%s[%u]",(output->vars+arg->index)->name,((arg->pattern_length_and_flags>>2)&0xf));
			}
			sys_io_print(".");
			for (u32 k=0;k<=(arg->pattern_length_and_flags&3);k++){
				sys_io_print("%c","xyzw"[(arg->pattern>>(k<<1))&3]);
			}
		}
	}
	if (output->instruction_count){
		sys_io_print("\n  ");
	}
	sys_io_print("]\n}\n");
}



SYS_PUBLIC void glsl_debug_print_linked_program(const glsl_linker_linked_program_t* linked_program){
	sys_io_print("{\n  uniforms: [");
	for (u32 i=0;i<linked_program->uniform_count;i++){
		const glsl_linker_linked_program_uniform_t* uniform=linked_program->uniforms+i;
		sys_io_print("\n    {\n      name: '%s',\n      slot: %u,\n      slot_count: %u\n    }%s",uniform->name,uniform->slot,uniform->slot_count,(i==linked_program->uniform_count-1?"":","));
	}
	if (linked_program->uniform_count){
		sys_io_print("\n  ");
	}
	sys_io_print("],\n  shaders: {");
	for (glsl_shader_type_t i=0;i<=GLSL_SHADER_MAX_TYPE;i++){
		if (!(linked_program->shader_bitmap&(1<<i))){
			continue;
		}
		const glsl_linker_linked_program_shader_t* shader=linked_program->shaders+i;
		sys_io_print("\n    %s: [",_glsl_shader_type_to_string[i]);
		for (u32 j=0;j<shader->length;j++){
			if (!(j%LINKED_SHADER_BYTES_PER_LINE)){
				sys_io_print("\n      ");
			}
			else{
				sys_io_print(" ");
			}
			sys_io_print("%X",*((const char*)(shader->data+j)));
		}
		if (shader->length){
			sys_io_print("\n    ");
		}
		sys_io_print("]%s",((linked_program->shader_bitmap>>(i+1))?",":""));
	}
	sys_io_print("\n  }\n}\n");
}
