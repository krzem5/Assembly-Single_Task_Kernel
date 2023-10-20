#include <kernel/handle/handle.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "omm"



HANDLE_DECLARE_TYPE(OMM_ALLOCATOR,{
	panic("Unable to delete HANDLE_TYPE_OMM_ALLOCATOR");
});



static void _allocator_add_page(omm_page_header_t** list_head,omm_page_header_t* page){
	page->prev=NULL;
	page->next=*list_head;
	if (*list_head){
		(*list_head)->prev=page;
	}
	*list_head=page;
}



static void _allocator_remove_page(omm_page_header_t** list_head,omm_page_header_t* page){
	if (page->prev){
		page->prev->next=page->next;
	}
	else{
		*list_head=page->next;
	}
	if (page->next){
		page->next->prev=page->prev;
	}
}



void* omm_alloc(omm_allocator_t* allocator){
	scheduler_pause();
	if (!allocator->handle.rb_node.key){
		handle_new(allocator,HANDLE_TYPE_OMM_ALLOCATOR,&(allocator->handle));
	}
	if (allocator->object_size<sizeof(omm_object_t)){
		allocator->object_size=sizeof(omm_object_t);
	}
	if (allocator->alignment&(allocator->alignment-1)){
		panic("omm_allocator_t alignment must be a power of 2");
	}
	if (allocator->page_count&(allocator->page_count-1)){
		panic("omm_allocator_t page_count must be a power of 2");
	}
	lock_acquire_exclusive(&(allocator->lock));
	omm_page_header_t* page=(allocator->page_used_head?allocator->page_used_head:allocator->page_free_head);
	if (!page){
		u64 page_address=pmm_alloc(allocator->page_count,*(allocator->pmm_counter),0)+VMM_HIGHER_HALF_ADDRESS_OFFSET;
		omm_object_t* head=NULL;
		for (u64 i=(sizeof(omm_page_header_t)+allocator->alignment-1)&(-((u64)(allocator->alignment)));i+allocator->object_size<=(allocator->page_count<<PAGE_SIZE_SHIFT);i+=allocator->object_size){
			omm_object_t* object=(void*)(page_address+i);
			object->next=head;
			head=object;
		}
		page=(void*)page_address;
		_allocator_add_page(&(allocator->page_free_head),page);
		page->head=head;
		page->object_size=allocator->object_size;
		page->used_count=0;
	}
	void* out=page->head;
	page->head=page->head->next;
	page->used_count++;
	if (page->used_count==(allocator->max_used_count>>1)){
		_allocator_remove_page(&(allocator->page_free_head),page);
		_allocator_add_page(&(allocator->page_used_head),page);
	}
	else if (page->used_count==allocator->max_used_count){
		_allocator_remove_page(&(allocator->page_used_head),page);
		_allocator_add_page(&(allocator->page_full_head),page);
	}
	allocator->allocation_count++;
	lock_release_exclusive(&(allocator->lock));
	scheduler_resume();
	return out;
}



void omm_dealloc(omm_allocator_t* allocator,void* object){
	scheduler_pause();
	lock_acquire_exclusive(&(allocator->lock));
	omm_page_header_t* page=(void*)(((u64)object)&(-(((u64)(allocator->page_count))<<PAGE_SIZE_SHIFT)));
	if (page->object_size!=allocator->object_size){
		panic("omm_dealloc: wrong allocator");
	}
	omm_object_t* head=object;
	head->next=page->head;
	page->head=head;
	if (page->used_count==allocator->max_used_count){
		_allocator_remove_page(&(allocator->page_full_head),page);
		_allocator_add_page(&(allocator->page_used_head),page);
	}
	else if (page->used_count==(allocator->max_used_count>>1)){
		_allocator_remove_page(&(allocator->page_used_head),page);
		_allocator_add_page(&(allocator->page_free_head),page);
	}
	page->used_count--;
	if (!page->used_count){
		_allocator_remove_page(&(allocator->page_free_head),page);
		pmm_dealloc(((u64)page)-VMM_HIGHER_HALF_ADDRESS_OFFSET,allocator->page_count,*(allocator->pmm_counter));
	}
	allocator->deallocation_count++;
	lock_release_exclusive(&(allocator->lock));
	scheduler_resume();
}
