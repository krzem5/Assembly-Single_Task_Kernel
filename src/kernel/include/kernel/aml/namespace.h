#ifndef _KERNEL_AML_NAMESPACE_H_
#define _KERNEL_AML_NAMESPACE_H_ 1
#include <kernel/aml/object.h>
#include <kernel/types.h>



#define AML_NAMESPACE_LOOKUP_FLAG_CREATE 1
#define AML_NAMESPACE_LOOKUP_FLAG_CLEAR 2
#define AML_NAMESPACE_LOOKUP_FLAG_LOCAL 4



typedef struct _AML_NAMESPACE{
	char name[5];
	aml_object_t* value;
	struct _AML_NAMESPACE* parent;
	struct _AML_NAMESPACE* prev_sibling;
	struct _AML_NAMESPACE* next_sibling;
	struct _AML_NAMESPACE* first_child;
} aml_namespace_t;



aml_namespace_t* aml_namespace_lookup(aml_namespace_t* root,const char* path,u8 flags);



#endif
