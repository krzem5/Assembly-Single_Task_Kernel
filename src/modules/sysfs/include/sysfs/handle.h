#ifndef _SYSFS_HANDLE_H_
#define _SYSFS_HANDLE_H_ 1
#include <kernel/vfs/node.h>



extern vfs_node_t* sysfs_handle_root;



void sysfs_handle_init(void);



#endif
