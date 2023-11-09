#ifndef _DYNAMICFS_DYNAMICFS_H_
#define _DYNAMICFS_DYNAMICFS_H_ 1
#include <kernel/fs/fs.h>
#include <kernel/memory/smm.h>
#include <kernel/types.h>
#include <kernel/vfs/node.h>



filesystem_t* dynamicfs_init(const char* path,filesystem_descriptor_t* fs_descriptor);



vfs_node_t* dynamicfs_create_node(vfs_node_t* parent,const char* name,u32 type,string_t* data);



vfs_node_t* dynamicfs_create_data_node(vfs_node_t* parent,const char* name,const char* format,...);



vfs_node_t* dynamicfs_create_link_node(vfs_node_t* parent,const char* name,const char* format,...);



#endif
