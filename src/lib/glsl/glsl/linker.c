#include <glsl/_internal/error.h>
#include <glsl/_internal/interface_allocator.h>
#include <glsl/ast.h>
#include <glsl/backend.h>
#include <glsl/error.h>
#include <glsl/linker.h>
#include <sys/heap/heap.h>
#include <sys/io/io.h>
#include <sys/string/string.h>
#include <sys/types.h>



#define gl_MaxVertexAttribs 16
#define gl_MaxVertexUniformComponents 1024
#define gl_MaxFragmentInputComponents 128



static void _remove_unused_vars(glsl_ast_t* ast){
	for (u32 i=0;i<ast->var_count;i++){
_retry:
		glsl_ast_var_t* var=ast->vars[i];
		if ((var->storage.type==GLSL_AST_VAR_STORAGE_TYPE_DEFAULT&&var->usage_flags==(GLSL_AST_VAR_USAGE_FLAG_READ|GLSL_AST_VAR_USAGE_FLAG_WRITE))||(var->storage.type!=GLSL_AST_VAR_STORAGE_TYPE_DEFAULT&&var->usage_flags)||(var->storage.flags&GLSL_AST_VAR_STORAGE_FLAG_BLOCK)||(var->flags&GLSL_AST_VAR_FLAG_BUILTIN)||!sys_string_compare(var->name,"main")){
			continue;
		}
		sys_heap_dealloc(NULL,var->name);
		glsl_ast_type_delete(var->type);
		if (var->value){
			glsl_ast_node_delete(var->value);
		}
		sys_heap_dealloc(NULL,var);
		ast->var_count--;
		ast->vars[i]=ast->vars[ast->var_count];
		goto _retry; // cannot continue due to possible underflow during loop condition
	}
	ast->vars=sys_heap_realloc(NULL,ast->vars,ast->var_count*sizeof(glsl_ast_var_t*));
}



static glsl_error_t _allocate_slots(glsl_ast_t* ast,_Bool alloc_input,u32 max_slots){
	glsl_error_t error=GLSL_NO_ERROR;
	glsl_interface_allocator_t allocator;
	_glsl_interface_allocator_init(max_slots,&allocator);
	u8 mask=GLSL_AST_VAR_STORAGE_FLAG_HAS_LAYOUT_LOCATION;
_second_pass:
	for (u32 i=0;i<ast->var_count;i++){
		glsl_ast_var_t* var=ast->vars[i];
		if (var->storage.type==(alloc_input?GLSL_AST_VAR_STORAGE_TYPE_IN:GLSL_AST_VAR_STORAGE_TYPE_OUT)&&!(var->flags&(GLSL_AST_VAR_FLAG_LINKED|GLSL_AST_VAR_FLAG_BUILTIN))&&(var->storage.flags&mask)==mask){
			var->flags|=GLSL_AST_VAR_FLAG_LINKED;
			var->link_slot=(mask?var->storage.layout_location:0xffffffff);
			if (!_glsl_interface_allocator_reserve(&allocator,&(var->link_slot),glsl_ast_type_get_slot_count(var->type))){
				error=_glsl_error_create_linker_unallocatable_layout(var->name);
				goto _error;
			}
		}
	}
	if (mask){
		mask=0;
		goto _second_pass;
	}
	_glsl_interface_allocator_deinit(&allocator);
	return GLSL_NO_ERROR;
_error:
	_glsl_interface_allocator_deinit(&allocator);
	return error;
}



static glsl_error_t _allocate_uniform_slots(glsl_linker_program_t* program,glsl_linker_linked_program_t* linked_program){
	glsl_error_t error=GLSL_NO_ERROR;
	glsl_interface_allocator_t allocator;
	_glsl_interface_allocator_init(gl_MaxVertexUniformComponents,&allocator);
	u8 mask=GLSL_AST_VAR_STORAGE_FLAG_HAS_LAYOUT_LOCATION;
_second_pass:
	for (glsl_shader_type_t i=0;i<=GLSL_SHADER_MAX_TYPE;i++){
		if (!(program->shader_bitmap&(1<<i))){
			continue;
		}
		for (u32 j=0;j<(program->shaders+i)->var_count;j++){
			glsl_ast_var_t* var=(program->shaders+i)->vars[j];
			if (var->storage.type==GLSL_AST_VAR_STORAGE_TYPE_UNIFORM&&!(var->flags&(GLSL_AST_VAR_FLAG_LINKED|GLSL_AST_VAR_FLAG_BUILTIN))&&(var->storage.flags&mask)==mask){
				var->flags|=GLSL_AST_VAR_FLAG_LINKED;
				for (u32 k=0;k<linked_program->uniform_count;k++){
					const glsl_linker_linked_program_uniform_t* uniform=linked_program->uniforms+k;
					if (!sys_string_compare(uniform->name,var->name)){
						if ((var->storage.flags&GLSL_AST_VAR_STORAGE_FLAG_HAS_LAYOUT_LOCATION)&&var->storage.layout_location!=uniform->slot){
							error=_glsl_error_create_linker_inconsistent_layout(var->name);
							goto _error;
						}
						if (!glsl_ast_type_is_equal(var->type,uniform->_type)){
							error=_glsl_error_create_linker_wrong_type(var->name,var->type,uniform->_type);
							goto _error;
						}
						var->link_slot=uniform->slot;
						goto _skip_slot_allocation;
					}
				}
				var->link_slot=(mask?var->storage.layout_location:0xffffffff);
				if (!_glsl_interface_allocator_reserve(&allocator,&(var->link_slot),glsl_ast_type_get_slot_count(var->type))){
					error=_glsl_error_create_linker_unallocatable_layout(var->name);
					goto _error;
				}
				linked_program->uniform_count++;
				linked_program->uniforms=sys_heap_realloc(NULL,linked_program->uniforms,linked_program->uniform_count*sizeof(glsl_linker_linked_program_uniform_t));
				(linked_program->uniforms+linked_program->uniform_count-1)->name=sys_string_duplicate(var->name);
				(linked_program->uniforms+linked_program->uniform_count-1)->slot=var->link_slot;
				(linked_program->uniforms+linked_program->uniform_count-1)->size=glsl_ast_type_get_slot_count(var->type);
				(linked_program->uniforms+linked_program->uniform_count-1)->_type=glsl_ast_type_duplicate(var->type);
_skip_slot_allocation:
			}
		}
	}
	if (mask){
		mask=0;
		goto _second_pass;
	}
	_glsl_interface_allocator_deinit(&allocator);
	return GLSL_NO_ERROR;
_error:
	_glsl_interface_allocator_deinit(&allocator);
	return error;
}



static glsl_error_t _check_compatibility(glsl_ast_t* output,glsl_ast_t* input,u32 max_slots){
	glsl_error_t error=GLSL_NO_ERROR;
	glsl_interface_allocator_t allocator;
	_glsl_interface_allocator_init(max_slots,&allocator);
	u8 mask=GLSL_AST_VAR_STORAGE_FLAG_HAS_LAYOUT_LOCATION;
_second_pass:
	for (u32 i=0;i<output->var_count;i++){
		glsl_ast_var_t* output_var=output->vars[i];
		if (output_var->storage.type!=GLSL_AST_VAR_STORAGE_TYPE_OUT||(output_var->storage.flags&mask)!=mask){
			continue;
		}
		for (u32 j=0;j<input->var_count;j++){
			glsl_ast_var_t* input_var=input->vars[j];
			if (input_var->storage.type==GLSL_AST_VAR_STORAGE_TYPE_IN&&!(input_var->flags&(GLSL_AST_VAR_FLAG_LINKED|GLSL_AST_VAR_FLAG_BUILTIN))&&!sys_string_compare(input_var->name,output_var->name)){
				if (!glsl_ast_type_is_equal(input_var->type,output_var->type)){
					error=_glsl_error_create_linker_wrong_type(output_var->name,output_var->type,input_var->type);
					goto _error;
				}
				if (((input_var->storage.flags^output_var->storage.flags)&GLSL_AST_VAR_STORAGE_FLAG_HAS_LAYOUT_LOCATION)||(mask&&input_var->storage.layout_location!=output_var->storage.layout_location)){
					error=_glsl_error_create_linker_inconsistent_layout(output_var->name);
					goto _error;
				}
				output_var->flags|=GLSL_AST_VAR_FLAG_LINKED;
				output_var->link_slot=(mask?output_var->storage.layout_location:0xffffffff);
				if (!_glsl_interface_allocator_reserve(&allocator,&(output_var->link_slot),glsl_ast_type_get_slot_count(output_var->type))){
					error=_glsl_error_create_linker_unallocatable_layout(output_var->name);
					goto _error;
				}
				input_var->flags|=GLSL_AST_VAR_FLAG_LINKED;
				input_var->link_slot=output_var->link_slot;
				goto _link_next_var;
			}
		}
		error=_glsl_error_create_linker_unlinked_var(output_var->name,1);
		goto _error;
_link_next_var:
	}
	if (mask){
		mask=0;
		goto _second_pass;
	}
	_glsl_interface_allocator_deinit(&allocator);
	for (u32 i=0;i<input->var_count;i++){
		glsl_ast_var_t* input_var=input->vars[i];
		if (input_var->storage.type!=GLSL_AST_VAR_STORAGE_TYPE_IN||(input_var->flags&(GLSL_AST_VAR_FLAG_LINKED|GLSL_AST_VAR_FLAG_BUILTIN))){
			continue;
		}
		return _glsl_error_create_linker_unlinked_var(input_var->name,0);
	}
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
			glsl_ast_delete(program->shaders+i);
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
		if ((linked_program->uniforms+i)->_type){
			glsl_ast_type_delete((linked_program->uniforms+i)->_type);
		}
	}
	sys_heap_dealloc(NULL,linked_program->uniforms);
	linked_program->shader_bitmap=0;
	linked_program->uniform_count=0;
	linked_program->uniforms=NULL;
}



SYS_PUBLIC glsl_error_t glsl_linker_program_link(glsl_linker_program_t* program,const glsl_backend_descriptor_t* backend,glsl_linker_linked_program_t* out){
	out->shader_bitmap=0;
	out->uniform_count=0;
	for (glsl_shader_type_t i=0;i<=GLSL_SHADER_MAX_TYPE;i++){
		(out->shaders+i)->data=NULL;
		(out->shaders+i)->length=0;
	}
	out->uniforms=NULL;
	for (glsl_shader_type_t i=0;i<=GLSL_SHADER_MAX_TYPE;i++){
		if (program->shader_bitmap&(1<<i)){
			_remove_unused_vars(program->shaders+i);
		}
	}
	if (!(program->shader_bitmap&(1<<GLSL_SHADER_TYPE_VERTEX))){
		return _glsl_error_create_linker_missing_shader(GLSL_SHADER_TYPE_VERTEX);
	}
	if (!(program->shader_bitmap&(1<<GLSL_SHADER_TYPE_FRAGMENT))){
		return _glsl_error_create_linker_missing_shader(GLSL_SHADER_TYPE_FRAGMENT);
	}
	glsl_error_t error=_allocate_slots(program->shaders+GLSL_SHADER_TYPE_VERTEX,1,gl_MaxVertexAttribs);
	if (error!=GLSL_NO_ERROR){
		return error;
	}
	error=_check_compatibility(program->shaders+GLSL_SHADER_TYPE_VERTEX,program->shaders+GLSL_SHADER_TYPE_FRAGMENT,gl_MaxFragmentInputComponents);
	if (error!=GLSL_NO_ERROR){
		return error;
	}
	error=_allocate_slots(program->shaders+GLSL_SHADER_TYPE_FRAGMENT,0,1);
	if (error!=GLSL_NO_ERROR){
		return error;
	}
	error=_allocate_uniform_slots(program,out);
	if (error!=GLSL_NO_ERROR){
		goto _cleanup;
	}
	for (u32 i=0;i<out->uniform_count;i++){
		glsl_ast_type_delete((out->uniforms+i)->_type);
		(out->uniforms+i)->_type=NULL;
	}
	if (!backend){
		return GLSL_NO_ERROR;
	}
	for (glsl_shader_type_t i=0;i<=GLSL_SHADER_MAX_TYPE;i++){
		if (!(program->shader_bitmap&(1<<i))){
			continue;
		}
		error=backend->shader_link_callback(program->shaders+i,i,out->shaders+i);
		if (error!=GLSL_NO_ERROR){
			goto _cleanup;
		}
	}
	return GLSL_NO_ERROR;
_cleanup:
	glsl_linker_linked_program_delete(out);
	return error;
}
