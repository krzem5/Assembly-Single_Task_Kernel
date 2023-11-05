#include <kernel/fs/fs.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/memory/smm.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "vfs_node"



vfs_node_t* vfs_node_create(filesystem_t* fs,const string_t* name){
	if (!fs->functions->create){
		return NULL;
	}
	vfs_node_t* out=fs->functions->create();
	if (!out){
		return NULL;
	}
	out->flags=0;
	spinlock_init(&(out->lock));
	out->name=smm_duplicate(name);
	out->relatives.parent=NULL;
	out->relatives.prev_sibling=NULL;
	out->relatives.next_sibling=NULL;
	out->relatives.child=NULL;
	out->relatives.external_prev_sibling=NULL;
	out->relatives.external_next_sibling=NULL;
	out->relatives.external_child=NULL;
	out->fs=fs;
	out->functions=fs->functions;
	return out;
}



void vfs_node_delete(vfs_node_t* node){
	if (!node->functions->delete){
		panic("vfs_node_delete: node->functions->delete not present");
	}
	if (node->relatives.child){
		panic("vfs_node_delete: non-NULL node->relatives.child");
	}
	if (node->relatives.prev_sibling||node->relatives.next_sibling){
		panic("vfs_node_delete: relink prev/next sibling");
	}
	smm_dealloc(node->name);
	node->functions->delete(node);
}



vfs_node_t* vfs_node_lookup(vfs_node_t* node,const string_t* name){
	vfs_node_t* out=node->relatives.child;
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
	out=node->relatives.external_child;
	for (;out;out=out->relatives.external_next_sibling){
		if (out->name->length!=name->length||out->name->hash!=name->hash){
			continue;
		}
		for (u32 i=0;i<out->name->length;i++){
			if (out->name->data[i]!=name->data[i]){
				goto _check_next_external_sibling;
			}
		}
		return out;
_check_next_external_sibling:
	}
	if (!node->functions->lookup){
		return NULL;
	}
	out=node->functions->lookup(node,name);
	if (!out){
		return NULL;
	}
	spinlock_acquire_exclusive(&(node->lock));
	out->relatives.parent=node;
	out->relatives.next_sibling=node->relatives.child;
	if (node->relatives.child){
		spinlock_acquire_exclusive(&(node->relatives.child->lock));
		node->relatives.child->relatives.prev_sibling=out;
		spinlock_release_exclusive(&(node->relatives.child->lock));
	}
	node->relatives.child=out;
	spinlock_release_exclusive(&(node->lock));
	return out;
}



u64 vfs_node_iterate(vfs_node_t* node,u64 pointer,string_t** out){
	if ((pointer>>63)||(!pointer&&node->relatives.external_child)){
		vfs_node_t* child=(pointer?((vfs_node_t*)pointer)->relatives.external_next_sibling:node->relatives.external_child);
		if (child){
			*out=smm_duplicate(child->name);
			return (u64)child;
		}
		pointer=0;
	}
	if (!node->functions->iterate){
		return 0;
	}
	u64 ret=node->functions->iterate(node,pointer,out);
	if (ret>>63){
		panic("vfs_node_iterate: node->functions->iterate returned an invalid pointer");
	}
	return ret;
}



_Bool vfs_node_link(vfs_node_t* node,vfs_node_t* parent){
	if (!node->functions->link){
		return 0;
	}
	return node->functions->link(node,parent);
}



_Bool vfs_node_unlink(vfs_node_t* node){
	if (!node->functions->unlink){
		return 0;
	}
	return node->functions->unlink(node);
}



s64 vfs_node_read(vfs_node_t* node,u64 offset,void* buffer,u64 size){
	if (!node->functions->read){
		return 0;
	}
	return node->functions->read(node,offset,buffer,size);
}



s64 vfs_node_write(vfs_node_t* node,u64 offset,const void* buffer,u64 size){
	if (!node->functions->write){
		return 0;
	}
	return node->functions->write(node,offset,buffer,size);
}



s64 vfs_node_resize(vfs_node_t* node,s64 offset,u32 flags){
	if (!node->functions->read){
		return 0;
	}
	return node->functions->resize(node,offset,flags);
}



void vfs_node_flush(vfs_node_t* node){
	if (node->functions->flush){
		node->functions->flush(node);
	}
}



void vfs_node_attach_external_child(vfs_node_t* node,vfs_node_t* child){
	spinlock_acquire_exclusive(&(node->lock));
	child->relatives.parent=node;
	child->relatives.external_next_sibling=node->relatives.external_child;
	if (node->relatives.external_child){
		spinlock_acquire_exclusive(&(node->relatives.external_child->lock));
		node->relatives.external_child->relatives.external_prev_sibling=child;
		spinlock_release_exclusive(&(node->relatives.external_child->lock));
	}
	node->relatives.external_child=child;
	spinlock_release_exclusive(&(node->lock));
}
