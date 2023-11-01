#include <kernel/fs/fs.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/memory/smm.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "vfs"



static vfs_node_t* _vfs_root_node=NULL;



void vfs_mount(filesystem_t* fs,const char* path){
	if (!path){
		_vfs_root_node=fs->root;
		spinlock_acquire_exclusive(&(_vfs_root_node->lock));
		_vfs_root_node->relatives.parent=NULL;
		spinlock_release_exclusive(&(_vfs_root_node->lock));
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
			if (i>=SMM_MAX_LENGTH){
				return NULL;
			}
		}
		if (i==1&&path[0]=='.'){
			path+=1;
			continue;
		}
		if (i==2&&path[0]=='.'&&path[1]=='.'){
			root=root->relatives.parent;
			if (!root){
				root=_vfs_root_node;
			}
			path+=2;
			continue;
		}
		SMM_TEMPORARY_STRING name=smm_alloc(path,i);
		root=vfs_node_lookup(root,name);
		path+=i;
	}
	return root;
}
