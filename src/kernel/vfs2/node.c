#include <kernel/fs/fs.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs2/name.h>
#include <kernel/vfs2/node.h>



vfs2_node_t* vfs2_node_create(struct _FILESYSTEM2* fs,const vfs2_name_t* name){
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



void vfs2_node_delete(vfs2_node_t* node){
	if (!node->functions->delete){
		panic("vfs2_node_delete: node->functions->delete not present");
	}
	if (node->relatives.child){
		panic("vfs2_node_delete: non-NULL node->relatives.child");
	}
	if (node->relatives.prev_sibling||node->relatives.next_sibling){
		panic("vfs2_node_delete: relink prev/next sibling");
	}
	vfs2_name_dealloc(node->name);
	node->functions->delete(node);
}



vfs2_node_t* vfs2_node_lookup(vfs2_node_t* node,const vfs2_name_t* name){
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



u64 vfs2_node_iterate(vfs2_node_t* node,u64 pointer,vfs2_name_t** out){
	if (!node->functions->iterate){
		return 0;
	}
	return node->functions->iterate(node,pointer,out);
}



_Bool vfs2_node_link(vfs2_node_t* node,vfs2_node_t* parent){
	if (!node->functions->link){
		return 0;
	}
	return node->functions->link(node,parent);
}



_Bool vfs2_node_unlink(vfs2_node_t* node){
	if (!node->functions->unlink){
		return 0;
	}
	return node->functions->unlink(node);
}



s64 vfs2_node_read(vfs2_node_t* node,u64 offset,void* buffer,u64 size){
	if (!node->functions->read){
		return 0;
	}
	return node->functions->read(node,offset,buffer,size);
}



s64 vfs2_node_write(vfs2_node_t* node,u64 offset,const void* buffer,u64 size){
	if (!node->functions->write){
		return 0;
	}
	return node->functions->write(node,offset,buffer,size);
}



s64 vfs2_node_resize(vfs2_node_t* node,s64 offset,u32 flags){
	if (!node->functions->read){
		return 0;
	}
	return node->functions->resize(node,offset,flags);
}



void vfs2_node_flush(vfs2_node_t* node){
	if (node->functions->flush){
		node->functions->flush(node);
	}
}
