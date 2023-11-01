#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "mmap"



static pmm_counter_descriptor_t _mmap_region_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_mmap_region");
static pmm_counter_descriptor_t _mmap_length_group_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_mmap_length_group");
static omm_allocator_t _mmap_region_allocator=OMM_ALLOCATOR_INIT_STRUCT("mmap_region",sizeof(vmm_memory_map_region_t),8,4,&_mmap_region_omm_pmm_counter);
static omm_allocator_t _mmap_length_group_allocator=OMM_ALLOCATOR_INIT_STRUCT("mmap_length_group",sizeof(vmm_memory_map_length_group_t),8,4,&_mmap_length_group_omm_pmm_counter);



static void _link_region_length(vmm_memory_map_t* mmap,vmm_memory_map_region_t* region){
	vmm_memory_map_length_group_t* length_group=(void*)rb_tree_lookup_node(&(mmap->length_tree),region->length);
	if (length_group){
		panic("_link_region_length");
	}
	length_group=omm_alloc(&_mmap_length_group_allocator);
	length_group->rb_node.key=region->length;
	length_group->head=region;
	region->group=length_group;
	region->group_prev=NULL;
	region->group_next=NULL;
	rb_tree_insert_node(&(mmap->length_tree),&(length_group->rb_node));
}



static void _unlink_region_length(vmm_memory_map_t* mmap,vmm_memory_map_region_t* region){
	vmm_memory_map_length_group_t* length_group=region->group;
	if (region->group_prev){
		region->group_prev->group_next=region->group_next;
	}
	else{
		length_group->head=region->group_next;
	}
	if (region->group_next){
		region->group_next->group_prev=region->group_prev;
	}
	if (!length_group->head){
		rb_tree_remove_node(&(mmap->length_tree),&(length_group->rb_node));
		omm_dealloc(&_mmap_length_group_allocator,length_group);
	}
}



void vmm_memory_map_init(u64 low,u64 high,vmm_memory_map_t* out){
	spinlock_init(&(out->lock));
	rb_tree_init(&(out->offset_tree));
	rb_tree_init(&(out->length_tree));
	vmm_memory_map_region_t* region=omm_alloc(&_mmap_region_allocator);
	region->is_used=0;
	region->rb_node.key=low;
	region->length=high-low;
	region->prev=NULL;
	region->next=NULL;
	rb_tree_insert_node(&(out->offset_tree),&(region->rb_node));
	_link_region_length(out,region);
}



void vmm_memory_map_deinit(vmm_pagemap_t* pagemap,vmm_memory_map_t* mmap){
	spinlock_acquire_exclusive(&(mmap->lock));
	panic("vmm_memory_map_deinit");
	rb_tree_init(&(mmap->offset_tree));
	rb_tree_init(&(mmap->length_tree));
	spinlock_release_exclusive(&(mmap->lock));
}



u64 vmm_memory_map_reserve(vmm_memory_map_t* mmap,u64 address,u64 length){
	if ((address|length)&(PAGE_SIZE-1)){
		panic("vmm_memory_map_reserve: unaligned arguments");
	}
	if (!length){
		return 0;
	}
	spinlock_acquire_exclusive(&(mmap->lock));
	vmm_memory_map_region_t* region;
	if (address){
		region=(void*)rb_tree_lookup_decreasing_node(&(mmap->offset_tree),address);
		if (!region){
			spinlock_release_exclusive(&(mmap->lock));
			return 0;
		}
	}
	else{
		vmm_memory_map_length_group_t* length_group=(void*)rb_tree_lookup_increasing_node(&(mmap->length_tree),address);
		if (!length_group){
			spinlock_release_exclusive(&(mmap->lock));
			return 0;
		}
		region=length_group->head;
		length_group->head=region->group_next;
		if (region->group_next){
			region->group_next->group_prev=NULL;
		}
	}
	if (!address){
		address=region->rb_node.key;
	}
	else if (region->is_used||address-region->rb_node.key+length>region->length){
		spinlock_release_exclusive(&(mmap->lock));
		return 0;
	}
	_unlink_region_length(mmap,region);
	if (address!=region->rb_node.key){
		vmm_memory_map_region_t* new_region=omm_alloc(&_mmap_region_allocator);
		new_region->rb_node.key=address;
		new_region->length=region->rb_node.key+region->length-address;
		new_region->prev=region;
		new_region->next=region->next;
		if (region->next){
			region->next->prev=new_region;
		}
		region->length=address-region->rb_node.key;
		region->next=new_region;
		_link_region_length(mmap,region);
		rb_tree_insert_node(&(mmap->offset_tree),&(new_region->rb_node));
		region=new_region;
	}
	region->is_used=1;
	if (region->length>length){
		vmm_memory_map_region_t* new_region=omm_alloc(&_mmap_region_allocator);
		new_region->is_used=0;
		new_region->rb_node.key=address+length;
		new_region->length=region->length-length;
		new_region->prev=region;
		new_region->next=region->next;
		if (region->next){
			region->next->prev=new_region;
		}
		region->length=length;
		region->next=new_region;
		rb_tree_insert_node(&(mmap->offset_tree),&(new_region->rb_node));
		_link_region_length(mmap,new_region);
	}
	spinlock_release_exclusive(&(mmap->lock));
	return address;
}



_Bool vmm_memory_map_release(vmm_memory_map_t* mmap,u64 address,u64 length){
	if ((address|length)&(PAGE_SIZE-1)){
		panic("vmm_memory_map_release: unaligned arguments");
	}
	spinlock_acquire_exclusive(&(mmap->lock));
	rb_tree_node_t* rb_node=rb_tree_lookup_decreasing_node(&(mmap->offset_tree),address);
	if (!rb_node){
		spinlock_release_exclusive(&(mmap->lock));
		return 0;
	}
	vmm_memory_map_region_t* region=((void*)rb_node)-__builtin_offsetof(vmm_memory_map_region_t,rb_node);
	if (!region->is_used){
		spinlock_release_exclusive(&(mmap->lock));
		return 0;
	}
	if (region->rb_node.key!=address||region->length!=length){
		panic("vmm_memory_map_release: partial release");
	}
	if (region->prev&&!region->prev->is_used){
		vmm_memory_map_region_t* prev_region=region->prev;
		rb_tree_remove_node(&(mmap->offset_tree),&(region->rb_node));
		_unlink_region_length(mmap,prev_region);
		prev_region->length+=length;
		prev_region->next=region->next;
		if (prev_region->next){
			prev_region->next->prev=prev_region;
		}
		omm_dealloc(&_mmap_region_allocator,region);
		region=prev_region;
	}
	if (region->next&&!region->next->is_used){
		vmm_memory_map_region_t* next_region=region->next;
		rb_tree_remove_node(&(mmap->offset_tree),&(next_region->rb_node));
		_unlink_region_length(mmap,next_region);
		region->length+=next_region->length;
		region->next=next_region->next;
		if (region->next){
			region->next->prev=region;
		}
		omm_dealloc(&_mmap_region_allocator,next_region);
	}
	region->is_used=0;
	_link_region_length(mmap,region);
	spinlock_release_exclusive(&(mmap->lock));
	return 1;
}
