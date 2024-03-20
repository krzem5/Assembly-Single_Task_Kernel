#ifndef _KERNEL_AML_RUNTIME_H_
#define _KERNEL_AML_RUNTIME_H_ 1
#include <kernel/aml/namespace.h>
#include <kernel/aml/object.h>
#include <kernel/types.h>



typedef struct _AML_RUNTIME_VARS{
	aml_object_t* args[8];
	aml_object_t* locals[8];
	aml_object_t* ret;
} aml_runtime_vars_t;



typedef struct _AML_RUNTIME_CONTEXT{
	const u8*const data;
	const u64 length;
	aml_namespace_t* namespace;
	aml_runtime_vars_t* vars;
	u64 offset;
} aml_runtime_context_t;



aml_object_t* aml_runtime_execute_single(aml_runtime_context_t* ctx);



_Bool aml_runtime_execute(aml_runtime_context_t* ctx);



aml_object_t* aml_runtime_execute_method(aml_object_t* method,u8 arg_count,aml_object_t*const* args);



#endif
