#include <kernel/error/error.h>
#include <kernel/fs/fs.h>
#include <kernel/id/flags.h>
#include <kernel/id/group.h>
#include <kernel/id/user.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/smm.h>
#include <kernel/mp/thread.h>
#include <kernel/mp/process.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/permissions.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "vfs"



static vfs_node_t* _vfs_root_node=NULL;



static _Bool _has_read_permissions(vfs_node_t* node,u32 flags,uid_t uid,gid_t gid){
	if (!(flags&VFS_LOOKUP_FLAG_CHECK_PERMISSIONS)){
		return 1;
	}
	return !!(vfs_permissions_get(node,uid,gid)&VFS_PERMISSION_EXEC);
}



KERNEL_PUBLIC error_t vfs_mount(filesystem_t* fs,const char* path,_Bool user_mode){
	if (fs->is_mounted){
		if (user_mode){
			return ERROR_ALREADY_MOUNTED;
		}
		panic("Filesystem is already mounted");
	}
	if (!path){
		if (user_mode){
			return ERROR_DENIED;
		}
		if (_vfs_root_node){
			_vfs_root_node->fs->is_mounted=0;
			if (_vfs_root_node->fs->descriptor->config->mount_callback){
				_vfs_root_node->fs->descriptor->config->mount_callback(_vfs_root_node->fs,NULL);
			}
		}
		_vfs_root_node=fs->root;
		spinlock_acquire_exclusive(&(_vfs_root_node->lock));
		_vfs_root_node->relatives.parent=NULL;
		spinlock_release_exclusive(&(_vfs_root_node->lock));
		process_kernel->vfs_root=_vfs_root_node;
		process_kernel->vfs_cwd=_vfs_root_node;
		fs->is_mounted=1;
		if (fs->descriptor->config->mount_callback){
			fs->descriptor->config->mount_callback(fs,"/");
		}
		return ERROR_OK;
	}
	vfs_node_t* parent;
	const char* child_name;
	if (vfs_lookup_for_creation(NULL,path,0,0,0,&parent,&child_name)){
		if (user_mode){
			return ERROR_ALREADY_PRESENT;
		}
		panic("vfs_mount: node already exists");
	}
	spinlock_acquire_exclusive(&(fs->root->lock));
	smm_dealloc(fs->root->name);
	fs->root->name=smm_alloc(child_name,0);
	spinlock_release_exclusive(&(fs->root->lock));
	vfs_node_attach_external_child(parent,fs->root);
	fs->is_mounted=1;
	if (fs->descriptor->config->mount_callback){
		fs->descriptor->config->mount_callback(fs,path);
	}
	return ERROR_OK;
}



KERNEL_PUBLIC vfs_node_t* vfs_lookup(vfs_node_t* root,const char* path,u32 flags,uid_t uid,gid_t gid){
	return vfs_lookup_for_creation(root,path,flags,uid,gid,NULL,NULL);
}



KERNEL_PUBLIC vfs_node_t* vfs_lookup_for_creation(vfs_node_t* root,const char* path,u32 flags,uid_t uid,gid_t gid,vfs_node_t** parent,const char** child_name){
	vfs_node_t* base_root_node=(THREAD_DATA->header.current_thread?THREAD_DATA->process->vfs_root:_vfs_root_node);
	if ((flags&VFS_LOOKUP_FLAG_CHECK_PERMISSIONS)&&((uid_get_flags(uid)|gid_get_flags(gid))&ID_FLAG_BYPASS_VFS_PERMISSIONS)){
		flags&=~VFS_LOOKUP_FLAG_CHECK_PERMISSIONS;
	}
	if (path[0]=='/'){
		root=base_root_node;
	}
	else if (!root){
		root=(THREAD_DATA->header.current_thread?THREAD_DATA->process->vfs_cwd:_vfs_root_node);
	}
	if (parent){
		*parent=NULL;
		*child_name=NULL;
	}
	while (root&&path[0]){
		if ((flags&VFS_LOOKUP_FLAG_FOLLOW_LINKS)&&(root->flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_LINK){
			if (!_has_read_permissions(root,flags,uid,gid)){
				return NULL;
			}
			char buffer[4096];
			buffer[vfs_node_read(root,0,buffer,4095,0)]=0;
			if (!buffer[0]){
				return NULL;
			}
			root=vfs_lookup_for_creation(root->relatives.parent,buffer,flags,uid,gid,NULL,NULL);
			if (!root){
				return NULL;
			}
		}
		if (path[0]=='/'){
			path++;
			continue;
		}
		u64 i=0;
		for (;path[i]&&path[i]!='/';i++){
			if (i>=SMM_MAX_LENGTH){
				return NULL;
			}
		}
		if (i==1&&path[0]=='.'){
			path+=1;
			continue;
		}
		if (i==2&&path[0]=='.'&&path[1]=='.'){
			if (root!=base_root_node&&root->relatives.parent){
				root=root->relatives.parent;
			}
			path+=2;
			continue;
		}
		if (!_has_read_permissions(root,flags,uid,gid)){
			return NULL;
		}
		SMM_TEMPORARY_STRING name=smm_alloc(path,i);
		vfs_node_t* child=vfs_node_lookup(root,name);
		path+=i;
		if (!child&&parent){
			if (!path[0]){
				*parent=root;
				*child_name=path-i;
			}
			return NULL;
		}
		root=child;
	}
	if (root&&(flags&VFS_LOOKUP_FLAG_FOLLOW_LINKS)&&(root->flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_LINK){
		if (!_has_read_permissions(root,flags,uid,gid)){
			return NULL;
		}
		char buffer[4096];
		buffer[vfs_node_read(root,0,buffer,4095,0)]=0;
		if (!buffer[0]){
			return NULL;
		}
		root=vfs_lookup_for_creation(root->relatives.parent,buffer,flags,uid,gid,NULL,NULL);
		if (!root){
			return NULL;
		}
	}
	return root;
}



KERNEL_PUBLIC u32 vfs_path(vfs_node_t* node,char* buffer,u32 buffer_length){
	vfs_node_t* base_root_node=(THREAD_DATA->header.current_thread?THREAD_DATA->process->vfs_root:_vfs_root_node);
	u32 i=buffer_length;
	for (;node&&node!=base_root_node;node=node->relatives.parent){
		if (i<node->name->length+1){
			return 0;
		}
		i-=node->name->length+1;
		buffer[i]='/';
		mem_copy(buffer+i+1,node->name->data,node->name->length);
	}
	if (!i){
		return 0;
	}
	if (i==buffer_length){
		i--;
		buffer[i]='/';
	}
	for (u32 j=0;j<buffer_length-i;j++){
		buffer[j]=buffer[i+j];
	}
	buffer[buffer_length-i]=0;
	return buffer_length-i;
}



KERNEL_PUBLIC vfs_node_t* vfs_get_root_node(void){
	return _vfs_root_node;
}
