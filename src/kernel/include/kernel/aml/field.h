#ifndef _KERNEL_AML_FIELD_H_
#define _KERNEL_AML_FIELD_H_ 1
#include <kernel/aml/object.h>
#include <kernel/types.h>



aml_object_t* aml_field_read(aml_object_t* object);



bool aml_field_write(aml_object_t* object,aml_object_t* value);



#endif
