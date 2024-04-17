#include <common/kfs2/api.h>
#include <common/kfs2/crc.h>
#include <common/kfs2/io.h>
#include <common/kfs2/structures.h>
#include <common/kfs2/util.h>



void kfs2_io_inode_read(kfs2_filesystem_t* fs,u32 inode,kfs2_node_t* out){
	void* buffer=fs->config.alloc_callback(1);
	if (fs->config.read_callback(fs->config.ctx,fs->config.start_lba+((fs->root_block.first_inode_block+KFS2_INODE_GET_BLOCK_INDEX(inode))<<fs->block_size_shift),buffer,1<<fs->block_size_shift)!=(1<<fs->block_size_shift)){
		panic("kfs2_io_inode_read: I/O error");
	}
	kfs2_node_t* node=(void*)(buffer+KFS2_INODE_GET_NODE_INDEX(inode)*sizeof(kfs2_node_t));
	if (!kfs2_verify_crc(node,sizeof(kfs2_node_t))){
		panic("kfs2_io_inode_read: invalid CRC");
	}
	*out=*node;
	out->_inode=inode;
	fs->config.dealloc_callback(buffer,1);
}



void kfs2_io_inode_write(kfs2_filesystem_t* fs,kfs2_node_t* node){
	void* buffer=fs->config.alloc_callback(1);
	if (fs->config.read_callback(fs->config.ctx,fs->config.start_lba+((fs->root_block.first_inode_block+KFS2_INODE_GET_BLOCK_INDEX(node->_inode))<<fs->block_size_shift),buffer,1<<fs->block_size_shift)!=(1<<fs->block_size_shift)){
		panic("kfs2_io_inode_write: I/O error");
	}
	kfs2_node_t* tmp_node=buffer+KFS2_INODE_GET_NODE_INDEX(node->_inode)*sizeof(kfs2_node_t);
	*tmp_node=*node;
	kfs2_insert_crc(tmp_node,sizeof(kfs2_node_t));
	if (fs->config.write_callback(fs->config.ctx,fs->config.start_lba+((fs->root_block.first_inode_block+KFS2_INODE_GET_BLOCK_INDEX(node->_inode))<<fs->block_size_shift),buffer,1<<fs->block_size_shift)!=(1<<fs->block_size_shift)){
		panic("kfs2_io_inode_write: I/O error");
	}
	fs->config.dealloc_callback(buffer,1);
}



void kfs2_io_data_block_read(kfs2_filesystem_t* fs,u64 block_index,void* buffer){
	if (fs->config.read_callback(fs->config.ctx,fs->config.start_lba+((fs->root_block.first_data_block+block_index)<<fs->block_size_shift),buffer,1<<fs->block_size_shift)!=(1<<fs->block_size_shift)){
		panic("kfs2_io_data_block_read: I/O error");
	}
}



void kfs2_io_data_block_write(kfs2_filesystem_t* fs,u64 block_index,const void* buffer){
	if (fs->config.write_callback(fs->config.ctx,fs->config.start_lba+((fs->root_block.first_data_block+block_index)<<fs->block_size_shift),buffer,1<<fs->block_size_shift)!=(1<<fs->block_size_shift)){
		panic("kfs2_io_data_block_read: I/O error");
	}
}
