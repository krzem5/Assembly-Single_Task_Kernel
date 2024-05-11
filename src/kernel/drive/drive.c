#include <kernel/drive/drive.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/partition/partition.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/util/string.h>
#define KERNEL_LOG_NAME "drive"



static pmm_counter_descriptor_t* _drive_buffer_pmm_counter=NULL;
static omm_allocator_t* _drive_allocator=NULL;

KERNEL_PUBLIC handle_type_t drive_handle_type=0;



static void _drive_handle_destructor(handle_t* handle){
	drive_t* drive=handle->object;
	if (drive->partition_table_descriptor){
		handle_release(&(drive->partition_table_descriptor->handle));
	}
	smm_dealloc(drive->serial_number);
	smm_dealloc(drive->model_number);
	omm_dealloc(_drive_allocator,drive);
}



KERNEL_EARLY_EARLY_INIT(){
	LOG("Initializing drive allocator...");
	_drive_buffer_pmm_counter=pmm_alloc_counter("kernel.drive.buffer");
	_drive_allocator=omm_init("kernel.drive",sizeof(drive_t),8,4);
	rwlock_init(&(_drive_allocator->lock));
	drive_handle_type=handle_alloc("kernel.drive",_drive_handle_destructor);
}



KERNEL_PUBLIC drive_t* drive_create(const drive_config_t* config){
	LOG("Creating drive '%s%ud%u' as '%s/%s'...",config->type->name,config->controller_index,config->device_index,config->model_number->data,config->serial_number->data);
	if (config->block_size&(config->block_size-1)){
		ERROR("Drive block size is not a power of 2");
		return NULL;
	}
	drive_t* out=omm_alloc(_drive_allocator);
	handle_new(out,drive_handle_type,&(out->handle));
	out->type=config->type;
	out->block_size_shift=__builtin_ctzll(config->block_size);
	out->controller_index=config->controller_index;
	out->device_index=config->device_index;
	out->serial_number=config->serial_number;
	out->model_number=config->model_number;
	out->block_count=config->block_count;
	out->block_size=config->block_size;
	out->extra_data=config->extra_data;
	out->partition_table_descriptor=NULL;
	INFO("Drive size: %v (%lu * %lu)",out->block_count*out->block_size,out->block_count,out->block_size);
	partition_load_from_drive(out);
	handle_finish_setup(&(out->handle));
	return out;
}



KERNEL_PUBLIC u64 drive_read(drive_t* drive,u64 offset,void* buffer,u64 size){
	if (offset>=drive->block_count){
		return 0;
	}
	if (offset+size>drive->block_count){
		size=drive->block_count-offset;
	}
	if (!size){
		return 0;
	}
	u64 aligned_buffer=pmm_alloc(pmm_align_up_address(size<<drive->block_size_shift)>>PAGE_SIZE_SHIFT,_drive_buffer_pmm_counter,0);
	u64 out=drive->type->io_callback(drive,offset&DRIVE_OFFSET_MASK,aligned_buffer,size);
	mem_copy(buffer,(void*)(aligned_buffer+VMM_HIGHER_HALF_ADDRESS_OFFSET),out<<drive->block_size_shift);
	pmm_dealloc(aligned_buffer,pmm_align_up_address(size<<drive->block_size_shift)>>PAGE_SIZE_SHIFT,_drive_buffer_pmm_counter);
	return out;
}



KERNEL_PUBLIC u64 drive_write(drive_t* drive,u64 offset,const void* buffer,u64 size){
	if ((drive->type->flags&DRIVE_TYPE_FLAG_READ_ONLY)||offset>=drive->block_count){
		return 0;
	}
	if (offset+size>drive->block_count){
		size=drive->block_count-offset;
	}
	if (!size){
		return 0;
	}
	u64 aligned_buffer=pmm_alloc(pmm_align_up_address(size<<drive->block_size_shift)>>PAGE_SIZE_SHIFT,_drive_buffer_pmm_counter,0);
	mem_copy((void*)(aligned_buffer+VMM_HIGHER_HALF_ADDRESS_OFFSET),buffer,size<<drive->block_size_shift);
	u64 out=drive->type->io_callback(drive,(offset&DRIVE_OFFSET_MASK)|DRIVE_OFFSET_FLAG_WRITE,aligned_buffer,size);
	pmm_dealloc(aligned_buffer,pmm_align_up_address(size<<drive->block_size_shift)>>PAGE_SIZE_SHIFT,_drive_buffer_pmm_counter);
	return out;
}



error_t syscall_drive_get_next(handle_id_t drive_handle_id){
	handle_descriptor_t* drive_handle_descriptor=handle_get_descriptor(drive_handle_type);
	rb_tree_node_t* rb_node=rb_tree_lookup_increasing_node(&(drive_handle_descriptor->tree),(drive_handle_id?drive_handle_id+1:0));
	return (rb_node?rb_node->key:0);
}



error_t syscall_drive_get_data(u64 drive_handle_id,KERNEL_USER_POINTER drive_user_data_t* buffer,u32 buffer_length){
	if (buffer_length<sizeof(drive_user_data_t)){
		return ERROR_INVALID_ARGUMENT(2);
	}
	if (syscall_get_user_pointer_max_length((void*)buffer)<buffer_length){
		return ERROR_INVALID_ARGUMENT(1);
	}
	handle_t* drive_handle=handle_lookup_and_acquire(drive_handle_id,drive_handle_type);
	if (!drive_handle){
		return ERROR_INVALID_HANDLE;
	}
	drive_t* drive=drive_handle->object;
	str_copy(drive->type->name,(char*)(buffer->type),sizeof(buffer->type));
	buffer->controller_index=drive->controller_index;
	buffer->device_index=drive->device_index;
	str_copy(drive->serial_number->data,(char*)(buffer->serial_number),sizeof(buffer->serial_number));
	str_copy(drive->model_number->data,(char*)(buffer->model_number),sizeof(buffer->model_number));
	buffer->block_count=drive->block_count;
	buffer->block_size=drive->block_size;
	str_copy((drive->partition_table_descriptor?drive->partition_table_descriptor->config->name:""),(char*)(buffer->partition_table_type),sizeof(buffer->partition_table_type));
	handle_release(drive_handle);
	return ERROR_OK;
}

