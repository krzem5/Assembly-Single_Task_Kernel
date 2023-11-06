#ifndef _DEVFS_FS_H_
#define _DEVFS_FS_H_ 1
#include <kernel/fs/fs.h>
#include <kernel/memory/smm.h>
#include <kernel/vfs/node.h>



extern filesystem_t* devfs;



void devfs_create_fs(void);



vfs_node_t* devfs_create_node(vfs_node_t* parent,const char* name,string_t* data);



void devfs_create_data_node(vfs_node_t* parent,const char* name,const char* format,...);



void devfs_create_link_node(vfs_node_t* parent,const char* name,const char* format,...);



#endif
