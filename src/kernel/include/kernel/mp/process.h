#ifndef _KERNEL_MP_PROCESS_H_
#define _KERNEL_MP_PROCESS_H_ 1
#include <kernel/mp/_mp_types.h>



extern process_t* process_kernel;



void process_init(void);



process_t* process_new(void);



#endif
