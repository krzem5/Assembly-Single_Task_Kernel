#ifndef _KERNEL_MP_PROCESS_H_
#define _KERNEL_MP_PROCESS_H_ 1
#include <kernel/mp/_mp_types.h>
#include <kernel/memory/mmap.h>



extern process_t* process_kernel;
extern mmap_t process_kernel_image_mmap;



void process_init(void);



process_t* process_new(void);



#endif
