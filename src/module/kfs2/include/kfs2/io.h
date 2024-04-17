#ifndef _KFS2_IO_H_
#define _KFS2_IO_H_ 1
#include <kernel/fs/fs.h>
#include <kernel/memory/smm.h>
#include <kernel/types.h>
#include <kfs2/api.h>
#include <kfs2/structures.h>



vfs_node_t* kfs2_io_inode_read(filesystem_t* fs,const string_t* name,u32 inode);



void kfs2_io_inode_write(kfs2_filesystem_t* fs,kfs2_node_t* node);



void kfs2_io_data_block_read(kfs2_filesystem_t* fs,u64 block_index,void* buffer);



void kfs2_io_data_block_write(kfs2_filesystem_t* fs,u64 block_index,const void* buffer);



#endif
