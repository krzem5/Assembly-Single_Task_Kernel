#include <kernel/fs/fs.h>
#include <kernel/lock/lock.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs2/name.h>
#include <kernel/vfs2/node.h>



static vfs2_node_t* _vfs2_root_node;



void vfs2_mount(filesystem2_t* fs,const char* path){
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
