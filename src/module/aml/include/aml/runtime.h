#ifndef _AML_RUNTIME_H_
#define _AML_RUNTIME_H_ 1
#include <aml/namespace.h>
#include <aml/object.h>
#include <kernel/types.h>



typedef struct _AML_RUNTIME_CONTEXT{
	const u8*const data;
	const u64 length;
	aml_namespace_t* namespace;
	u64 offset;
} aml_runtime_context_t;



aml_object_t* aml_runtime_execute_single(aml_runtime_context_t* ctx);



_Bool aml_runtime_execute(aml_runtime_context_t* ctx);



#endif
