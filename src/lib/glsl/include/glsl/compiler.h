#ifndef _GLSL_COMPILER_H_
#define _GLSL_COMPILER_H_ 1
#include <glsl/ast.h>
#include <glsl/error.h>
#include <glsl/shader.h>
#include <sys/types.h>



#define GLSL_COMPILATION_OUTPUT_VAR_TYPE_INPUT 0
#define GLSL_COMPILATION_OUTPUT_VAR_TYPE_OUTPUT 1
#define GLSL_COMPILATION_OUTPUT_VAR_TYPE_UNIFORM 2

#define GLSL_COMPILATION_OUTPUT_VAR_MAX_TYPE GLSL_COMPILATION_OUTPUT_VAR_TYPE_UNIFORM

#define GLSL_INSTRUCTION_TYPE_NOP 0 	// no-op
#define GLSL_INSTRUCTION_TYPE_MOV 1 	// move
#define GLSL_INSTRUCTION_TYPE_ADD 2 	// add
#define GLSL_INSTRUCTION_TYPE_SUB 3 	// subtract
#define GLSL_INSTRUCTION_TYPE_MUL 4 	// multiply
#define GLSL_INSTRUCTION_TYPE_DIV 5 	// divide
#define GLSL_INSTRUCTION_TYPE_MOD 6 	// modulo
#define GLSL_INSTRUCTION_TYPE_AND 7 	// bitwise and
#define GLSL_INSTRUCTION_TYPE_IOR 8 	// bitwise inclusive or
#define GLSL_INSTRUCTION_TYPE_XOR 9 	// bitwise exclusive or
#define GLSL_INSTRUCTION_TYPE_DP2 10	// 2D dot product
#define GLSL_INSTRUCTION_TYPE_DP3 11	// 3D dot product
#define GLSL_INSTRUCTION_TYPE_DP4 12	// 4D dot product
#define GLSL_INSTRUCTION_TYPE_EXP 13	// exponentiation (base 2)
#define GLSL_INSTRUCTION_TYPE_LOG 14	// logarithm (base 2)
#define GLSL_INSTRUCTION_TYPE_POW 15	// power
#define GLSL_INSTRUCTION_TYPE_SRT 16	// square root
#define GLSL_INSTRUCTION_TYPE_RCP 17	// reciprocal
#define GLSL_INSTRUCTION_TYPE_MAD 18	// multiply-and-add
#define GLSL_INSTRUCTION_TYPE_FLR 19	// floor
#define GLSL_INSTRUCTION_TYPE_CEL 20	// ceiling
#define GLSL_INSTRUCTION_TYPE_RND 21	// round
#define GLSL_INSTRUCTION_TYPE_FRC 22	// fraction
#define GLSL_INSTRUCTION_TYPE_SIN 23	// sine
#define GLSL_INSTRUCTION_TYPE_COS 24	// cosine
#define GLSL_INSTRUCTION_TYPE_IGN 25	// ignore (fragment shader only)
#define GLSL_INSTRUCTION_TYPE_EMV 26	// emit vertex (geometry shader only)
#define GLSL_INSTRUCTION_TYPE_EMP 27	// emit primitive (geometry shader only)

#define GLSL_INSTRUCTION_ARG_FLAG_CONST 0x40
#define GLSL_INSTRUCTION_ARG_FLAG_LOCAL 0x80



typedef u16 glsl_compilation_output_var_type_t;



typedef u32 glsl_instruction_type_t;



typedef struct _GLSL_INSTURCTION_ARG{
	u16 index;
	u8 pattern_length_and_flags;
	u8 pattern;
} glsl_instruction_arg_t;



typedef struct _GLSL_INSTURCTION{
	glsl_instruction_type_t type;
	u32 arg_count;
	glsl_instruction_arg_t args[6];
} glsl_instruction_t;



typedef struct _GLSL_COMPILATION_OUTPUT_VAR{
	glsl_compilation_output_var_type_t type;
	u16 slot_count;
	u32 slot;
	char* name;
} glsl_compilation_output_var_t;



typedef struct _GLSL_COMPILATION_OUTPUT{
	glsl_shader_type_t shader_type;
	u16 var_count;
	u16 instruction_count;
	u16 const_count;
	u16 local_count;
	u16 _var_capacity;
	u16 _instruction_capacity;
	glsl_compilation_output_var_t* vars;
	glsl_instruction_t** instructions;
	float* consts;
} glsl_compilation_output_t;



glsl_error_t glsl_compiler_compile(const glsl_ast_t* ast,glsl_compilation_output_t* out);



void glsl_compiler_compilation_output_delete(glsl_compilation_output_t* output);



#endif
