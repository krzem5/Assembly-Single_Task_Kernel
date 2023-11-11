#ifndef _PROCFS_PROCESS_H_
#define _PROCFS_PROCESS_H_ 1
#include <kernel/vfs/node.h>



extern vfs_node_t* procfs_process_root;



void procfs_process_init(void);



#endif
