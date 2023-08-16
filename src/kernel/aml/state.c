#include <kernel/aml/aml.h>
#include <kernel/log/log.h>
#include <kernel/kernel.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "aml"



#include <kernel/serial/serial.h>
void aml_print_object(const aml_object_t* object,u32 indent){
	for (u32 i=0;i<indent;i++){
		serial_send(" ",1);
	}
	log("[%x %x]%s\n",object->opcode[0],object->opcode[1],((object->flags&AML_OBJECT_FLAG_BYTE_DATA)?" -> bytes":""));
	if (object->flags&AML_OBJECT_FLAG_BYTE_DATA){
		return;
	}
	for (u32 i=0;i<object->data_length;i++){
		aml_print_object(object->data.objects+i,indent+2);
	}
}



void aml_build_runtime(aml_object_t* root){
	LOG("Building AML runtime...");
	aml_print_object(root,0);
	for (;;);
}
