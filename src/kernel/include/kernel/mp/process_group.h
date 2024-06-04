#ifndef _KERNEL_MP_PROCESS_GROUP_H_
#define _KERNEL_MP_PROCESS_GROUP_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/mp/_mp_types.h>



extern handle_type_t process_group_handle_type;



process_group_t* process_group_create(process_t* process);



void process_group_join(process_group_t* group,process_t* process);



void process_group_leave(process_group_t* group,process_t* process);



#endif
