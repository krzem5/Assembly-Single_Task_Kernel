#include <kernel/fs/fs.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>



void fs_node_allocator_init(u8 fs_index,u8 node_size,fs_node_allocator_t* out){
	LOG("Initializing file system node allocator...");
	out->fs_index=fs_index;
	out->first=0;
	out->last=(1<<FS_NODE_ALLOCATOR_SIZE_SHIFT)-2;
	out->next_id=0;
	out->data=VMM_TRANSLATE_ADDRESS(pmm_alloc(pmm_align_up_address(((1<<FS_NODE_ALLOCATOR_SIZE_SHIFT)-1)*sizeof(fs_node_allocator_entry_t))>>PAGE_SIZE_SHIFT));
	INFO("Allocated %v of allocator data at %p",pmm_align_up_address(((1<<FS_NODE_ALLOCATOR_SIZE_SHIFT)-1)*sizeof(fs_node_allocator_entry_t)),VMM_TRANSLATE_ADDRESS_REVERSE(out->data));
	void* node_data=VMM_TRANSLATE_ADDRESS(pmm_alloc(pmm_align_up_address(((1<<FS_NODE_ALLOCATOR_SIZE_SHIFT)-1)*node_size)>>PAGE_SIZE_SHIFT));
	INFO("Allocated %v of allocator data at %p",pmm_align_up_address(((1<<FS_NODE_ALLOCATOR_SIZE_SHIFT)-1)*node_size),VMM_TRANSLATE_ADDRESS_REVERSE(node_data));
	for (fs_node_allocator_index_t i=0;i<(1<<FS_NODE_ALLOCATOR_SIZE_SHIFT)-1;i++){
		(out->data+i)->id=i;
		(out->data+i)->node=node_data;
		(out->data+i)->node->type=FS_NODE_TYPE_INVALID;
		(out->data+i)->id_at_index=i;
		(out->data+i)->prev=i-1;
		(out->data+i)->next=i+1;
		node_data+=node_size;
	}
}



fs_node_t* fs_node_allocator_get(fs_node_allocator_t* allocator,fs_node_id_t id,_Bool allocate_if_not_present){
	if (allocate_if_not_present&&id==FS_NODE_ID_EMPTY){
		id=allocator->next_id|(((u64)(allocator->fs_index))<<56);
		allocator->next_id++;
	}
	if (id==FS_NODE_ID_EMPTY||id==FS_NODE_ID_UNKNOWN){
		return NULL;
	}
	fs_node_allocator_index_t i=1<<(FS_NODE_ALLOCATOR_SIZE_SHIFT-1);
	fs_node_allocator_index_t j=(1<<(FS_NODE_ALLOCATOR_SIZE_SHIFT-1))-1;
	fs_node_allocator_index_t k;
	fs_node_allocator_entry_t* entry;
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
			if (!allocate_if_not_present&&entry->node->type==FS_NODE_TYPE_INVALID){
				return NULL;
			}
			entry->node->id=id;
			return entry->node;
		}
		i>>=1;
		j=(id<entry->id?j-i:j+i);
	} while (i);
	if (!allocate_if_not_present){
		return NULL;
	}
	if (entry->id<id&&j<(1<<FS_NODE_ALLOCATOR_SIZE_SHIFT)-2){
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
	return entry->node;
}
