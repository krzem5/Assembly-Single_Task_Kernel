#include <kernel/types.h>



KERNEL_PUBLIC u8 __attribute__((section(".module"))) wrong_module_section[1234];



void init(void){
	return;
}
