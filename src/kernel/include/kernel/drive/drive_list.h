#ifndef _KERNEL_DRIVE_DRIVE_LIST_H_
#define _KERNEL_DRIVE_DRIVE_LIST_H_ 1
#include <kernel/drive/drive.h>
#include <kernel/types.h>



extern drive_t* drive_data;



void drive_list_add_drive(const drive_t* drive);



void drive_list_load_partitions(void);



#endif
