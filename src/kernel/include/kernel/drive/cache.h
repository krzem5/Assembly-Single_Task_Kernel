#ifndef _KERNEL_DRIVE_CACHE_H_
#define _KERNEL_DRIVE_CACHE_H_ 1
#include <kernel/drive/drive.h>
#include <kernel/types.h>



u64 drive_cache_get(drive_t* drive,u64 offset,void* buffer,u64 size);



void drive_cache_set(drive_t* drive,u64 offset,const void* buffer,u64 size);



#endif
