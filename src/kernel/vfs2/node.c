#include <kernel/fs/fs.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs2/name.h>
#include <kernel/vfs2/node.h>



vfs2_node_t* vfs2_node_create(struct _FILESYSTEM2* fs,const vfs2_node_name_t* name){
	if (!fs->functions->create){
		return NULL;
	}
	vfs2_node_t* out=fs->functions->create();
	if (!out){
		return NULL;
	}
	out->flags=0;
	lock_init(&(out->lock));
	out->name=vfs2_name_duplicate(name);
	out->relatives.parent=NULL;
	out->relatives.prev_sibling=NULL;
	out->relatives.next_sibling=NULL;
	out->relatives.child=NULL;
	out->fs=fs;
	out->functions=fs->functions;
	return out;
}



vfs2_node_t* vfs2_node_get_child(vfs2_node_t* node,const vfs2_node_name_t* name){
	vfs2_node_t* out=node->relatives.child;
	for (;out;out=out->relatives.next_sibling){
		if (out->name->length!=name->length||out->name->hash!=name->hash){
			continue;
		}
		for (u32 i=0;i<out->name->length;i++){
			if (out->name->data[i]!=name->data[i]){
				goto _check_next_sibling;
			}
		}
		return out;
_check_next_sibling:
	}
	if (!node->functions->lookup){
		return NULL;
	}
	out=node->functions->lookup(node,name);
	if (!out){
		return NULL;
	}
	lock_acquire_exclusive(&(node->lock));
	out->relatives.parent=node;
	out->relatives.next_sibling=node->relatives.child;
	node->relatives.child=out;
	lock_release_exclusive(&(node->lock));
	return out;
}
