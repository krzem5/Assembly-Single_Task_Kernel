#include <kernel/drive/drive.h>
#include <kernel/vfs/allocator.h>
#include <kernel/vfs/vfs.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "vfs"



void* KERNEL_CORE_CODE vfs_alloc(partition_t* fs,const char* name,u8 name_length){
	if (name_length>63){
		name_length=63;
		WARN_CORE("vfs_node_t.name_length too long");
	}
	vfs_node_t* out=vfs_allocator_get(&(fs->allocator),VFS_NODE_ID_EMPTY,1);
	out->type=VFS_NODE_TYPE_FILE;
	out->vfs_index=fs->index;
	out->name_length=name_length;
	out->flags=0;
	memcpy(out->name,name,name_length+1);
	out->name[name_length]=0;
	out->parent=VFS_NODE_ID_UNKNOWN;
	out->prev_sibling=VFS_NODE_ID_UNKNOWN;
	out->next_sibling=VFS_NODE_ID_UNKNOWN;
	out->first_child=VFS_NODE_ID_UNKNOWN;
	return out;
}



_Bool vfs_dealloc(vfs_node_t* node){
	if (!node){
		return 1;
	}
	partition_t* fs=partition_get(node->vfs_index);
	lock_acquire_exclusive(&(fs->lock));
	_Bool out=fs->config->delete(fs,node);
	if (out){
		node->type=VFS_NODE_TYPE_INVALID;
	}
	lock_release_exclusive(&(fs->lock));
	return out;
}



vfs_node_t* vfs_get_by_id(vfs_node_id_t id){
	return vfs_allocator_get(&(partition_get(id>>56)->allocator),id,0);
}



vfs_node_t* KERNEL_CORE_CODE vfs_get_by_path(vfs_node_t* root,const char* path,u8 type){
	if (path[0]=='/'){
		if (!partition_boot){
			panic("Root file system not located yet; partition must be specified");
			return NULL;
		}
		root=partition_boot->root;
	}
	else{
		u16 partition_name_length=0;
		while (path[partition_name_length]&&path[partition_name_length]!=':'&&path[partition_name_length]!='/'){
			partition_name_length++;
		}
		if (path[partition_name_length]==':'){
			root=NULL;
			for (partition_t* fs=partition_data;fs;fs=fs->next){
				if (fs->name_length!=partition_name_length){
					continue;
				}
				for (u8 i=0;i<partition_name_length;i++){
					if (fs->name[i]!=path[i]){
						goto _check_next_fs;
					}
				}
				root=fs->root;
				break;
_check_next_fs:
			}
			if (!root){
				ERROR_CORE("Partition not found");
				return NULL;
			}
			path+=partition_name_length+1;
		}
	}
	if (!root){
		ERROR_CORE("No root provided");
		return NULL;
	}
	const char* name=path;
	u8 name_length=0;
	vfs_node_t* node_parent=root;
	vfs_node_t* node=root;
	while (path[0]){
		if (path[0]=='/'){
			path++;
			continue;
		}
		u8 i=1;
		while (path[i]&&path[i]!='/'){
			i++;
		}
		if (!node||node->type!=VFS_NODE_TYPE_DIRECTORY){
			return NULL;
		}
		if (i==1&&path[0]=='.'){
			path++;
			continue;
		}
		if (i==2&&path[0]=='.'&&path[1]=='.'){
			path+=2;
			if (!(node->flags&VFS_NODE_FLAG_ROOT)){
				node=vfs_get_relative(node,VFS_RELATIVE_PARENT);
			}
			continue;
		}
		name=path;
		name_length=i;
		node_parent=node;
		node=vfs_get_child(node,path,i);
		path+=i;
	}
	if (node||type==VFS_NODE_TYPE_INVALID||!name_length){
		return node;
	}
	partition_t* fs=partition_get(node_parent->vfs_index);
	lock_release_exclusive(&(fs->lock));
	node=fs->config->create(fs,type==VFS_NODE_TYPE_DIRECTORY,name,name_length);
	lock_release_exclusive(&(fs->lock));
	if (!node){
		return NULL;
	}
	vfs_node_t* first_child=vfs_get_relative(node_parent,VFS_RELATIVE_FIRST_CHILD);
	_Bool out=vfs_set_relative(node,VFS_RELATIVE_PARENT,node_parent);
	if (first_child){
		out&=vfs_set_relative(first_child,VFS_RELATIVE_PREV_SIBLING,node);
	}
	out&=vfs_set_relative(node,VFS_RELATIVE_NEXT_SIBLING,first_child);
	out&=vfs_set_relative(node_parent,VFS_RELATIVE_FIRST_CHILD,node);
	return (out?node:NULL);
}



vfs_node_t* KERNEL_CORE_CODE vfs_get_relative(vfs_node_t* node,u8 relative){
	if (!node){
		return NULL;
	}
	vfs_node_id_t* id;
	switch (relative){
		case VFS_RELATIVE_PARENT:
			if (node->flags&VFS_NODE_FLAG_ROOT){
				return node;
			}
			id=&(node->parent);
			break;
		case VFS_RELATIVE_PREV_SIBLING:
			id=&(node->prev_sibling);
			break;
		case VFS_RELATIVE_NEXT_SIBLING:
			id=&(node->next_sibling);
			break;
		case VFS_RELATIVE_FIRST_CHILD:
			if (node->type!=VFS_NODE_TYPE_DIRECTORY){
				return NULL;
			}
			id=&(node->first_child);
			break;
		default:
			return NULL;
	}
	if (*id==VFS_NODE_ID_EMPTY){
		return NULL;
	}
	partition_t* fs=partition_get(node->vfs_index);
	vfs_node_t* out=vfs_allocator_get(&(fs->allocator),*id,0);
	if (!out){
		lock_acquire_exclusive(&(fs->lock));
		out=fs->config->get_relative(fs,node,relative);
		*id=(out?out->id:VFS_NODE_ID_EMPTY);
		lock_release_exclusive(&(fs->lock));
	}
	return out;
}



_Bool vfs_set_relative(vfs_node_t* node,u8 relative,vfs_node_t* other){
	if (!node){
		return 0;
	}
	partition_t* fs=partition_get(node->vfs_index);
	vfs_node_id_t* id;
	switch (relative){
		case VFS_RELATIVE_PARENT:
			id=&(node->parent);
			break;
		case VFS_RELATIVE_PREV_SIBLING:
			id=&(node->prev_sibling);
			break;
		case VFS_RELATIVE_NEXT_SIBLING:
			id=&(node->next_sibling);
			break;
		case VFS_RELATIVE_FIRST_CHILD:
			if (node->type!=VFS_NODE_TYPE_DIRECTORY){
				return 0;
			}
			id=&(node->first_child);
			break;
		default:
			return 0;
	}
	vfs_node_id_t other_id=(other?other->id:VFS_NODE_ID_EMPTY);
	if (*id==other_id){
		return 1;
	}
	lock_acquire_exclusive(&(fs->lock));
	_Bool out=fs->config->set_relative(fs,node,relative,other);
	if (out){
		*id=other_id;
	}
	lock_release_exclusive(&(fs->lock));
	return out;
}



_Bool vfs_move(vfs_node_t* src_node,vfs_node_t* dst_node){
	if (!src_node||!dst_node||src_node->vfs_index!=dst_node->vfs_index||src_node->type!=dst_node->type){
		return 0;
	}
	partition_t* fs=partition_get(src_node->vfs_index);
	_Bool out=1;
	if (src_node->type==VFS_NODE_TYPE_DIRECTORY){
		vfs_node_t* child=vfs_get_relative(src_node,VFS_RELATIVE_FIRST_CHILD);
		out&=vfs_set_relative(src_node,VFS_RELATIVE_FIRST_CHILD,NULL);
		out&=vfs_set_relative(dst_node,VFS_RELATIVE_FIRST_CHILD,child);
		while (child){
			out&=vfs_set_relative(child,VFS_RELATIVE_PARENT,dst_node);
			child=vfs_get_relative(child,VFS_RELATIVE_NEXT_SIBLING);
		}
	}
	else{
		lock_acquire_exclusive(&(fs->lock));
		out=fs->config->move_file(fs,src_node,dst_node);
		lock_release_exclusive(&(fs->lock));
	}
	return out;
}



_Bool vfs_delete(vfs_node_t* node){
	if (node->type==VFS_NODE_TYPE_DIRECTORY&&vfs_get_relative(node,VFS_RELATIVE_FIRST_CHILD)){
		return 0;
	}
	vfs_node_t* prev=vfs_get_relative(node,VFS_RELATIVE_PREV_SIBLING);
	vfs_node_t* next=vfs_get_relative(node,VFS_RELATIVE_NEXT_SIBLING);
	_Bool out=1;
	if (prev){
		out&=vfs_set_relative(prev,VFS_RELATIVE_NEXT_SIBLING,next);
	}
	else{
		out&=vfs_set_relative(vfs_get_relative(node,VFS_RELATIVE_PARENT),VFS_RELATIVE_FIRST_CHILD,next);
	}
	if (next){
		out&=vfs_set_relative(next,VFS_RELATIVE_PREV_SIBLING,prev);
	}
	if (out){
		out=vfs_dealloc(node);
	}
	return out;
}



vfs_node_t* KERNEL_CORE_CODE vfs_get_child(vfs_node_t* parent,const char* name,u8 name_length){
	if (!parent||parent->type!=VFS_NODE_TYPE_DIRECTORY){
		return NULL;
	}
	for (vfs_node_t* child=vfs_get_relative(parent,VFS_RELATIVE_FIRST_CHILD);child;child=vfs_get_relative(child,VFS_RELATIVE_NEXT_SIBLING)){
		if (child->name_length!=name_length){
			continue;
		}
		for (u8 i=0;i<name_length;i++){
			if (child->name[i]!=name[i]){
				goto _next_child;
			}
		}
		return child;
_next_child:
	}
	return NULL;
}



u64 KERNEL_CORE_CODE vfs_read(vfs_node_t* node,u64 offset,void* buffer,u64 count){
	if (node->type!=VFS_NODE_TYPE_FILE||!count){
		return 0;
	}
	partition_t* fs=partition_get(node->vfs_index);
	if (!(fs->config->flags&PARTITION_FILE_SYSTEM_CONFIG_FLAG_ALIGNED_IO)){
		lock_acquire_exclusive(&(fs->lock));
		u64 out=fs->config->read(fs,node,offset,buffer,count);
		lock_release_exclusive(&(fs->lock));
		return out;
	}
	u64 out=0;
	u16 extra=offset&(fs->drive->block_size-1);
	lock_acquire_exclusive(&(fs->lock));
	if (extra){
		u8 chunk[4096];
		if (fs->drive->block_size>4096){
			panic("Unimplemented");
		}
		u64 chunk_length=fs->config->read(fs,node,offset-extra,chunk,fs->drive->block_size);
		if (chunk_length<fs->drive->block_size){
			lock_release_exclusive(&(fs->lock));
			return 0;
		}
		extra=fs->drive->block_size-extra;
		if (extra>count){
			extra=count;
		}
		memcpy(buffer,chunk+(offset&(fs->drive->block_size-1)),extra);
		offset+=extra;
		buffer+=extra;
		count-=extra;
		out+=extra;
	}
	extra=count&(fs->drive->block_size-1);
	if (count-extra){
		out+=fs->config->read(fs,node,offset,buffer,count-extra);
	}
	if (extra){
		u8 chunk[4096];
		if (fs->drive->block_size>4096){
			panic("Unimplemented");
		}
		u64 chunk_length=fs->config->read(fs,node,offset+count-extra,chunk,fs->drive->block_size);
		if (chunk_length>extra){
			chunk_length=extra;
		}
		memcpy(buffer+count-extra,chunk,chunk_length);
		out+=chunk_length;
	}
	lock_release_exclusive(&(fs->lock));
	return out;
}



u64 vfs_write(vfs_node_t* node,u64 offset,const void* buffer,u64 count){
	if (node->type!=VFS_NODE_TYPE_FILE||!count){
		return 0;
	}
	partition_t* fs=partition_get(node->vfs_index);
	if (!(fs->config->flags&PARTITION_FILE_SYSTEM_CONFIG_FLAG_ALIGNED_IO)){
		lock_acquire_exclusive(&(fs->lock));
		u64 out=fs->config->write(fs,node,offset,buffer,count);
		lock_release_exclusive(&(fs->lock));
		return out;
	}
	u64 out=0;
	u16 extra=offset&(fs->drive->block_size-1);
	lock_acquire_exclusive(&(fs->lock));
	if (extra){
		panic("Unimplemented: vfs_write.offset_extra");
	}
	extra=count&(fs->drive->block_size-1);
	if (count-extra){
		out+=fs->config->write(fs,node,offset,buffer,count-extra);
	}
	if (extra){
		u8 chunk[4096];
		if (fs->drive->block_size>4096){
			panic("Unimplemented");
		}
		u64 chunk_length=fs->config->read(fs,node,offset+count-extra,chunk,fs->drive->block_size);
		if (chunk_length>extra){
			chunk_length=extra;
		}
		memcpy(chunk,buffer+count-extra,chunk_length);
		chunk_length=fs->config->write(fs,node,offset+count-extra,chunk,fs->drive->block_size);
		if (chunk_length>extra){
			chunk_length=extra;
		}
		out+=chunk_length;
	}
	lock_release_exclusive(&(fs->lock));
	return out;
}



u64 vfs_get_size(vfs_node_t* node){
	partition_t* fs=partition_get(node->vfs_index);
	lock_acquire_exclusive(&(fs->lock));
	u64 out=fs->config->get_size(fs,node);
	lock_release_exclusive(&(fs->lock));
	return out;
}



_Bool vfs_set_size(vfs_node_t* node,u64 size){
	partition_t* fs=partition_get(node->vfs_index);
	lock_acquire_exclusive(&(fs->lock));
	_Bool out=fs->config->set_size(fs,node,size);
	lock_release_exclusive(&(fs->lock));
	return out;
}



u32 vfs_get_full_path(vfs_node_t* node,char* buffer,u32 buffer_length){
	if (!buffer_length){
		return 0;
	}
	u32 i=buffer_length-1;
	buffer[i]=0;
	for (;1;node=vfs_get_relative(node,VFS_RELATIVE_PARENT)){
		if (i<node->name_length){
			return 0;
		}
		i-=node->name_length;
		memcpy(buffer+i,node->name,node->name_length);
		if (node->flags&VFS_NODE_FLAG_ROOT){
			break;
		}
		if (!i){
			return 0;
		}
		i--;
		buffer[i]='/';
	}
	if (i==buffer_length-1){
		i--;
		buffer[i]='/';
	}
	if (node->vfs_index!=partition_boot->index){
		const partition_t* partition=partition_get(node->vfs_index);
		if (i<partition->name_length+1){
			return 0;
		}
		i--;
		buffer[i]=':';
		i-=partition->name_length;
		memcpy(buffer+i,partition->name,partition->name_length);
	}
	if (!i){
		return buffer_length-2;
	}
	for (u32 j=0;j<buffer_length-i;j++){
		buffer[j]=buffer[i+j];
	}
	return buffer_length-i-1;
}
