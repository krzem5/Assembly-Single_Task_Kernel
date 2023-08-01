#include <kernel/drive/drive.h>
#include <kernel/drive/drive_list.h>
#include <kernel/fs/fs.h>
#include <kernel/fs/node_allocator.h>
#include <kernel/fs/partition.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/memcpy.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "fs"



static fs_file_system_t* KERNEL_CORE_DATA _fs_file_systems;
static u8 KERNEL_CORE_DATA _fs_file_system_count;
static u8 KERNEL_CORE_DATA _fs_root_file_systems_index;



static fs_node_t* KERNEL_CORE_CODE _alloc_node(fs_file_system_t* fs,const char* name,u8 name_length){
	if (name_length>64){
		name_length=64;
		ERROR_CORE("fs_node_t.name_length too large");
	}
	fs_node_t* out=fs_node_allocator_get(&(fs->allocator),FS_NODE_ID_EMPTY,1);
	out->type=FS_NODE_TYPE_FILE;
	out->fs_index=fs-_fs_file_systems;
	out->name_length=name_length;
	out->flags=0;
	for (u8 i=0;i<name_length;i++){
		out->name[i]=name[i];
	}
	out->name[name_length]=0;
	out->parent=FS_NODE_ID_UNKNOWN;
	out->prev_sibling=FS_NODE_ID_UNKNOWN;
	out->next_sibling=FS_NODE_ID_UNKNOWN;
	out->first_child=FS_NODE_ID_UNKNOWN;
	return out;
}



void KERNEL_CORE_CODE fs_init(void){
	LOG_CORE("Initializing file system...");
	_fs_file_systems=VMM_TRANSLATE_ADDRESS(pmm_alloc(pmm_align_up_address(FS_MAX_FILE_SYSTEMS*sizeof(fs_file_system_t))>>PAGE_SIZE_SHIFT,PMM_COUNTER_FS));
	_fs_file_system_count=0;
	_fs_root_file_systems_index=FS_INVALID_FILE_SYSTEM_INDEX;
}



void* KERNEL_CORE_CODE fs_create_file_system(const drive_t* drive,const fs_partition_config_t* partition_config,const fs_file_system_config_t* config,void* extra_data){
	if (_fs_file_system_count>=FS_MAX_FILE_SYSTEMS){
		ERROR_CORE("Too many file systems!");
		return NULL;
	}
	fs_file_system_t* fs=_fs_file_systems+_fs_file_system_count;
	_fs_file_system_count++;
	lock_init(&(fs->lock));
	fs->config=config;
	fs->partition_config=*partition_config;
	fs->index=_fs_file_system_count-1;
	u8 i=0;
	while (drive->name[i]){
		fs->name[i]=drive->name[i];
		i++;
	}
	if (partition_config->type==FS_PARTITION_TYPE_DRIVE){
		fs->name[i]=0;
		fs->name_length=i;
	}
	else if (partition_config->index<10){
		fs->name[i]='p';
		fs->name[i+1]=partition_config->index+48;
		fs->name[i+2]=0;
		fs->name_length=i+2;
	}
	else if (partition_config->index<100){
		fs->name[i]='p';
		fs->name[i+1]=partition_config->index/10+48;
		fs->name[i+2]=(partition_config->index%10)+48;
		fs->name[i+3]=0;
		fs->name_length=i+3;
	}
	else{
		fs->name[i]='p';
		fs->name[i+1]=partition_config->index/100+48;
		fs->name[i+2]=((partition_config->index/10)%10)+48;
		fs->name[i+3]=(partition_config->index%10)+48;
		fs->name[i+4]=0;
		fs->name_length=i+4;
	}
	fs->drive=drive;
	fs->extra_data=extra_data;
	fs_node_allocator_init(_fs_file_system_count-1,config->node_size,&(fs->allocator));
	LOG_CORE("Created file system '%s' from drive '%s'",fs->name,drive->model_number);
	fs->root=_alloc_node(fs,"",0);
	fs->root->type=FS_NODE_TYPE_DIRECTORY;
	fs->root->flags|=FS_NODE_FLAG_ROOT;
	fs->root->parent=fs->root->id;
	fs->root->prev_sibling=fs->root->id;
	fs->root->next_sibling=fs->root->id;
	return fs->root;
}



u8 fs_get_file_system_count(void){
	return _fs_file_system_count;
}



const fs_file_system_t* KERNEL_CORE_CODE fs_get_file_system(u8 fs_index){
	return (fs_index<_fs_file_system_count?_fs_file_systems+fs_index:NULL);
}



u8 fs_get_boot_file_system(void){
	return _fs_root_file_systems_index;
}



void KERNEL_CORE_CODE fs_set_boot_file_system(u8 fs_index){
	if (_fs_root_file_systems_index!=FS_INVALID_FILE_SYSTEM_INDEX){
		WARN_CORE("fs_set_boot_file_system called more than once");
		return;
	}
	_fs_root_file_systems_index=fs_index;
	drive_list_set_boot_drive((_fs_file_systems+fs_index)->drive->index);
}



void* KERNEL_CORE_CODE fs_alloc_node(u8 fs_index,const char* name,u8 name_length){
	return _alloc_node(_fs_file_systems+fs_index,name,name_length);
}



_Bool fs_dealloc_node(fs_node_t* node){
	if (!node){
		return 1;
	}
	fs_file_system_t* fs=_fs_file_systems+node->fs_index;
	lock_acquire(&(fs->lock));
	_Bool out=fs->config->delete(fs,node);
	if (out){
		node->type=FS_NODE_TYPE_INVALID;
	}
	lock_release(&(fs->lock));
	return out;
}



fs_node_t* fs_get_node_by_id(fs_node_id_t id){
	return fs_node_allocator_get(&((_fs_file_systems+(id>>56))->allocator),id,0);
}



fs_node_t* KERNEL_CORE_CODE fs_get_node(fs_node_t* root,const char* path,u8 type){
	if (path[0]=='/'){
		if (_fs_root_file_systems_index==FS_INVALID_FILE_SYSTEM_INDEX){
			ERROR_CORE("Root file system not located yet; partition must be specified");
			return NULL;
		}
		root=(_fs_file_systems+_fs_root_file_systems_index)->root;
	}
	else{
		u16 partition_name_length=0;
		while (path[partition_name_length]&&path[partition_name_length]!=':'&&path[partition_name_length]!='/'){
			partition_name_length++;
		}
		if (path[partition_name_length]==':'){
			root=NULL;
			fs_file_system_t* fs=_fs_file_systems;
			for (u8 i=0;i<_fs_file_system_count;i++){
				if (fs->name_length!=partition_name_length){
					goto _check_next_fs;
				}
				for (u8 j=0;j<partition_name_length;j++){
					if (fs->name[j]!=path[j]){
						goto _check_next_fs;
					}
				}
				root=fs->root;
				break;
_check_next_fs:
				fs++;
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
	fs_node_t* node_parent=root;
	fs_node_t* node=root;
	while (path[0]){
		if (path[0]=='/'){
			path++;
			continue;
		}
		u8 i=1;
		while (path[i]&&path[i]!='/'){
			i++;
		}
		if (!node||node->type!=FS_NODE_TYPE_DIRECTORY){
			return NULL;
		}
		if (i==1&&path[0]=='.'){
			path++;
			continue;
		}
		if (i==2&&path[0]=='.'&&path[1]=='.'){
			path+=2;
			if (!(node->flags&FS_NODE_FLAG_ROOT)){
				node=fs_get_node_relative(node,FS_NODE_RELATIVE_PARENT);
			}
			continue;
		}
		name=path;
		name_length=i;
		node_parent=node;
		node=fs_get_node_child(node,path,i);
		path+=i;
	}
	if (node||type==FS_NODE_TYPE_INVALID||!name_length){
		return node;
	}
	fs_file_system_t* fs=_fs_file_systems+node_parent->fs_index;
	lock_release(&(fs->lock));
	node=fs->config->create(fs,type==FS_NODE_TYPE_DIRECTORY,name,name_length);
	lock_release(&(fs->lock));
	if (!node){
		return NULL;
	}
	fs_node_t* first_child=fs_get_node_relative(node_parent,FS_NODE_RELATIVE_FIRST_CHILD);
	_Bool out=fs_set_node_relative(node,FS_NODE_RELATIVE_PARENT,node_parent);
	if (first_child){
		out&=fs_set_node_relative(first_child,FS_NODE_RELATIVE_PREV_SIBLING,node);
	}
	out&=fs_set_node_relative(node,FS_NODE_RELATIVE_NEXT_SIBLING,first_child);
	out&=fs_set_node_relative(node_parent,FS_NODE_RELATIVE_FIRST_CHILD,node);
	return (out?node:NULL);
}



fs_node_t* KERNEL_CORE_CODE fs_get_node_relative(fs_node_t* node,u8 relative){
	if (!node){
		return NULL;
	}
	fs_node_id_t* id;
	switch (relative){
		case FS_NODE_RELATIVE_PARENT:
			if (node->flags&FS_NODE_FLAG_ROOT){
				return node;
			}
			id=&(node->parent);
			break;
		case FS_NODE_RELATIVE_PREV_SIBLING:
			id=&(node->prev_sibling);
			break;
		case FS_NODE_RELATIVE_NEXT_SIBLING:
			id=&(node->next_sibling);
			break;
		case FS_NODE_RELATIVE_FIRST_CHILD:
			if (node->type!=FS_NODE_TYPE_DIRECTORY){
				return NULL;
			}
			id=&(node->first_child);
			break;
		default:
			return NULL;
	}
	if (*id==FS_NODE_ID_EMPTY){
		return NULL;
	}
	fs_file_system_t* fs=_fs_file_systems+node->fs_index;
	fs_node_t* out=fs_node_allocator_get(&(fs->allocator),*id,0);
	if (!out){
		lock_acquire(&(fs->lock));
		out=fs->config->get_relative(fs,node,relative);
		*id=(out?out->id:FS_NODE_ID_EMPTY);
		lock_release(&(fs->lock));
	}
	return out;
}



_Bool fs_set_node_relative(fs_node_t* node,u8 relative,fs_node_t* other){
	if (!node){
		return 0;
	}
	fs_file_system_t* fs=_fs_file_systems+node->fs_index;
	fs_node_id_t* id;
	switch (relative){
		case FS_NODE_RELATIVE_PARENT:
			id=&(node->parent);
			break;
		case FS_NODE_RELATIVE_PREV_SIBLING:
			id=&(node->prev_sibling);
			break;
		case FS_NODE_RELATIVE_NEXT_SIBLING:
			id=&(node->next_sibling);
			break;
		case FS_NODE_RELATIVE_FIRST_CHILD:
			if (node->type!=FS_NODE_TYPE_DIRECTORY){
				return 0;
			}
			id=&(node->first_child);
			break;
		default:
			return 0;
	}
	fs_node_id_t other_id=(other?other->id:FS_NODE_ID_EMPTY);
	if (*id==other_id){
		return 1;
	}
	lock_acquire(&(fs->lock));
	_Bool out=fs->config->set_relative(fs,node,relative,other);
	if (out){
		*id=other_id;
	}
	lock_release(&(fs->lock));
	return out;
}



_Bool fs_move_node(fs_node_t* src_node,fs_node_t* dst_node){
	if (!src_node||!dst_node||src_node->fs_index!=dst_node->fs_index||src_node->type!=dst_node->type){
		return 0;
	}
	fs_file_system_t* fs=_fs_file_systems+src_node->fs_index;
	_Bool out=1;
	if (src_node->type==FS_NODE_TYPE_DIRECTORY){
		fs_node_t* child=fs_get_node_relative(src_node,FS_NODE_RELATIVE_FIRST_CHILD);
		out&=fs_set_node_relative(src_node,FS_NODE_RELATIVE_FIRST_CHILD,NULL);
		out&=fs_set_node_relative(dst_node,FS_NODE_RELATIVE_FIRST_CHILD,child);
		while (child){
			out&=fs_set_node_relative(child,FS_NODE_RELATIVE_PARENT,dst_node);
			child=fs_get_node_relative(child,FS_NODE_RELATIVE_NEXT_SIBLING);
		}
	}
	else{
		lock_acquire(&(fs->lock));
		out=fs->config->move_file(fs,src_node,dst_node);
		lock_release(&(fs->lock));
	}
	return out;
}



fs_node_t* KERNEL_CORE_CODE fs_get_node_child(fs_node_t* parent,const char* name,u8 name_length){
	if (!parent||parent->type!=FS_NODE_TYPE_DIRECTORY){
		return NULL;
	}
	for (fs_node_t* child=fs_get_node_relative(parent,FS_NODE_RELATIVE_FIRST_CHILD);child;child=fs_get_node_relative(child,FS_NODE_RELATIVE_NEXT_SIBLING)){
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



u64 KERNEL_CORE_CODE fs_read(fs_node_t* node,u64 offset,void* buffer,u64 count){
	if (node->type!=FS_NODE_TYPE_FILE||!count){
		return 0;
	}
	fs_file_system_t* fs=_fs_file_systems+node->fs_index;
	if (!(fs->config->flags&FS_FILE_SYSTEM_CONFIG_FLAG_ALIGNED_IO)){
		lock_acquire(&(fs->lock));
		u64 out=fs->config->read(fs,node,offset,buffer,count);
		lock_release(&(fs->lock));
		return out;
	}
	u64 out=0;
	u16 extra=offset&(fs->drive->block_size-1);
	lock_acquire(&(fs->lock));
	if (extra){
		u8 chunk[4096];
		if (fs->drive->block_size>4096){
			ERROR_CORE("Unimplemented");
			return 0;
		}
		u64 chunk_length=fs->config->read(fs,node,offset-extra,chunk,fs->drive->block_size);
		if (chunk_length<fs->drive->block_size){
			lock_release(&(fs->lock));
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
			ERROR_CORE("Unimplemented");
			return 0;
		}
		u64 chunk_length=fs->config->read(fs,node,offset+count-extra,chunk,fs->drive->block_size);
		if (chunk_length>extra){
			chunk_length=extra;
		}
		memcpy(buffer+count-extra,chunk,chunk_length);
		out+=chunk_length;
	}
	lock_release(&(fs->lock));
	return out;
}



u64 fs_write(fs_node_t* node,u64 offset,const void* buffer,u64 count){
	if (node->type!=FS_NODE_TYPE_FILE||!count){
		return 0;
	}
	fs_file_system_t* fs=_fs_file_systems+node->fs_index;
	if (!(fs->config->flags&FS_FILE_SYSTEM_CONFIG_FLAG_ALIGNED_IO)){
		lock_acquire(&(fs->lock));
		u64 out=fs->config->write(fs,node,offset,buffer,count);
		lock_release(&(fs->lock));
		return out;
	}
	WARN("Unimplemented: fs_write");
	return 0;
}



u64 fs_get_size(fs_node_t* node){
	fs_file_system_t* fs=_fs_file_systems+node->fs_index;
	lock_acquire(&(fs->lock));
	u64 out=fs->config->get_size(fs,node);
	lock_release(&(fs->lock));
	return out;
}



void fs_flush_cache(void){
	LOG("Flushing file system cache...");
	for (u8 i=0;i<_fs_file_system_count;i++){
		fs_file_system_t* fs=_fs_file_systems+i;
		lock_acquire(&(fs->lock));
		fs->config->flush_cache(fs);
		lock_release(&(fs->lock));
	}
}
