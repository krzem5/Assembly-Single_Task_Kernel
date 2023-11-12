#ifndef _SYSFS_FS_H_
#define _SYSFS_FS_H_ 1
#include <kernel/fs/fs.h>



extern filesystem_t* sysfs;



void sysfs_create_fs(void);



#endif
