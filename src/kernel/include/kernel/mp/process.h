#ifndef _KERNEL_MP_PROCESS_H_
#define _KERNEL_MP_PROCESS_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/memory/mmap.h>
#include <kernel/mp/_mp_types.h>



#define PROCESS_ACL_FLAG_CREATE_THREAD 1
#define PROCESS_ACL_FLAG_TERMINATE 2



extern handle_type_t process_handle_type;
extern process_t* process_kernel;
extern mmap_t process_kernel_image_mmap;



void process_init(void);



process_t* process_create(const char* image,const char* name);



#endif
