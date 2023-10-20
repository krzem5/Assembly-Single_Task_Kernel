#include <kernel/fs/fs.h>
#include <kernel/lock/lock.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/name.h>
#include <kernel/vfs/node.h>



static vfs_node_t* _vfs_root_node;



void vfs_mount(filesystem_t* fs,const char* path){
	if (!path){
		if (_vfs_root_node){
			panic("Root filesystem already registered");
		}
		_vfs_root_node=fs->root;
		lock_acquire_exclusive(&(fs->root->lock));
		vfs_name_dealloc(fs->root->name);
		fs->root->name=vfs_name_alloc("",0);
		lock_release_exclusive(&(fs->root->lock));
		return;
	}
	panic("vfs_mount");
}



vfs_node_t* vfs_lookup(vfs_node_t* root,const char* path){
	if (!root){
		root=_vfs_root_node;
	}
	while (root&&path[0]){
		if (path[0]=='/'){
			path++;
			continue;
		}
		u64 i=0;
		for (;path[i]&&path[i]!='/';i++){
			if (i>=VFS_NAME_MAX_LENGTH){
				return NULL;
			}
		}
		if (i==1&&path[0]=='.'){
			path+=i;
			continue;
		}
		if (i==2&&path[0]=='.'&&path[1]=='.'){
			root=root->relatives.parent;
			if (!root){
				root=_vfs_root_node;
			}
			path+=i;
			continue;
		}
		vfs_name_t* name=vfs_name_alloc(path,i);
		root=vfs_node_lookup(root,name);
		vfs_name_dealloc(name);
		path+=i;
	}
	return root;
}
