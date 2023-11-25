#ifndef _AML_FIELD_H_
#define _AML_FIELD_H_ 1
#include <aml/object.h>
#include <kernel/types.h>



aml_object_t* aml_field_read(aml_object_t* object);



_Bool aml_field_write(aml_object_t* object,aml_object_t* value);



#endif
