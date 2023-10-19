#include <kernel/fs/fs.h>
#include <kernel/lock/lock.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs2/name.h>
#include <kernel/vfs2/node.h>



static vfs2_node_t* _vfs2_root_node;



void vfs2_mount(filesystem_t* fs,const char* path){
	if (!path){
		if (_vfs2_root_node){
			panic("Root filesystem already registered");
		}
		_vfs2_root_node=fs->root;
		lock_acquire_exclusive(&(fs->root->lock));
		vfs2_name_dealloc(fs->root->name);
		fs->root->name=vfs2_name_alloc("",0);
		lock_release_exclusive(&(fs->root->lock));
		return;
	}
	panic("vfs2_mount");
}



vfs2_node_t* vfs2_lookup(vfs2_node_t* root,const char* path){
	if (!root){
		root=_vfs2_root_node;
	}
	while (root&&path[0]){
		if (path[0]=='/'){
			path++;
			continue;
		}
		u64 i=0;
		for (;path[i]&&path[i]!='/';i++){
			if (i>=VFS2_NAME_MAX_LENGTH){
				return NULL;
			}
		}
		vfs2_name_t* name=vfs2_name_alloc(path,i);
		root=vfs2_node_lookup(root,name);
		vfs2_name_dealloc(name);
		path+=i;
	}
	return root;
}
