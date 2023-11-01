#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "mmap"



static pmm_counter_descriptor_t _mmap_range_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_mmap_range");
static omm_allocator_t _mmap_region_allocator=OMM_ALLOCATOR_INIT_STRUCT("mmap_range",sizeof(vmm_memory_map_region_t),8,4,&_mmap_range_omm_pmm_counter);



void vmm_memory_map_init(u64 low,u64 high,vmm_memory_map_t* out){
	spinlock_init(&(out->lock));
	rb_tree_init(&(out->offset_tree));
	rb_tree_init(&(out->length_tree));
	vmm_memory_map_region_t* region=omm_alloc(&_mmap_region_allocator);
	region->is_used=0;
	region->rb_offset_node.key=low;
	region->rb_length_node.key=high-low;
	region->prev=NULL;
	region->next=NULL;
	rb_tree_insert_node(&(out->offset_tree),&(region->rb_offset_node));
	rb_tree_insert_node(&(out->length_tree),&(region->rb_length_node));
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
		rb_tree_node_t* rb_node=rb_tree_lookup_decreasing_node(&(mmap->offset_tree),address);
		if (!rb_node){
			spinlock_release_exclusive(&(mmap->lock));
			return 0;
		}
		region=((void*)rb_node)-__builtin_offsetof(vmm_memory_map_region_t,rb_offset_node);
	}
	else{
		rb_tree_node_t* rb_node=rb_tree_lookup_increasing_node(&(mmap->length_tree),address);
		if (!rb_node){
			spinlock_release_exclusive(&(mmap->lock));
			return 0;
		}
		region=((void*)rb_node)-__builtin_offsetof(vmm_memory_map_region_t,rb_length_node);
	}
	if (!address){
		address=region->rb_offset_node.key;
	}
	else if (region->is_used||address-region->rb_offset_node.key+length>region->rb_length_node.key){
		spinlock_release_exclusive(&(mmap->lock));
		return 0;
	}
	rb_tree_remove_node(&(mmap->length_tree),&(region->rb_length_node));
	if (address!=region->rb_offset_node.key){
		vmm_memory_map_region_t* new_region=omm_alloc(&_mmap_region_allocator);
		new_region->rb_offset_node.key=address;
		new_region->rb_length_node.key=region->rb_offset_node.key+region->rb_length_node.key-address;
		new_region->prev=region;
		new_region->next=region->next;
		if (region->next){
			region->next->prev=new_region;
		}
		region->rb_length_node.key=address-region->rb_offset_node.key;
		region->next=new_region;
		rb_tree_insert_node(&(mmap->length_tree),&(region->rb_length_node));
		rb_tree_insert_node(&(mmap->offset_tree),&(new_region->rb_offset_node));
		region=new_region;
	}
	region->is_used=1;
	if (region->rb_length_node.key>length){
		vmm_memory_map_region_t* new_region=omm_alloc(&_mmap_region_allocator);
		new_region->is_used=0;
		new_region->rb_offset_node.key=address+length;
		new_region->rb_length_node.key=region->rb_length_node.key-length;
		new_region->prev=region;
		new_region->next=region->next;
		if (region->next){
			region->next->prev=new_region;
		}
		region->rb_length_node.key=length;
		region->next=new_region;
		rb_tree_insert_node(&(mmap->offset_tree),&(new_region->rb_offset_node));
		rb_tree_insert_node(&(mmap->length_tree),&(new_region->rb_length_node));
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
	vmm_memory_map_region_t* region=((void*)rb_node)-__builtin_offsetof(vmm_memory_map_region_t,rb_offset_node);
	if (!region->is_used){
		spinlock_release_exclusive(&(mmap->lock));
		return 0;
	}
	if (region->rb_offset_node.key!=address||region->rb_length_node.key!=length){
		panic("vmm_memory_map_release: partial release");
	}
	if (region->prev&&!region->prev->is_used){
		vmm_memory_map_region_t* prev_region=region->prev;
		rb_tree_remove_node(&(mmap->offset_tree),&(region->rb_offset_node));
		rb_tree_remove_node(&(mmap->length_tree),&(prev_region->rb_length_node));
		prev_region->rb_length_node.key+=length;
		prev_region->next=region->next;
		if (prev_region->next){
			prev_region->next->prev=prev_region;
		}
		omm_dealloc(&_mmap_region_allocator,region);
		region=prev_region;
	}
	if (region->next&&!region->next->is_used){
		vmm_memory_map_region_t* next_region=region->next;
		rb_tree_remove_node(&(mmap->offset_tree),&(next_region->rb_offset_node));
		rb_tree_remove_node(&(mmap->length_tree),&(next_region->rb_length_node));
		region->rb_length_node.key+=next_region->rb_length_node.key;
		region->next=next_region->next;
		if (region->next){
			region->next->prev=region;
		}
		omm_dealloc(&_mmap_region_allocator,next_region);
	}
	region->is_used=0;
	rb_tree_insert_node(&(mmap->length_tree),&(region->rb_length_node));
	spinlock_release_exclusive(&(mmap->lock));
	return 1;
}
