#include <kernel/drive/drive.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "drive_cache"



#define DRIVE_CACHE_LOOKUP_TABLE_MASK 0xffffff



// static omm_allocator_t* _drive_allocator=NULL;
static u64* _drive_cache_lookup_table=NULL;



static inline u64 _create_key(drive_t* drive,u64 address){
	return address^(HANDLE_ID_GET_INDEX(drive->handle.rb_node.key)<<40);
}



static inline u16 _create_small_key(drive_t* drive,u64 address){
	address^=HANDLE_ID_GET_INDEX(drive->handle.rb_node.key);
	address^=address>>32;
	return (address^(address>>16))&DRIVE_CACHE_LOOKUP_TABLE_MASK;
}



KERNEL_EARLY_EARLY_INIT(){
	LOG("Initializing drive cache...");
	_drive_cache_lookup_table=(void*)(pmm_alloc(pmm_align_up_address((DRIVE_CACHE_LOOKUP_TABLE_MASK+1)*sizeof(u64))>>PAGE_SIZE_SHIFT,pmm_alloc_counter("drive_cache"),0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	// _drive_allocator=omm_init("drive",sizeof(drive_t),8,4,pmm_alloc_counter("omm_drive"));
	// spinlock_init(&(_drive_allocator->lock));
	// drive_handle_type=handle_alloc("drive",_drive_handle_destructor);
}



u64 drive_cache_get(drive_t* drive,u64 offset,void* buffer,u64 size){
	u16 small_key=_create_small_key(drive,offset);
	u64 key=_create_key(drive,offset);
	if (_drive_cache_lookup_table[small_key]&&_drive_cache_lookup_table[small_key]!=key){
		WARN("Collision %p | %p",_drive_cache_lookup_table[small_key],key);
	}
	_drive_cache_lookup_table[small_key]=key;
	return 0;
}



void drive_cache_set(drive_t* drive,u64 offset,const void* buffer,u64 size){
	return;
}
