#include <kernel/fs/fs.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/memory/smm.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "vfs"



static vfs_node_t* _vfs_root_node=NULL;



static vfs_node_t* _lookup_node(vfs_node_t* root,const char* path,_Bool follow_links,vfs_node_t** parent,const char** child_name){
	if (!root||path[0]=='/'){
		root=_vfs_root_node;
	}
	if (parent){
		*parent=NULL;
		*child_name=NULL;
	}
	while (root&&path[0]){
		if (follow_links&&(root->flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_LINK){
			char buffer[4096];
			if (vfs_node_read(root,0,buffer,4096)<=0){
				return NULL;
			}
			root=_lookup_node(root->relatives.parent,buffer,1,NULL,NULL);
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
			root=root->relatives.parent;
			if (!root){
				root=_vfs_root_node;
			}
			path+=2;
			continue;
		}
		SMM_TEMPORARY_STRING name=smm_alloc(path,i);
		vfs_node_t* child=vfs_node_lookup(root,name);
		path+=i;
		if (!child&&parent){
			if (!path[0]){
				*parent=root;
				*child_name=path-i;
				return NULL;
			}
			panic("_lookup_node: alloc virtual node");
		}
		root=child;
	}
	if (root&&follow_links&&(root->flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_LINK){
		char buffer[4096];
		if (vfs_node_read(root,0,buffer,4096)<=0){
			return NULL;
		}
		root=_lookup_node(root->relatives.parent,buffer,1,NULL,NULL);
		if (!root){
			return NULL;
		}
	}
	return root;
}



void vfs_mount(filesystem_t* fs,const char* path){
	if (!path){
		_vfs_root_node=fs->root;
		spinlock_acquire_exclusive(&(_vfs_root_node->lock));
		_vfs_root_node->relatives.parent=NULL;
		spinlock_release_exclusive(&(_vfs_root_node->lock));
		return;
	}
	vfs_node_t* parent;
	const char* child_name;
	if (_lookup_node(NULL,path,0,&parent,&child_name)){
		panic("vfs_mount: node already exists");
	}
	spinlock_acquire_exclusive(&(fs->root->lock));
	smm_dealloc(fs->root->name);
	fs->root->name=smm_alloc(child_name,0);
	spinlock_release_exclusive(&(fs->root->lock));
	vfs_node_attach_external_child(parent,fs->root);
}



vfs_node_t* vfs_lookup(vfs_node_t* root,const char* path,_Bool follow_links){
	return _lookup_node(root,path,follow_links,NULL,NULL);
}
