#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "mmap"



static pmm_counter_descriptor_t _mmap_file_pmm_counter=PMM_COUNTER_INIT_STRUCT("mmap_file");
static pmm_counter_descriptor_t _mmap_generic_pmm_counter=PMM_COUNTER_INIT_STRUCT("mmap_generic");
static pmm_counter_descriptor_t _mmap_region_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_mmap_region");
static pmm_counter_descriptor_t _mmap_length_group_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_mmap_length_group");
static omm_allocator_t _mmap_region_allocator=OMM_ALLOCATOR_INIT_STRUCT("mmap_region",sizeof(mmap_region_t),8,4,&_mmap_region_omm_pmm_counter);
static omm_allocator_t _mmap_length_group_allocator=OMM_ALLOCATOR_INIT_STRUCT("mmap_length_group",sizeof(mmap_length_group_t),8,4,&_mmap_length_group_omm_pmm_counter);



static void _add_region_to_length_tree(mmap_t* mmap,mmap_region_t* region){
	mmap_length_group_t* length_group=(void*)rb_tree_lookup_node(&(mmap->length_tree),region->length);
	if (length_group){
		region->group_prev=NULL;
		region->group_next=length_group->head;
		if (length_group->head){
			length_group->head->group_prev=region;
		}
		length_group->head=region;
		return;
	}
	length_group=omm_alloc(&_mmap_length_group_allocator);
	length_group->rb_node.key=region->length;
	length_group->head=region;
	region->group=length_group;
	region->group_prev=NULL;
	region->group_next=NULL;
	rb_tree_insert_node(&(mmap->length_tree),&(length_group->rb_node));
}



static void _remove_region_from_length_tree(mmap_t* mmap,mmap_region_t* region){
	mmap_length_group_t* length_group=region->group;
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



static void __DEBUG(mmap_t* mmap){
	INFO("Debug:");
	for (rb_tree_node_t* rb_node=rb_tree_iter_start(&(mmap->offset_tree));rb_node;rb_node=rb_tree_iter_next(&(mmap->offset_tree),rb_node)){
		mmap_region_t* region=(mmap_region_t*)rb_node;
		INFO("%p - %p: %X",region->rb_node.key,region->length,region->flags);
	}
}



static _Bool _dealloc_region(mmap_t* mmap,mmap_region_t* region){
	if (!region->flags){
		spinlock_release_exclusive(&(mmap->lock));
		return 0;
	}
	for (u64 i=0;i<region->length;i+=PAGE_SIZE){
		u64 physical_address=vmm_unmap_page(mmap->pagemap,region->rb_node.key+i)&VMM_PAGE_ADDRESS_MASK;
		if (physical_address){
			if (region->file&&!(region->flags&MMAP_REGION_FLAG_NO_FILE_WRITEBACK)){
				panic("mmap_dealloc: file-backed memory region writeback");
			}
			pmm_dealloc(physical_address,1,region->pmm_counter);
		}
	}
	(void)__DEBUG;
	// __DEBUG(mmap);
	spinlock_release_exclusive(&(mmap->lock));
	return 1;
	if (region->prev&&!region->prev->flags){
		mmap_region_t* prev_region=region->prev;
		rb_tree_remove_node(&(mmap->offset_tree),&(region->rb_node));
		_remove_region_from_length_tree(mmap,prev_region);
		prev_region->length+=region->length;
		prev_region->next=region->next;
		if (prev_region->next){
			prev_region->next->prev=prev_region;
		}
		omm_dealloc(&_mmap_region_allocator,region);
		region=prev_region;
	}
	if (region->next&&!region->next->flags){
		mmap_region_t* next_region=region->next;
		rb_tree_remove_node(&(mmap->offset_tree),&(next_region->rb_node));
		_remove_region_from_length_tree(mmap,next_region);
		region->length+=next_region->length;
		region->next=next_region->next;
		if (region->next){
			region->next->prev=region;
		}
		omm_dealloc(&_mmap_region_allocator,next_region);
	}
	region->flags=0;
	_add_region_to_length_tree(mmap,region);
	// __DEBUG(mmap);
	spinlock_release_exclusive(&(mmap->lock));
	return 1;
}



void mmap_init(vmm_pagemap_t* pagemap,u64 low,u64 high,mmap_t* out){
	out->pagemap=pagemap;
	spinlock_init(&(out->lock));
	rb_tree_init(&(out->offset_tree));
	rb_tree_init(&(out->length_tree));
	mmap_region_t* region=omm_alloc(&_mmap_region_allocator);
	region->flags=0;
	region->rb_node.key=low;
	region->length=high-low;
	region->prev=NULL;
	region->next=NULL;
	rb_tree_insert_node(&(out->offset_tree),&(region->rb_node));
	_add_region_to_length_tree(out,region);
}



void mmap_deinit(mmap_t* mmap){
	spinlock_acquire_exclusive(&(mmap->lock));
	panic("mmap_deinit");
	rb_tree_init(&(mmap->offset_tree));
	rb_tree_init(&(mmap->length_tree));
	spinlock_release_exclusive(&(mmap->lock));
}



mmap_region_t* mmap_alloc(mmap_t* mmap,u64 address,u64 length,pmm_counter_descriptor_t* pmm_counter,u64 flags,vfs_node_t* file){
	if ((address|length)&(PAGE_SIZE-1)){
		panic("mmap_alloc: unaligned arguments");
	}
	if (!pmm_counter){
		pmm_counter=(file?&_mmap_file_pmm_counter:&_mmap_generic_pmm_counter);
	}
	if (!length&&file){
		length=pmm_align_up_address(vfs_node_resize(file,0,VFS_NODE_FLAG_RESIZE_RELATIVE));
	}
	if (!length){
		return NULL;
	}
	spinlock_acquire_exclusive(&(mmap->lock));
	mmap_region_t* region;
	if (address){
		region=(void*)rb_tree_lookup_decreasing_node(&(mmap->offset_tree),address);
		if (!region){
			spinlock_release_exclusive(&(mmap->lock));
			return NULL;
		}
	}
	else{
		mmap_length_group_t* length_group=(void*)rb_tree_lookup_increasing_node(&(mmap->length_tree),address);
		if (!length_group){
			spinlock_release_exclusive(&(mmap->lock));
			return NULL;
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
	else if (region->flags||address-region->rb_node.key+length>region->length){
		spinlock_release_exclusive(&(mmap->lock));
		return NULL;
	}
	_remove_region_from_length_tree(mmap,region);
	if (address!=region->rb_node.key){
		mmap_region_t* new_region=omm_alloc(&_mmap_region_allocator);
		new_region->rb_node.key=address;
		new_region->length=region->rb_node.key+region->length-address;
		new_region->prev=region;
		new_region->next=region->next;
		if (region->next){
			region->next->prev=new_region;
		}
		region->length=address-region->rb_node.key;
		region->next=new_region;
		_add_region_to_length_tree(mmap,region);
		rb_tree_insert_node(&(mmap->offset_tree),&(new_region->rb_node));
		region=new_region;
	}
	if (region->length>length){
		mmap_region_t* new_region=omm_alloc(&_mmap_region_allocator);
		new_region->flags=0;
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
		_add_region_to_length_tree(mmap,new_region);
	}
	region->flags=flags|MMAP_REGION_FLAG_USED;
	region->pmm_counter=pmm_counter;
	region->file=file;
	spinlock_release_exclusive(&(mmap->lock));
	if (flags&MMAP_REGION_FLAG_COMMIT){
		if (file){
			panic("mmap_alloc: both file and MMAP_REGION_FLAG_COMMIT specfied");
		}
		u64 vmm_flags=mmap_get_vmm_flags(region);
		for (u64 i=0;i<length;i+=PAGE_SIZE){
			vmm_map_page(mmap->pagemap,pmm_alloc(1,pmm_counter,0),address+i,vmm_flags);
		}
	}
	return region;
}



_Bool mmap_dealloc(mmap_t* mmap,u64 address,u64 length){
	if ((address|length)&(PAGE_SIZE-1)){
		panic("mmap_dealloc: unaligned arguments");
	}
	spinlock_acquire_exclusive(&(mmap->lock));
	rb_tree_node_t* rb_node=rb_tree_lookup_decreasing_node(&(mmap->offset_tree),address);
	if (!rb_node){
		spinlock_release_exclusive(&(mmap->lock));
		return 0;
	}
	mmap_region_t* region=((void*)rb_node)-__builtin_offsetof(mmap_region_t,rb_node);
	if (region->rb_node.key!=address||region->length!=length){
		panic("mmap_dealloc: partial release");
	}
	return _dealloc_region(mmap,region);
}



_Bool mmap_dealloc_region(mmap_t* mmap,mmap_region_t* region){
	spinlock_acquire_exclusive(&(mmap->lock));
	return _dealloc_region(mmap,region);
}



_Bool mmap_set_memory(mmap_t* mmap,mmap_region_t* region,const void* data,u64 length){
	spinlock_acquire_shared(&(mmap->lock));
	if (!(region->flags&MMAP_REGION_FLAG_COMMIT)){
		panic("mmap_set_memory: MMAP_REGION_FLAG_COMMIT flag missing");
	}
	if (length>region->length){
		length=region->length;
	}
	for (u64 i=0;i<length;i+=PAGE_SIZE){
		u64 physical_address=vmm_virtual_to_physical(mmap->pagemap,region->rb_node.key+i);
		if (!physical_address){
			panic("mmap_set_memory: wrong memory mapping");
		}
		memcpy((void*)(physical_address+VMM_HIGHER_HALF_ADDRESS_OFFSET),data+i,(length-i>PAGE_SIZE?PAGE_SIZE:length-i));
	}
	spinlock_release_shared(&(mmap->lock));
	return 1;
}



mmap_region_t* mmap_lookup(mmap_t* mmap,u64 address){
	spinlock_acquire_shared(&(mmap->lock));
	mmap_region_t* out=(void*)rb_tree_lookup_decreasing_node(&(mmap->offset_tree),address);
	spinlock_release_shared(&(mmap->lock));
	return (out&&out->flags?out:NULL);
}



u64 mmap_get_vmm_flags(mmap_region_t* region){
	u64 out=VMM_PAGE_FLAG_PRESENT;
	if (region->flags&MMAP_REGION_FLAG_VMM_READWRITE){
		out|=VMM_PAGE_FLAG_READWRITE;
	}
	if (region->flags&MMAP_REGION_FLAG_VMM_USER){
		out|=VMM_PAGE_FLAG_USER;
	}
	if (region->flags&MMAP_REGION_FLAG_VMM_NOEXECUTE){
		out|=VMM_PAGE_FLAG_NOEXECUTE;
	}
	return out;
}
