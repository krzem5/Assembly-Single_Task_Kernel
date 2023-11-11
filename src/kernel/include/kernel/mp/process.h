#ifndef _KERNEL_MP_PROCESS_H_
#define _KERNEL_MP_PROCESS_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/memory/mmap.h>
#include <kernel/mp/_mp_types.h>



extern handle_type_t HANDLE_TYPE_PROCESS;
extern process_t* process_kernel;
extern mmap_t process_kernel_image_mmap;



void process_init(void);



process_t* process_new(const char* image,const char* name);



#endif
