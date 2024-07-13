#include <kernel/drive/drive.h>
#include <kernel/event/device.h>
#include <kernel/format/format.h>
#include <kernel/fs/fs.h>
#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/partition/partition.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/string.h>
#include <kernel/util/memory.h>
#define KERNEL_LOG_NAME "partition"



static omm_allocator_t* KERNEL_INIT_WRITE _partition_allocator=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _partition_table_descriptor_allocator=NULL;

KERNEL_PUBLIC handle_type_t KERNEL_INIT_WRITE partition_handle_type=0;
KERNEL_PUBLIC handle_type_t KERNEL_INIT_WRITE partition_table_descriptor_handle_type=0;



static void _partition_handle_destructor(handle_t* handle){
	partition_t* partition=KERNEL_CONTAINEROF(handle,partition_t,handle);
	event_dispatch_partition_delete_notification(partition);
	if (partition->drive){
		handle_release(&(partition->drive->handle));
	}
	if (partition->descriptor){
		handle_release(&(partition->descriptor->handle));
	}
	smm_dealloc(partition->name);
	omm_dealloc(_partition_allocator,partition);
}



KERNEL_PUBLIC partition_table_descriptor_t* partition_register_table_descriptor(const partition_table_descriptor_config_t* config){
	LOG("Registering partition table descriptor '%s'...",config->name);
	if (!partition_table_descriptor_handle_type){
		partition_table_descriptor_handle_type=handle_alloc("kernel.partition.table_descriptor",0,0);
	}
	if (!_partition_table_descriptor_allocator){
		_partition_table_descriptor_allocator=omm_init("kernel.partition.table_descriptor",sizeof(partition_table_descriptor_t),8,1);
	}
	partition_table_descriptor_t* out=omm_alloc(_partition_table_descriptor_allocator);
	out->config=config;
	handle_new(partition_table_descriptor_handle_type,&(out->handle));
	HANDLE_FOREACH(drive_handle_type){
		drive_t* drive=KERNEL_CONTAINEROF(handle,drive_t,handle);
		if (drive->partition_table_descriptor){
			continue;
		}
		handle_acquire(&(out->handle));
		drive->partition_table_descriptor=out;
		if (config->load_callback(drive)){
			INFO("Detected partitioning of drive '%s' as '%s'",drive->model_number->data,config->name);
		}
		else{
			drive->partition_table_descriptor=NULL;
			handle_release(&(out->handle));
		}
	}
	return out;
}



KERNEL_PUBLIC void partition_unregister_table_descriptor(partition_table_descriptor_t* descriptor){
	LOG("Unregistering partition table descriptor '%s'...",descriptor->config->name);
	handle_destroy(&(descriptor->handle));
	omm_dealloc(_partition_table_descriptor_allocator,descriptor);
}



void partition_load_from_drive(drive_t* drive){
	LOG("Loading partitions from drive '%s'...",drive->model_number->data);
	handle_acquire(&(drive->handle));
	HANDLE_FOREACH(partition_table_descriptor_handle_type){
		partition_table_descriptor_t* descriptor=KERNEL_CONTAINEROF(handle,partition_table_descriptor_t,handle);
		if (!descriptor->config->load_callback){
			continue;
		}
		drive->partition_table_descriptor=descriptor;
		if (descriptor->config->load_callback(drive)){
			INFO("Detected drive partitioning as '%s'",descriptor->config->name);
			// do not release handle; a reference to partition_table_descriptor is stored in the drive structure
			return;
		}
	}
	handle_release(&(drive->handle));
	drive->partition_table_descriptor=NULL;
	WARN("Unable to detect partition type of drive '%s'",drive->model_number->data);
}



KERNEL_PUBLIC partition_t* partition_create(drive_t* drive,u32 index,const char* name,u64 start_lba,u64 end_lba,const u8* guid){
	LOG("Creating partition '%s' on drive '%s'...",name,drive->model_number->data);
	handle_acquire(&(drive->partition_table_descriptor->handle));
	if (!_partition_allocator){
		_partition_allocator=omm_init("kernel.partition",sizeof(partition_t),8,4);
		rwlock_init(&(_partition_allocator->lock));
	}
	if (!partition_handle_type){
		partition_handle_type=handle_alloc("kernel.partition",0,_partition_handle_destructor);
	}
	partition_t* out=omm_alloc(_partition_allocator);
	handle_new(partition_handle_type,&(out->handle));
	out->descriptor=drive->partition_table_descriptor;
	out->drive=drive;
	out->index=index;
	out->name=smm_alloc(name,0);
	out->start_lba=start_lba;
	out->end_lba=end_lba;
	if (guid){
		mem_copy(out->guid,guid,16);
	}
	else{
		mem_fill(out->guid,16,0);
	}
	out->fs=NULL;
	out->fs=fs_load(out);
	if (!out->fs){
		WARN("No filesystem detected on partition '%s%ud%up%u/%s'",drive->type->name,drive->controller_index,drive->device_index,index,name);
	}
	event_dispatch_partition_create_notification(out);
	return out;
}


KERNEL_PUBLIC bool partition_format(drive_t* drive,partition_table_descriptor_t* descriptor){
	return (descriptor->config->format_callback?descriptor->config->format_callback(drive):0);
}



error_t syscall_partition_get_next(handle_id_t partition_handle_id){
	handle_descriptor_t* partition_handle_descriptor=handle_get_descriptor(partition_handle_type);
	rb_tree_node_t* rb_node=rb_tree_lookup_increasing_node(&(partition_handle_descriptor->tree),(partition_handle_id?partition_handle_id+1:0));
	return (rb_node?rb_node->key:0);
}



error_t syscall_partition_get_data(u64 partition_handle_id,KERNEL_USER_POINTER partition_user_data_t* buffer,u32 buffer_length){
	if (buffer_length<sizeof(partition_user_data_t)){
		return ERROR_INVALID_ARGUMENT(2);
	}
	if (syscall_get_user_pointer_max_length((void*)buffer)<buffer_length){
		return ERROR_INVALID_ARGUMENT(1);
	}
	handle_t* partition_handle=handle_lookup_and_acquire(partition_handle_id,partition_handle_type);
	if (!partition_handle){
		return ERROR_INVALID_HANDLE;
	}
	partition_t* partition=KERNEL_CONTAINEROF(partition_handle,partition_t,handle);
	str_copy(partition->name->data,(char*)(buffer->name),sizeof(buffer->name));
	str_copy(partition->descriptor->config->name,(char*)(buffer->type),sizeof(buffer->type));
	buffer->drive=partition->drive->handle.rb_node.key;
	buffer->index=partition->index;
	buffer->start_lba=partition->start_lba;
	buffer->end_lba=partition->end_lba;
	buffer->fs=(partition->fs?partition->fs->handle.rb_node.key:0);
	handle_release(partition_handle);
	return ERROR_OK;
}



error_t syscall_partition_table_descriptor_format(handle_id_t drive_handle_id,handle_id_t partition_table_descriptor_handle_id){
	handle_t* drive_handle=handle_lookup_and_acquire(drive_handle_id,drive_handle_type);
	if (!drive_handle){
		return ERROR_INVALID_HANDLE;
	}
	handle_t* partition_table_descriptor_handle=handle_lookup_and_acquire(partition_table_descriptor_handle_id,partition_table_descriptor_handle_type);
	if (!partition_table_descriptor_handle){
		handle_release(drive_handle);
		return ERROR_INVALID_HANDLE;
	}
	error_t out=(partition_format(KERNEL_CONTAINEROF(drive_handle,drive_t,handle),KERNEL_CONTAINEROF(partition_table_descriptor_handle,partition_table_descriptor_t,handle))?ERROR_OK:ERROR_FAILED);
	handle_release(drive_handle);
	handle_release(partition_table_descriptor_handle);
	return out;
}



error_t syscall_partition_table_descriptor_get_next(handle_id_t partition_table_descriptor_handle_id){
	handle_descriptor_t* partition_table_descriptor_handle_descriptor=handle_get_descriptor(partition_table_descriptor_handle_type);
	rb_tree_node_t* rb_node=rb_tree_lookup_increasing_node(&(partition_table_descriptor_handle_descriptor->tree),(partition_table_descriptor_handle_id?partition_table_descriptor_handle_id+1:0));
	return (rb_node?rb_node->key:0);
}



error_t syscall_partition_table_descriptor_get_data(u64 partition_table_descriptor_handle_id,KERNEL_USER_POINTER partition_table_descriptor_user_data_t* buffer,u32 buffer_length){
	if (buffer_length<sizeof(partition_table_descriptor_user_data_t)){
		return ERROR_INVALID_ARGUMENT(2);
	}
	if (syscall_get_user_pointer_max_length((void*)buffer)<buffer_length){
		return ERROR_INVALID_ARGUMENT(1);
	}
	handle_t* partition_table_descriptor_handle=handle_lookup_and_acquire(partition_table_descriptor_handle_id,partition_table_descriptor_handle_type);
	if (!partition_table_descriptor_handle){
		return ERROR_INVALID_HANDLE;
	}
	partition_table_descriptor_t* partition_table_descriptor=KERNEL_CONTAINEROF(partition_table_descriptor_handle,partition_table_descriptor_t,handle);
	str_copy(partition_table_descriptor->config->name,(char*)(buffer->name),sizeof(buffer->name));
	buffer->flags=0;
	if (partition_table_descriptor->config->format_callback){
		buffer->flags|=PARTITION_TABLE_DESCRIPTOR_FLAG_CAN_FORMAT;
	}
	handle_release(partition_table_descriptor_handle);
	return ERROR_OK;
}
