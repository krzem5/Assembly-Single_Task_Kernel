#ifndef _PROCFS_FS_H_
#define _PROCFS_FS_H_ 1
#include <kernel/fs/fs.h>



extern filesystem_t* procfs;



void procfs_create_fs(void);



#endif
