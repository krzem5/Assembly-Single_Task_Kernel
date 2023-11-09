#ifndef _DEVFS_FS_H_
#define _DEVFS_FS_H_ 1
#include <kernel/fs/fs.h>



extern filesystem_t* devfs;



void devfs_create_fs(void);



#endif
