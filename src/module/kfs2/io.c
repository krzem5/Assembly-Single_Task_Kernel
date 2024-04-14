#include <kernel/drive/drive.h>
#include <kernel/fs/fs.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/memory/vmm.h>
#include <kernel/module/module.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#include <kfs2/crc.h>
#include <kfs2/io.h>
#include <kfs2/structures.h>



static pmm_counter_descriptor_t* KERNEL_INIT_WRITE _kfs2_io_inode_buffer_pmm_counter=NULL;



MODULE_INIT(){
	_kfs2_io_inode_buffer_pmm_counter=pmm_alloc_counter("kfs2_io_inode_buffer");
}



vfs_node_t* kfs2_io_inode_read(filesystem_t* fs,const string_t* name,u32 inode){
	void* buffer=(void*)(pmm_alloc(1,_kfs2_io_inode_buffer_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	partition_t* partition=fs->partition;
	drive_t* drive=partition->drive;
	kfs2_fs_extra_data_t* extra_data=fs->extra_data;
	kfs2_vfs_node_t* out=NULL;
	if (drive_read(drive,partition->start_lba+((extra_data->root_block.first_inode_block+KFS2_INODE_GET_BLOCK_INDEX(inode))<<extra_data->block_size_shift),buffer,1<<extra_data->block_size_shift)!=(1<<extra_data->block_size_shift)){
		panic("kfs2_io_inode_read: I/O error");
	}
	kfs2_node_t* node=(void*)(buffer+KFS2_INODE_GET_NODE_INDEX(inode)*sizeof(kfs2_node_t));
	if (!kfs2_verify_crc(node,sizeof(kfs2_node_t))){
		panic("kfs2_io_inode_read: invlaid CRC");
	}
	out=(kfs2_vfs_node_t*)vfs_node_create(fs,NULL,name,0);
	if ((node->flags&KFS2_INODE_TYPE_MASK)==KFS2_INODE_TYPE_DIRECTORY){
		out->node.flags|=VFS_NODE_TYPE_DIRECTORY;
	}
	else if ((node->flags&KFS2_INODE_TYPE_MASK)==KFS2_INODE_TYPE_LINK){
		out->node.flags|=VFS_NODE_TYPE_LINK;
	}
	else{
		out->node.flags|=VFS_NODE_TYPE_FILE;
	}
	out->node.flags|=((node->flags&KFS2_INODE_PERMISSION_MASK)>>KFS2_INODE_PERMISSION_SHIFT)<<VFS_NODE_PERMISSION_SHIFT;
	out->node.time_access=node->time_access;
	out->node.time_modify=node->time_modify;
	out->node.time_change=node->time_change;
	out->node.time_birth=node->time_birth;
	out->node.gid=node->gid;
	out->node.uid=node->uid;
	out->kfs2_node=*node;
	out->kfs2_node._inode=inode;
	pmm_dealloc(((u64)buffer)-VMM_HIGHER_HALF_ADDRESS_OFFSET,1,_kfs2_io_inode_buffer_pmm_counter);
	return (vfs_node_t*)out;
}



void kfs2_io_inode_write(kfs2_vfs_node_t* node){
	void* buffer=(void*)(pmm_alloc(1,_kfs2_io_inode_buffer_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	partition_t* partition=node->node.fs->partition;
	drive_t* drive=partition->drive;
	kfs2_fs_extra_data_t* extra_data=node->node.fs->extra_data;
	if (drive_read(drive,partition->start_lba+((extra_data->root_block.first_inode_block+KFS2_INODE_GET_BLOCK_INDEX(node->kfs2_node._inode))<<extra_data->block_size_shift),buffer,1<<extra_data->block_size_shift)!=(1<<extra_data->block_size_shift)){
		panic("kfs2_io_inode_write: I/O error");
	}
	kfs2_node_t* tmp_node=(void*)(buffer+KFS2_INODE_GET_NODE_INDEX(node->kfs2_node._inode)*sizeof(kfs2_node_t));
	*tmp_node=node->kfs2_node;
	kfs2_insert_crc(tmp_node,sizeof(kfs2_node_t));
	if (drive_write(drive,partition->start_lba+((extra_data->root_block.first_inode_block+KFS2_INODE_GET_BLOCK_INDEX(node->kfs2_node._inode))<<extra_data->block_size_shift),buffer,1<<extra_data->block_size_shift)!=(1<<extra_data->block_size_shift)){
		panic("kfs2_io_inode_write: I/O error");
	}
	pmm_dealloc(((u64)buffer)-VMM_HIGHER_HALF_ADDRESS_OFFSET,1,_kfs2_io_inode_buffer_pmm_counter);
}



void kfs2_io_data_block_read(filesystem_t* fs,u64 block_index,void* buffer){
	kfs2_fs_extra_data_t* extra_data=fs->extra_data;
	partition_t* partition=fs->partition;
	drive_t* drive=partition->drive;
	if (drive_read(drive,partition->start_lba+((extra_data->root_block.first_data_block+block_index)<<extra_data->block_size_shift),buffer,1<<extra_data->block_size_shift)!=(1<<extra_data->block_size_shift)){
		panic("kfs2_io_data_block_read: I/O error");
	}
}



void kfs2_io_data_block_write(filesystem_t* fs,u64 block_index,const void* buffer){
	kfs2_fs_extra_data_t* extra_data=fs->extra_data;
	partition_t* partition=fs->partition;
	drive_t* drive=partition->drive;
	if (drive_write(drive,partition->start_lba+((extra_data->root_block.first_data_block+block_index)<<extra_data->block_size_shift),buffer,1<<extra_data->block_size_shift)!=(1<<extra_data->block_size_shift)){
		panic("kfs2_io_data_block_write: I/O error");
	}
}
