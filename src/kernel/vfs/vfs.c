#include <kernel/error/error.h>
#include <kernel/fs/fs.h>
#include <kernel/id/flags.h>
#include <kernel/id/group.h>
#include <kernel/id/user.h>
#include <kernel/lock/rwlock.h>
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



static bool _has_read_permissions(vfs_node_t* node,u32 flags,uid_t uid,gid_t gid){
	if (!(flags&VFS_LOOKUP_FLAG_CHECK_PERMISSIONS)){
		return 1;
	}
	return !!(vfs_permissions_get(node,uid,gid)&VFS_PERMISSION_READ);
}



KERNEL_PUBLIC error_t vfs_mount(filesystem_t* fs,const char* path,bool user_mode){
	if (fs->is_mounted){
		if (user_mode){
			return ERROR_ALREADY_MOUNTED;
		}
		panic("vfs_mount: filesystem is already mounted");
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
			vfs_node_unref(_vfs_root_node);
		}
		vfs_node_ref(fs->root);
		_vfs_root_node=fs->root;
		rwlock_acquire_write(&(_vfs_root_node->lock));
		_vfs_root_node->relatives.parent=NULL;
		rwlock_release_write(&(_vfs_root_node->lock));
		vfs_node_t* old_root=process_kernel->vfs_root;
		vfs_node_t* old_cwd=process_kernel->vfs_cwd;
		vfs_node_ref(_vfs_root_node);
		vfs_node_ref(_vfs_root_node);
		process_kernel->vfs_root=_vfs_root_node;
		process_kernel->vfs_cwd=_vfs_root_node;
		if (old_root){
			vfs_node_unref(old_root);
		}
		if (old_cwd){
			vfs_node_unref(old_cwd);
		}
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
	if (!parent||!child_name){
		if (user_mode){
			return ERROR_NOT_FOUND;
		}
		panic("vfs_mount: path does not exist");
	}
	rwlock_acquire_write(&(fs->root->lock));
	smm_dealloc(fs->root->name);
	fs->root->name=smm_alloc(child_name,0);
	rwlock_release_write(&(fs->root->lock));
	vfs_node_attach_child(parent,fs->root);
	if (parent){
		vfs_node_unref(parent);
	}
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
	vfs_node_ref(base_root_node);
	if ((flags&VFS_LOOKUP_FLAG_CHECK_PERMISSIONS)&&((uid_get_flags(uid)|gid_get_flags(gid))&ID_FLAG_BYPASS_VFS_PERMISSIONS)){
		flags&=~VFS_LOOKUP_FLAG_CHECK_PERMISSIONS;
	}
	if (path[0]=='/'){
		root=base_root_node;
	}
	else if (!root){
		root=(THREAD_DATA->header.current_thread?THREAD_DATA->process->vfs_cwd:base_root_node);
	}
	vfs_node_ref(root);
	if (parent){
		*parent=NULL;
		*child_name=NULL;
	}
	vfs_node_t* out=NULL;
	while (root&&path[0]){
		if ((flags&(VFS_LOOKUP_FLAG_FOLLOW_LINKS|VFS_LOOKUP_FLAG_FIND_LINKS))&&(root->flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_LINK){
			if (flags&VFS_LOOKUP_FLAG_FIND_LINKS){
				out=VFS_LOOKUP_LINK_FOUND;
				goto _cleanup;
			}
			if (!_has_read_permissions(root,flags,uid,gid)){
				goto _cleanup;
			}
			char buffer[4096];
			buffer[vfs_node_read(root,0,buffer,4095,0)]=0;
			if (!buffer[0]){
				goto _cleanup;
			}
			vfs_node_t* new_root=vfs_lookup_for_creation(root->relatives.parent,buffer,flags,uid,gid,NULL,NULL);
			if (!new_root){
				goto _cleanup;
			}
			vfs_node_unref(root);
			root=new_root;
		}
		if (path[0]=='/'){
			path++;
			continue;
		}
		u64 i=0;
		for (;path[i]&&path[i]!='/';i++){
			if (i>=SMM_MAX_LENGTH){
				goto _cleanup;
			}
		}
		if (i==1&&path[0]=='.'){
			path+=1;
			continue;
		}
		if (i==2&&path[0]=='.'&&path[1]=='.'){
			if (root!=base_root_node&&root->relatives.parent){
				vfs_node_t* new_root=root->relatives.parent;
				vfs_node_ref(new_root);
				vfs_node_unref(root);
				root=new_root;
			}
			path+=2;
			continue;
		}
		if (!_has_read_permissions(root,flags,uid,gid)){
			goto _cleanup;
		}
		string_t* name=smm_alloc(path,i);
		vfs_node_t* child=vfs_node_lookup(root,name);
		smm_dealloc(name);
		path+=i;
		if (!child&&parent){
			if (!path[0]){
				*parent=root;
				*child_name=path-i;
			}
			goto _cleanup;
		}
		if (child==base_root_node){ // parent is its own descendant
			goto _cleanup;
		}
		vfs_node_unref(root);
		root=child;
	}
	if (root&&(flags&(VFS_LOOKUP_FLAG_FOLLOW_LINKS|VFS_LOOKUP_FLAG_FIND_LINKS))&&(root->flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_LINK){
		if (flags&VFS_LOOKUP_FLAG_FIND_LINKS){
			out=VFS_LOOKUP_LINK_FOUND;
			goto _cleanup;
		}
		if (!_has_read_permissions(root,flags,uid,gid)){
			goto _cleanup;
		}
		char buffer[4096];
		buffer[vfs_node_read(root,0,buffer,4095,0)]=0;
		if (!buffer[0]){
			goto _cleanup;
		}
		vfs_node_t* new_root=vfs_lookup_for_creation(root->relatives.parent,buffer,flags,uid,gid,NULL,NULL);
		if (!new_root){
			goto _cleanup;
		}
		vfs_node_unref(root);
		root=new_root;
	}
	vfs_node_unref(base_root_node);
	return root;
_cleanup:
	vfs_node_unref(base_root_node);
	vfs_node_unref(root);
	return out;
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
	if (_vfs_root_node){
		vfs_node_ref(_vfs_root_node);
	}
	return _vfs_root_node;
}
