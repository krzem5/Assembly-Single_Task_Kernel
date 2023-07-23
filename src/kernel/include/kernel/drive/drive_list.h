#ifndef _KERNEL_DRIVE_DRIVE_LIST_H_
#define _KERNEL_DRIVE_DRIVE_LIST_H_ 1
#include <kernel/drive/drive.h>
#include <kernel/types.h>



void drive_list_init(void);



void drive_list_add_drive(const drive_t* drive);



void drive_list_load_partitions(void);



u32 drive_list_get_length(void);



const drive_t* drive_list_get_drive(u32 index);



#endif
