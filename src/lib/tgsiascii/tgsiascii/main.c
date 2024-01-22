#include <glsl/backend.h>
#include <glsl/compiler.h>
#include <glsl/error.h>
#include <glsl/linker.h>
#include <sys/format/format.h>
#include <sys/heap/heap.h>
#include <sys/io/io.h>
#include <sys/memory/memory.h>
#include <sys/string/string.h>
#include <sys/types.h>
#include <sys/util/var_arg.h>



#define COMPILATION_OUTPUT_BUFFER_GROWTH_SIZE 256 // power of 2



static const char* _glsl_instruction_type_to_tgsi_instruction[]={
	[GLSL_INSTRUCTION_TYPE_MOV]="MOV",
	[GLSL_INSTRUCTION_TYPE_ADD]="ADD",
	[GLSL_INSTRUCTION_TYPE_SUB]="SUB",
	[GLSL_INSTRUCTION_TYPE_MUL]="MUL",
	[GLSL_INSTRUCTION_TYPE_DIV]="DIV",
	[GLSL_INSTRUCTION_TYPE_MOD]="MOD",
	[GLSL_INSTRUCTION_TYPE_AND]="AND",
	[GLSL_INSTRUCTION_TYPE_IOR]="OR",
	[GLSL_INSTRUCTION_TYPE_XOR]="XOR",
	[GLSL_INSTRUCTION_TYPE_DP2]="DP2",
	[GLSL_INSTRUCTION_TYPE_DP3]="DP3",
	[GLSL_INSTRUCTION_TYPE_DP4]="DP4",
	[GLSL_INSTRUCTION_TYPE_EXP]="EX2",
	[GLSL_INSTRUCTION_TYPE_LOG]="LG2",
	[GLSL_INSTRUCTION_TYPE_POW]="POW",
	[GLSL_INSTRUCTION_TYPE_SRT]="SQRT",
	[GLSL_INSTRUCTION_TYPE_RCP]="RCP",
	[GLSL_INSTRUCTION_TYPE_MAD]="MAD",
	[GLSL_INSTRUCTION_TYPE_FLR]="FLR",
	[GLSL_INSTRUCTION_TYPE_CEL]="CEIL",
	[GLSL_INSTRUCTION_TYPE_RND]="ROUND",
	[GLSL_INSTRUCTION_TYPE_FRC]="FRC",
	[GLSL_INSTRUCTION_TYPE_SIN]="SIN",
	[GLSL_INSTRUCTION_TYPE_COS]="COS",
	[GLSL_INSTRUCTION_TYPE_IGN]="KILL",
	[GLSL_INSTRUCTION_TYPE_EMV]="EMIT",
	[GLSL_INSTRUCTION_TYPE_EMP]="ENDPRIM",
};



static void _output_string(glsl_linker_linked_program_shader_t* out,const char* str,u32 length){
	if (!length){
		length=sys_string_length(str);
	}
	if (((-out->length)&(COMPILATION_OUTPUT_BUFFER_GROWTH_SIZE-1))<length){
		out->data=sys_heap_realloc(NULL,out->data,(out->length+length+COMPILATION_OUTPUT_BUFFER_GROWTH_SIZE-1)&(-COMPILATION_OUTPUT_BUFFER_GROWTH_SIZE));
	}
	sys_memory_copy(str,out->data+out->length,length);
	out->length+=length;
}



static void _output_string_template(glsl_linker_linked_program_shader_t* out,const char* template,...){
	char buffer[256];
	sys_var_arg_list_t va;
	sys_var_arg_init(va,template);
	_output_string(out,buffer,sys_format_string_va(buffer,256,template,&va));
	sys_var_arg_deinit(va);
}



static glsl_error_t _glsl_shader_link_callback(const glsl_compilation_output_t* output,glsl_linker_linked_program_shader_t* out){
	out->data=NULL;
	out->length=0;
	switch (output->shader_type){
		case GLSL_SHADER_TYPE_VERTEX:
			_output_string(out,"VERT\n",0);
			break;
		case GLSL_SHADER_TYPE_FRAGMENT:
			_output_string(out,"FRAG\nPROPERTY FS_COLOR0_WRITES_ALL_CBUFS 1\n",0);
			break;
	}
	u32 max_input_slot=0;
	u32 max_output_slot=0;
	for (u32 i=0;i<output->var_count;i++){
		const glsl_compilation_output_var_t* var=output->vars+i;
		u32 slot_end=var->slot+var->slot_count;
		if (var->type==GLSL_COMPILATION_OUTPUT_VAR_TYPE_INPUT){
			max_input_slot=(slot_end>max_input_slot?slot_end:max_input_slot);
		}
		else if (var->type==GLSL_COMPILATION_OUTPUT_VAR_TYPE_OUTPUT){
			max_output_slot=(slot_end>max_output_slot?slot_end:max_output_slot);
		}
	}
	for (u32 i=0;i<output->var_count;i++){
		glsl_compilation_output_var_t* var=output->vars+i;
		const char* storage=NULL;
		const char* suffix="";
		switch (var->type){
			case GLSL_COMPILATION_OUTPUT_VAR_TYPE_INPUT:
				storage="IN";
				if (output->shader_type==GLSL_SHADER_TYPE_FRAGMENT){
					suffix=", GENERIC";
				}
				break;
			case GLSL_COMPILATION_OUTPUT_VAR_TYPE_OUTPUT:
				storage="OUT";
				if (output->shader_type==GLSL_SHADER_TYPE_FRAGMENT){
					suffix=", COLOR";
				}
				else{
					suffix=", GENERIC";
				}
				break;
			case GLSL_COMPILATION_OUTPUT_VAR_TYPE_UNIFORM:
				storage="CONST";
				break;
			case GLSL_COMPILATION_OUTPUT_VAR_TYPE_BUILTIN_POSITION:
				if (output->shader_type==GLSL_SHADER_TYPE_VERTEX){
					storage="OUT";
					var->type=GLSL_COMPILATION_OUTPUT_VAR_TYPE_OUTPUT;
					var->slot=max_output_slot;
					max_output_slot+=var->slot_count;
				}
				else{
					storage="IN";
					var->type=GLSL_COMPILATION_OUTPUT_VAR_TYPE_INPUT;
					var->slot=max_input_slot;
					max_input_slot+=var->slot_count;
				}
				suffix=", POSITION";
				break;
		}
		if (var->slot_count==1){
			_output_string_template(out,"DCL %s[%u]%s\n",storage,var->slot,suffix);
		}
		else{
			_output_string_template(out,"DCL %s[%u..%u]%s\n",storage,var->slot,var->slot+var->slot_count-1,suffix);
		}
	}
	if (output->local_count){
		_output_string_template(out,"DCL TEMP[0..%u], LOCAL\n",output->local_count-1);
	}
	for (u32 i=0;i<output->const_count;i+=4){
		_output_string_template(out,"IMM[%u] FLT32 {0x%w, 0x%w, 0x%w, 0x%w}\n",i>>2,output->consts_as_ints[i],output->consts_as_ints[i+1],output->consts_as_ints[i+2],output->consts_as_ints[i+3]);
	}
	u32 offset=0;
	for (u32 i=0;i<output->instruction_count;i++){
		const glsl_instruction_t* instruction=output->instructions[i];
		if (instruction->type==GLSL_INSTRUCTION_TYPE_NOP){
			offset++;
			continue;
		}
		_output_string_template(out,"%u:\t%s",i-offset,_glsl_instruction_type_to_tgsi_instruction[instruction->type]);
		for (u32 j=0;j<instruction->arg_count;j++){
			const glsl_instruction_arg_t* arg=instruction->args+j;
			const char* storage=NULL;
			u32 slot=arg->index;
			if (arg->pattern_length_and_flags&GLSL_INSTRUCTION_ARG_FLAG_CONST){
				storage="IMM";
			}
			else if (arg->pattern_length_and_flags&GLSL_INSTRUCTION_ARG_FLAG_LOCAL){
				storage="TEMP";
			}
			else{
				switch ((output->vars+arg->index)->type){
					case GLSL_COMPILATION_OUTPUT_VAR_TYPE_INPUT:
						storage="IN";
						break;
					case GLSL_COMPILATION_OUTPUT_VAR_TYPE_OUTPUT:
						storage="OUT";
						break;
					case GLSL_COMPILATION_OUTPUT_VAR_TYPE_UNIFORM:
						storage="CONST";
						break;
				}
				slot=(output->vars+arg->index)->slot+((arg->pattern_length_and_flags>>2)&0xf);
			}
			char swizzle[5];
			u8 pattern_length=(j?4:(arg->pattern_length_and_flags&3)+1);
			u8 pattern=arg->pattern&((1<<(((arg->pattern_length_and_flags&3)+1)<<1))-1);
			for (u32 k=0;k<pattern_length;k++){
				swizzle[k]="xyzw"[(pattern>>(k<<1))&3];
			}
			swizzle[pattern_length]=0;
			_output_string_template(out,"%s\t%s[%u].%s",(j?",":""),storage,slot,swizzle);
		}
		_output_string(out,"\n",0);
	}
	_output_string_template(out,"%u:\tEND\n%c",output->instruction_count-offset,0);
	for (;out->length&3;out->length++){
		*((char*)(out->data+out->length))=0;
	}
	out->data=sys_heap_realloc(NULL,out->data,out->length);
	return GLSL_NO_ERROR;
}



static const glsl_backend_descriptor_t _tgsi_glsl_backend_descriptor={
	"tgsi",
	_glsl_shader_link_callback
};



SYS_PUBLIC const glsl_backend_descriptor_t* _glsl_backend_query_descriptor(void){
	return &_tgsi_glsl_backend_descriptor;
}
