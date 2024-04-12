#include <kernel/clock/clock.h>
#include <kernel/fs/fs.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/time/time.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/util/spinloop.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "vfs_node"



static omm_allocator_t* _vfs_node_empty_node_allocator=NULL;



static vfs_node_t* _empty_node_alloc(vfs_node_t* parent,const string_t* name,u32 flags){
	if (parent||name){
		return NULL;
	}
	return omm_alloc(_vfs_node_empty_node_allocator);
}



static void _empty_node_dealloc(vfs_node_t* node){
	omm_dealloc(_vfs_node_empty_node_allocator,node);
}



static const vfs_functions_t _vfs_node_empty_functions={
	_empty_node_alloc,
	_empty_node_dealloc,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};



static vfs_node_t* _init_node(filesystem_t* fs,const vfs_functions_t* functions,vfs_node_t* parent,const string_t* name,u32 flags){
	if (!functions->create){
		return NULL;
	}
	vfs_node_t* out=((flags&VFS_NODE_FLAG_CREATE)?functions->create(parent,name,flags):functions->create(NULL,NULL,0));
	if (!out){
		return NULL;
	}
	spinlock_init(&(out->lock));
	u64 time=clock_get_time()+time_boot_offset;
	out->flags=0;
	out->rc=0;
	out->name=smm_duplicate(name);
	out->relatives.parent=NULL;
	out->relatives.prev_sibling=NULL;
	out->relatives.next_sibling=NULL;
	out->relatives.child=NULL;
	out->fs=fs;
	out->functions=functions;
	out->time_access=time;
	out->time_modify=time;
	out->time_change=time;
	out->time_birth=time;
	out->gid=0;
	out->uid=0;
	return out;
}



KERNEL_INIT(){
	LOG("Initializing VFS nodes...");
	_vfs_node_empty_node_allocator=omm_init("vfs_empty_node",sizeof(vfs_node_t),8,1,pmm_alloc_counter("omm_vfs_empty_node"));
	spinlock_init(&(_vfs_node_empty_node_allocator->lock));
}



KERNEL_PUBLIC vfs_node_t* vfs_node_create(filesystem_t* fs,vfs_node_t* parent,const string_t* name,u32 flags){
	if (!fs){
		fs=parent->fs;
	}
	return _init_node(fs,fs->functions,parent,name,flags);
}



KERNEL_PUBLIC vfs_node_t* vfs_node_create_virtual(vfs_node_t* parent,const vfs_functions_t* functions,const string_t* name){
	vfs_node_t* out=_init_node(NULL,(functions?functions:&_vfs_node_empty_functions),NULL,name,0);
	if (!out){
		return NULL;
	}
	out->flags|=VFS_NODE_FLAG_VIRTUAL;
	vfs_node_attach_child(parent,out);
	return out;
}



KERNEL_PUBLIC void vfs_node_delete(vfs_node_t* node){
	if (!node->functions->delete){
		panic("vfs_node_delete: node->functions->delete not present");
	}
	if (node->relatives.parent){
		panic("vfs_node_delete: unlink parent");
	}
	if (node->relatives.child){
		panic("vfs_node_delete: delete children");
	}
	SPINLOOP(node->rc);
	smm_dealloc(node->name);
	node->functions->delete(node);
}



KERNEL_PUBLIC vfs_node_t* vfs_node_lookup(vfs_node_t* node,const string_t* name){
	vfs_node_t* out=node->relatives.child;
	for (;out;out=out->relatives.next_sibling){
		if (smm_equal(out->name,name)){
			return out;
		}
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



KERNEL_PUBLIC u64 vfs_node_iterate(vfs_node_t* node,u64 pointer,string_t** out){
	if (!pointer||(pointer>>63)){
		vfs_node_t* child=(pointer?((vfs_node_t*)pointer)->relatives.next_sibling:node->relatives.child);
		for (;child;child=child->relatives.next_sibling){
			if (child->flags&VFS_NODE_FLAG_VIRTUAL){
				*out=smm_duplicate(child->name);
				return (u64)child;
			}
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



KERNEL_PUBLIC _Bool vfs_node_link(vfs_node_t* node,vfs_node_t* parent){
	if (!node->functions->link){
		return 0;
	}
	return node->functions->link(node,parent);
}



KERNEL_PUBLIC _Bool vfs_node_unlink(vfs_node_t* node){
	if (!node->functions->unlink){
		return 0;
	}
	return node->functions->unlink(node);
}



KERNEL_PUBLIC u64 vfs_node_read(vfs_node_t* node,u64 offset,void* buffer,u64 size,u32 flags){
	if (!size||!node->functions->read){
		return 0;
	}
	return node->functions->read(node,offset,buffer,size,flags);
}



KERNEL_PUBLIC u64 vfs_node_write(vfs_node_t* node,u64 offset,const void* buffer,u64 size,u32 flags){
	if (!size||!node->functions->write){
		return 0;
	}
	return node->functions->write(node,offset,buffer,size,flags);
}



KERNEL_PUBLIC u64 vfs_node_resize(vfs_node_t* node,s64 offset,u32 flags){
	if (!node->functions->resize){
		return 0;
	}
	return node->functions->resize(node,offset,flags);
}



KERNEL_PUBLIC void vfs_node_flush(vfs_node_t* node){
	if (node->functions->flush){
		node->functions->flush(node);
	}
}



KERNEL_PUBLIC void vfs_node_attach_child(vfs_node_t* node,vfs_node_t* child){
	spinlock_acquire_exclusive(&(node->lock));
	child->relatives.parent=node;
	child->relatives.next_sibling=node->relatives.child;
	if (node->relatives.child){
		spinlock_acquire_exclusive(&(node->relatives.child->lock));
		node->relatives.child->relatives.prev_sibling=child;
		spinlock_release_exclusive(&(node->relatives.child->lock));
	}
	node->relatives.child=child;
	spinlock_release_exclusive(&(node->lock));
}



KERNEL_PUBLIC void vfs_node_dettach_child(vfs_node_t* node){
	spinlock_acquire_exclusive(&(node->lock));
	if (node->relatives.parent){
		if (node->relatives.prev_sibling){
			spinlock_acquire_exclusive(&(node->relatives.prev_sibling->lock));
			node->relatives.prev_sibling->relatives.next_sibling=node->relatives.next_sibling;
			spinlock_release_exclusive(&(node->relatives.prev_sibling->lock));
		}
		else{
			spinlock_acquire_exclusive(&(node->relatives.parent->lock));
			node->relatives.parent->relatives.child=node->relatives.next_sibling;
			spinlock_release_exclusive(&(node->relatives.parent->lock));
		}
		if (node->relatives.next_sibling){
			spinlock_acquire_exclusive(&(node->relatives.next_sibling->lock));
			node->relatives.next_sibling->relatives.prev_sibling=node->relatives.prev_sibling;
			spinlock_release_exclusive(&(node->relatives.next_sibling->lock));
		}
		node->relatives.parent=NULL;
	}
	spinlock_release_exclusive(&(node->lock));
}
