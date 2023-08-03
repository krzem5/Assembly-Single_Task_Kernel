#ifndef _KERNEL_DRIVE_DRIVE_LIST_H_
#define _KERNEL_DRIVE_DRIVE_LIST_H_ 1
#include <kernel/drive/drive.h>
#include <kernel/types.h>



extern drive_t* KERNEL_CORE_DATA drive_data;
extern u32 KERNEL_CORE_DATA drive_count;



void drive_list_init(void);



void drive_list_add_drive(const drive_t* drive);



void drive_list_load_partitions(void);



#endif
