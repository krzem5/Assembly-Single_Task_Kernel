#include <kernel/vfs/allocator.h>
#include <kernel/vfs/vfs.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "vfs_allocator"



void KERNEL_CORE_CODE vfs_allocator_init(u8 vfs_index,u8 node_size,vfs_allocator_t* out){
	LOG_CORE("Initializing file system node allocator...");
	if (node_size<sizeof(vfs_node_t)){
		panic("vfs_allocator_init: node_size too small");
	}
	lock_init(&(out->lock));
	out->vfs_index=vfs_index;
	out->first=0;
	out->last=(1<<VFS_ALLOCATOR_SIZE_SHIFT)-2;
	out->next_id=1;
	out->data=kmm_alloc(((1<<VFS_ALLOCATOR_SIZE_SHIFT)-1)*sizeof(vfs_allocator_entry_t));
	void* node_data=kmm_alloc((1<<VFS_ALLOCATOR_SIZE_SHIFT)*node_size);
	for (vfs_allocator_index_t i=0;i<(1<<VFS_ALLOCATOR_SIZE_SHIFT)-1;i++){
		(out->data+i)->id=i;
		(out->data+i)->node=node_data;
		(out->data+i)->node->type=VFS_NODE_TYPE_INVALID;
		(out->data+i)->id_at_index=i;
		(out->data+i)->prev=i-1;
		(out->data+i)->next=i+1;
		node_data+=node_size;
	}
	out->root_node=node_data;
	out->root_node->id=((u64)vfs_index)<<56;
	out->root_node->type=VFS_NODE_TYPE_INVALID;
}



vfs_node_t* KERNEL_CORE_CODE vfs_allocator_get(vfs_allocator_t* allocator,vfs_node_id_t id,_Bool allocate_if_not_present){
	vfs_node_t* out=NULL;
	lock_acquire_exclusive(&(allocator->lock));
	if (allocate_if_not_present&&id==VFS_NODE_ID_EMPTY){
		if (allocator->root_node->type==VFS_NODE_TYPE_INVALID){
			out=allocator->root_node;
			goto _return;
		}
		id=allocator->next_id|(((u64)(allocator->vfs_index))<<56);
		allocator->next_id++;
	}
	if (id==VFS_NODE_ID_EMPTY||id==VFS_NODE_ID_UNKNOWN){
		goto _return;
	}
	if (!(id<<8)){
		out=allocator->root_node;
		goto _return;
	}
	vfs_allocator_index_t i=1<<(VFS_ALLOCATOR_SIZE_SHIFT-1);
	vfs_allocator_index_t j=(1<<(VFS_ALLOCATOR_SIZE_SHIFT-1))-1;
	vfs_allocator_index_t k;
	vfs_allocator_entry_t* entry;
	do{
		k=(allocator->data+j)->id_at_index;
		entry=allocator->data+k;
		if (entry->id==id){
			if (k!=allocator->first){
				if (k==allocator->last){
					allocator->last=entry->prev;
				}
				else{
					(allocator->data+entry->prev)->next=entry->next;
					(allocator->data+entry->next)->prev=entry->prev;
				}
				(allocator->data+allocator->first)->prev=k;
				entry->next=allocator->first;
				allocator->first=k;
			}
			if (!allocate_if_not_present&&entry->node->type==VFS_NODE_TYPE_INVALID){
				goto _return;
			}
			entry->node->id=id;
			out=entry->node;
			goto _return;
		}
		i>>=1;
		j=(id<entry->id?j-i:j+i);
	} while (i);
	if (!allocate_if_not_present){
		goto _return;
	}
	if (entry->id<id&&j<(1<<VFS_ALLOCATOR_SIZE_SHIFT)-2){
		j++;
	}
	k=allocator->last;
	if (k<j){
		for (i=k;i<j;i++){
			(allocator->data+i)->id_at_index=(allocator->data+i+1)->id_at_index;
		}
	}
	else{
		for (i=k;i>j;i--){
			(allocator->data+i)->id_at_index=(allocator->data+i-1)->id_at_index;
		}
	}
	(allocator->data+j)->id_at_index=k;
	entry=allocator->data+k;
	entry->id=id;
	entry->node->id=id;
	entry->next=allocator->first;
	(allocator->data+allocator->first)->prev=k;
	allocator->first=k;
	allocator->last=entry->prev;
	out=entry->node;
_return:
	lock_release_exclusive(&(allocator->lock));
	return out;
}
