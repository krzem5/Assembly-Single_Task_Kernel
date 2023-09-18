#ifndef _KERNEL_MP_PROCESS_H_
#define _KERNEL_MP_PROCESS_H_ 1
#include <kernel/mp/_mp_types.h>



process_t* process_new(void);



void process_delete(process_t* process);



#endif
