#ifndef _COMMON_KFS2_IO_H_
#define _COMMON_KFS2_IO_H_ 1
#include <common/kfs2/api.h>
#include <common/kfs2/structures.h>
#include <common/kfs2/util.h>



void kfs2_io_inode_read(kfs2_filesystem_t* fs,u32 inode,kfs2_node_t* out);



void kfs2_io_inode_write(kfs2_filesystem_t* fs,kfs2_node_t* node);



void kfs2_io_data_block_read(kfs2_filesystem_t* fs,u64 block_index,void* buffer);



void kfs2_io_data_block_write(kfs2_filesystem_t* fs,u64 block_index,const void* buffer);



#endif
