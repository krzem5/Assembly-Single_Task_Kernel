#include <glsl/_internal/error.h>
#include <glsl/_internal/interface_allocator.h>
#include <glsl/backend.h>
#include <glsl/compiler.h>
#include <glsl/error.h>
#include <glsl/linker.h>
#include <sys/heap/heap.h>
#include <sys/io/io.h>
#include <sys/string/string.h>
#include <sys/types.h>



#define gl_MaxVertexAttribs 16
#define gl_MaxVertexUniformComponents 1024
#define gl_MaxFragmentInputComponents 128



static glsl_error_t _allocate_slots(glsl_compilation_output_t* output,glsl_compilation_output_var_type_t type,u32 max_slots){
	glsl_error_t error=GLSL_NO_ERROR;
	glsl_interface_allocator_t allocator;
	_glsl_interface_allocator_init(max_slots,&allocator);
	_Bool has_slot=1;
_second_pass:
	for (u32 i=0;i<output->var_count;i++){
		glsl_compilation_output_var_t* var=output->vars+i;
		if (var->type==type&&(var->slot!=0xffffffff)==has_slot){
			if (!_glsl_interface_allocator_reserve(&allocator,&(var->slot),var->slot_count,1)){
				error=_glsl_error_create_linker_unallocatable_layout(var->name);
				goto _error;
			}
		}
	}
	if (has_slot){
		has_slot=0;
		goto _second_pass;
	}
	_glsl_interface_allocator_deinit(&allocator);
	return GLSL_NO_ERROR;
_error:
	_glsl_interface_allocator_deinit(&allocator);
	return error;
}



static glsl_error_t _check_compatibility(glsl_compilation_output_t* output,glsl_compilation_output_t* input,u32 max_slots){
	glsl_error_t error=GLSL_NO_ERROR;
	glsl_interface_allocator_t allocator;
	_glsl_interface_allocator_init(max_slots,&allocator);
	_Bool has_slot=1;
_second_pass:
	for (u32 i=0;i<output->var_count;i++){
		glsl_compilation_output_var_t* output_var=output->vars+i;
		if ((output_var->slot!=0xffffffff)!=has_slot){
			continue;
		}
		if (output_var->type==GLSL_COMPILATION_OUTPUT_VAR_TYPE_BUILTIN_POSITION){
			if (!_glsl_interface_allocator_reserve(&allocator,&(output_var->slot),output_var->slot_count,1)){
				error=_glsl_error_create_linker_unallocatable_layout(output_var->name);
				goto _error;
			}
			continue;
		}
		if (output_var->type!=GLSL_COMPILATION_OUTPUT_VAR_TYPE_OUTPUT){
			continue;
		}
		for (u32 j=0;j<input->var_count;j++){
			glsl_compilation_output_var_t* input_var=input->vars+j;
			if (input_var->type!=GLSL_COMPILATION_OUTPUT_VAR_TYPE_INPUT||(input_var->slot!=0xffffffff)!=has_slot||sys_string_compare(input_var->name,output_var->name)){
				continue;
			}
			if (input_var->slot_count!=output_var->slot_count){
				error=_glsl_error_create_linker_wrong_type(output_var->name,output_var->slot_count,input_var->slot_count);
				goto _error;
			}
			if (output_var->slot!=input_var->slot){
				error=_glsl_error_create_linker_inconsistent_layout(output_var->name);
				goto _error;
			}
			if (!_glsl_interface_allocator_reserve(&allocator,&(output_var->slot),output_var->slot_count,1)){
				error=_glsl_error_create_linker_unallocatable_layout(output_var->name);
				goto _error;
			}
			input_var->slot=output_var->slot;
			goto _link_next_var;
		}
		error=_glsl_error_create_linker_unlinked_var(output_var->name,1);
		goto _error;
_link_next_var:
	}
	if (has_slot){
		has_slot=0;
		goto _second_pass;
	}
	_glsl_interface_allocator_deinit(&allocator);
	for (u32 j=0;j<input->var_count;j++){
		glsl_compilation_output_var_t* input_var=input->vars+j;
		if (input_var->type!=GLSL_COMPILATION_OUTPUT_VAR_TYPE_INPUT||input_var->slot!=0xffffffff){
			continue;
		}
		return _glsl_error_create_linker_unlinked_var(input_var->name,0);
	}
	return GLSL_NO_ERROR;
_error:
	_glsl_interface_allocator_deinit(&allocator);
	return error;
}



static glsl_error_t _allocate_global_slots(glsl_linker_program_t* program,glsl_linker_linked_program_t* linked_program){
	glsl_error_t error=GLSL_NO_ERROR;
	glsl_interface_allocator_t allocator;
	_glsl_interface_allocator_init(gl_MaxVertexUniformComponents,&allocator);
	_Bool has_slot=1;
	linked_program->uniform_slot_count=0;
_second_pass:
	for (glsl_shader_type_t i=0;i<=GLSL_SHADER_MAX_TYPE;i++){
		if (!(program->shader_bitmap&(1<<i))){
			continue;
		}
		for (u32 j=0;j<(program->shaders+i)->var_count;j++){
			glsl_compilation_output_var_t* var=(program->shaders+i)->vars+j;
			if ((var->type==GLSL_COMPILATION_OUTPUT_VAR_TYPE_UNIFORM||var->type==GLSL_COMPILATION_OUTPUT_VAR_TYPE_SAMPLER)&&(var->slot!=0xffffffff)==has_slot){
				for (u32 k=0;k<linked_program->uniform_count;k++){
					const glsl_linker_linked_program_uniform_t* uniform=linked_program->uniforms+k;
					if (!sys_string_compare(uniform->name,var->name)){
						if (var->slot!=0xffffffff&&var->slot!=uniform->slot){
							error=_glsl_error_create_linker_inconsistent_layout(var->name);
							goto _error;
						}
						if (var->slot_count!=uniform->slot_count){
							error=_glsl_error_create_linker_wrong_type(var->name,var->slot_count,uniform->slot_count);
							goto _error;
						}
						var->slot=uniform->slot;
						goto _skip_slot_allocation;
					}
				}
				if (!_glsl_interface_allocator_reserve(&allocator,&(var->slot),var->slot_count,1)){
					error=_glsl_error_create_linker_unallocatable_layout(var->name);
					goto _error;
				}
				linked_program->uniform_count++;
				linked_program->uniforms=sys_heap_realloc(NULL,linked_program->uniforms,linked_program->uniform_count*sizeof(glsl_linker_linked_program_uniform_t));
				(linked_program->uniforms+linked_program->uniform_count-1)->name=sys_string_duplicate(var->name);
				(linked_program->uniforms+linked_program->uniform_count-1)->slot=var->slot;
				(linked_program->uniforms+linked_program->uniform_count-1)->slot_count=var->slot_count;
				if (var->slot+var->slot_count>linked_program->uniform_slot_count){
					linked_program->uniform_slot_count=var->slot+var->slot_count;
				}
_skip_slot_allocation:
			}
		}
	}
	if (has_slot){
		has_slot=0;
		goto _second_pass;
	}
	_glsl_interface_allocator_deinit(&allocator);
	return GLSL_NO_ERROR;
_error:
	_glsl_interface_allocator_deinit(&allocator);
	return error;
}



SYS_PUBLIC void glsl_linker_program_init(glsl_linker_program_t* program){
	program->shader_bitmap=0;
}



SYS_PUBLIC void glsl_linker_program_delete(glsl_linker_program_t* program){
	for (glsl_shader_type_t i=0;i<=GLSL_SHADER_MAX_TYPE;i++){
		if (program->shader_bitmap&(1<<i)){
			glsl_compiler_compilation_output_delete(program->shaders+i);
		}
	}
	program->shader_bitmap=0;
}



SYS_PUBLIC void glsl_linker_linked_program_delete(glsl_linker_linked_program_t* linked_program){
	for (glsl_shader_type_t i=0;i<=GLSL_SHADER_MAX_TYPE;i++){
		if (linked_program->shader_bitmap&(1<<i)){
			sys_heap_dealloc(NULL,(linked_program->shaders+i)->data);
			(linked_program->shaders+i)->data=NULL;
			(linked_program->shaders+i)->length=0;
		}
	}
	for (u32 i=0;i<linked_program->uniform_count;i++){
		sys_heap_dealloc(NULL,(linked_program->uniforms+i)->name);
	}
	sys_heap_dealloc(NULL,linked_program->uniforms);
	linked_program->shader_bitmap=0;
	linked_program->uniform_count=0;
	linked_program->uniforms=NULL;
}



SYS_PUBLIC glsl_error_t glsl_linker_attach_program(glsl_linker_program_t* program,glsl_compilation_output_t* output){
	if (program->shader_bitmap&(1<<output->shader_type)){
		return _glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
	}
	program->shader_bitmap|=1<<output->shader_type;
	program->shaders[output->shader_type]=*output;
	output->shader_type=0;
	output->var_count=0;
	output->instruction_count=0;
	output->const_count=0;
	output->local_count=0;
	output->_var_capacity=0;
	output->_instruction_capacity=0;
	output->vars=NULL;
	output->instructions=NULL;
	output->consts=NULL;
	return GLSL_NO_ERROR;
}



SYS_PUBLIC glsl_error_t glsl_linker_program_link(glsl_linker_program_t* program,const glsl_backend_descriptor_t* backend,glsl_linker_linked_program_t* out){
	glsl_error_t error=GLSL_NO_ERROR;
	out->shader_bitmap=0;
	out->uniform_count=0;
	for (glsl_shader_type_t i=0;i<=GLSL_SHADER_MAX_TYPE;i++){
		(out->shaders+i)->data=NULL;
		(out->shaders+i)->length=0;
	}
	out->uniforms=NULL;
	if (!(program->shader_bitmap&(1<<GLSL_SHADER_TYPE_VERTEX))){
		return _glsl_error_create_linker_missing_shader(GLSL_SHADER_TYPE_VERTEX);
	}
	if (!(program->shader_bitmap&(1<<GLSL_SHADER_TYPE_FRAGMENT))){
		return _glsl_error_create_linker_missing_shader(GLSL_SHADER_TYPE_FRAGMENT);
	}
	error=_allocate_slots(program->shaders+GLSL_SHADER_TYPE_VERTEX,GLSL_COMPILATION_OUTPUT_VAR_TYPE_INPUT,gl_MaxVertexAttribs);
	if (error!=GLSL_NO_ERROR){
		return error;
	}
	error=_allocate_slots(program->shaders+GLSL_SHADER_TYPE_FRAGMENT,GLSL_COMPILATION_OUTPUT_VAR_TYPE_OUTPUT,1);
	if (error!=GLSL_NO_ERROR){
		return error;
	}
	error=_check_compatibility(program->shaders+GLSL_SHADER_TYPE_VERTEX,program->shaders+GLSL_SHADER_TYPE_FRAGMENT,gl_MaxFragmentInputComponents);
	if (error!=GLSL_NO_ERROR){
		return error;
	}
	error=_allocate_global_slots(program,out);
	if (error!=GLSL_NO_ERROR){
		goto _cleanup;
	}
	if (!backend){
		return GLSL_NO_ERROR;
	}
	for (glsl_shader_type_t i=0;i<=GLSL_SHADER_MAX_TYPE;i++){
		if (!(program->shader_bitmap&(1<<i))){
			continue;
		}
		error=backend->shader_link_callback(program->shaders+i,out->shaders+i);
		if (error!=GLSL_NO_ERROR){
			goto _cleanup;
		}
		out->shader_bitmap|=1<<i;
	}
	return GLSL_NO_ERROR;
_cleanup:
	glsl_linker_linked_program_delete(out);
	return error;
}
