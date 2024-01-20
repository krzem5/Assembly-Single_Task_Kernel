#ifndef _DYNAMICFS_DYNAMICFS_H_
#define _DYNAMICFS_DYNAMICFS_H_ 1
#include <kernel/fs/fs.h>
#include <kernel/memory/smm.h>
#include <kernel/types.h>
#include <kernel/vfs/node.h>



typedef	u64 (*dynamicfs_read_callback_t)(void*,u64,void*,u64);



static inline void dynamicfs_set_root_only(vfs_node_t* node){
	node->flags=(node->flags&(~VFS_NODE_PERMISSION_MASK))|(((node->flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_DIRECTORY||(node->flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_LINK?0500:0400)<<VFS_NODE_PERMISSION_SHIFT);
}



filesystem_t* dynamicfs_init(const char* path,const filesystem_descriptor_config_t* fs_descriptor_config);



vfs_node_t* dynamicfs_create_node(vfs_node_t* parent,const char* name,u32 type,string_t* data,dynamicfs_read_callback_t read_callback,void* read_callback_ctx);



vfs_node_t* dynamicfs_create_data_node(vfs_node_t* parent,const char* name,const char* format,...);



vfs_node_t* dynamicfs_create_link_node(vfs_node_t* parent,const char* name,const char* format,...);



void dynamicfs_delete_node(vfs_node_t* node,_Bool delete_string);



u64 dynamicfs_process_simple_read(const void* data,u64 length,u64 offset,void* buffer,u64 size);



u64 dynamicfs_integer_read_callback(void* ctx,u64 offset,void* buffer,u64 size);



u64 dynamicfs_string_read_callback(void* ctx,u64 offset,void* buffer,u64 size);



#endif
