#include <kernel/drive/drive.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "drive"



static pmm_counter_descriptor_t _drive_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_drive");
static omm_allocator_t* _drive_allocator=NULL;

handle_type_t drive_handle_type=0;
handle_type_t drive_type_handle_type=0;



static void _drive_handle_destructor(handle_t* handle){
	drive_t* drive=handle->object;
	WARN("Delete drive: %s%ud%u",drive->type->name,drive->controller_index,drive->device_index);
	handle_release(&(drive->type->handle));
	if (drive->partition_table_descriptor){
		handle_release(&(drive->partition_table_descriptor->handle));
	}
	omm_dealloc(_drive_allocator,drive);
}



void drive_register_type(drive_type_t* type){
	LOG("Registering drive type '%s'...",type->name);
	if (!drive_type_handle_type){
		drive_type_handle_type=handle_alloc("drive_type",NULL);
	}
	handle_new(type,drive_type_handle_type,&(type->handle));
	handle_finish_setup(&(type->handle));
}



void drive_unregister_type(drive_type_t* type){
	LOG("Unregistering drive type '%s'...",type->name);
	handle_destroy(&(type->handle));
}



drive_t* drive_create(const drive_config_t* config){
	if (!_drive_allocator){
		_drive_allocator=omm_init("drive",sizeof(drive_t),8,4,&_drive_omm_pmm_counter);
		spinlock_init(&(_drive_allocator->lock));
	}
	if (!drive_handle_type){
		drive_handle_type=handle_alloc("drive",_drive_handle_destructor);
	}
	handle_acquire(&(config->type->handle));
	LOG("Creating drive '%s%ud%u' as '%s/%s'...",config->type->name,config->controller_index,config->device_index,config->model_number->data,config->serial_number->data);
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
	if (out->block_size&(out->block_size-1)){
		WARN("Drive block size is not a power of 2");
	}
	partition_load_from_drive(out);
	handle_finish_setup(&(out->handle));
	return out;
}



u64 drive_read(drive_t* drive,u64 offset,void* buffer,u64 size){
	return drive->type->io_callback(drive,offset&DRIVE_OFFSET_MASK,buffer,size);
}



u64 drive_write(drive_t* drive,u64 offset,const void* buffer,u64 size){
	return drive->type->io_callback(drive,(offset&DRIVE_OFFSET_MASK)|DRIVE_OFFSET_FLAG_WRITE,(void*)buffer,size);
}
