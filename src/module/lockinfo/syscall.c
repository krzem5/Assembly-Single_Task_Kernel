#include <kernel/lock/profiling.h>
#include <kernel/log/log.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "lockinfo"



typedef struct _LOCKINFO_USER_DATA_DESCRIPTOR{
	u32 type_line;
	u32 line;
	char func[64];
	char type_func[64];
	char name[128];
	char type_name[128];
	u64 count;
	u64 ticks;
	u64 max_ticks;
} lockinfo_user_data_descriptor_t;



static _Bool _syscall_lockinfo_get_data_descriptor(u32 index,u32 type_index,KERNEL_USER lockinfo_user_data_descriptor_t* out){
	if (type_index>=LOCK_PROFILING_MAX_LOCK_TYPES||!(lock_profiling_type_descriptors+type_index)->func||syscall_get_user_pointer_max_length((void*)out)<sizeof(lockinfo_user_data_descriptor_t)){
		return 0;
	}
	const lock_profiling_data_descriptor_t* descriptor=lock_profiling_data_descriptor_head;
	for (;index;index--){
		if (!descriptor){
			return 0;
		}
		descriptor=descriptor->next;
	}
	if (!descriptor||!descriptor->func){
		return 0;
	}
	out->line=descriptor->line;
	out->type_line=(lock_profiling_type_descriptors+type_index)->line;
	strcpy((char*)(out->func),descriptor->func,64);
	strcpy((char*)(out->type_func),(lock_profiling_type_descriptors+type_index)->func,64);
	strcpy((char*)(out->name),descriptor->arg,128);
	strcpy((char*)(out->type_name),((lock_profiling_type_descriptors+type_index)->arg?(lock_profiling_type_descriptors+type_index)->arg:""),128);
	out->count=(descriptor->data+type_index)->count;
	out->ticks=(descriptor->data+type_index)->ticks;
	out->max_ticks=(descriptor->data+type_index)->max_ticks;
	return 1;
}



static syscall_callback_t const _lockinfo_syscall_functions[]={
	[1]=(syscall_callback_t)_syscall_lockinfo_get_data_descriptor,
};



_Bool lockinfo_syscall_init(void){
	if (!lock_profiling_type_descriptors||!lock_profiling_data_descriptor_head){
		WARN("No lock profiling data present");
		return 0;
	}
	LOG("Initializing lockinfo syscalls...");
	syscall_create_table("lockinfo",_lockinfo_syscall_functions,sizeof(_lockinfo_syscall_functions)/sizeof(syscall_callback_t));
	return 1;
}
