#include <kernel/lock/profiling.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/symbol/symbol.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/string.h>
#define KERNEL_LOG_NAME "lockinfo"



typedef struct _LOCKINFO_USER_DESCRIPTOR{
	u32 id;
	u32 flags;
	char module[64];
	char func[128];
	u64 offset;
} lockinfo_user_descriptor_t;



typedef struct _LOCKINFO_USER_STATS{
	u32 id;
	char module[64];
	char func[128];
	u64 offset;
	u64 count;
	u64 ticks;
	u64 max_ticks;
} lockinfo_user_stats_t;



static bool _syscall_lockinfo_get_descriptor(u32 index,KERNEL_USER_POINTER lockinfo_user_descriptor_t* out){
	if (syscall_get_user_pointer_max_length((void*)out)<sizeof(lockinfo_user_descriptor_t)){
		return 0;
	}
	lock_profiling_descriptor_t descriptor;
	if (!lock_profiling_get_descriptor(index,&descriptor)){
		return 0;
	}
	u64 full_address=descriptor.address|0xffffffff00000000ull;
	const symbol_t* symbol=symbol_lookup(full_address);
	if (!symbol){
		return 0;
	}
	out->id=descriptor.id;
	out->flags=descriptor.flags;
	str_copy(symbol->module,(char*)(out->module),sizeof(out->module));
	str_copy(symbol->name->data,(char*)(out->func),sizeof(out->func));
	out->offset=full_address-symbol->rb_node.key;
	return 1;
}



static bool _syscall_lockinfo_get_stats(u32 index,KERNEL_USER_POINTER lockinfo_user_stats_t* out){
	if (syscall_get_user_pointer_max_length((void*)out)<sizeof(lockinfo_user_stats_t)){
		return 0;
	}
	lock_profiling_stats_t stats;
	if (!lock_profiling_get_stats(index,&stats)){
		return 0;
	}
	u64 full_address=stats.address|0xffffffff00000000ull;
	const symbol_t* symbol=symbol_lookup(full_address);
	if (!symbol){
		return 0;
	}
	out->id=stats.id;
	str_copy(symbol->module,(char*)(out->module),sizeof(out->module));
	str_copy(symbol->name->data,(char*)(out->func),sizeof(out->func));
	out->offset=full_address-symbol->rb_node.key;
	out->count=stats.count;
	out->ticks=stats.ticks;
	out->max_ticks=stats.max_ticks;
	return 1;
}



static syscall_callback_t const _lockinfo_syscall_functions[]={
	[1]=(syscall_callback_t)_syscall_lockinfo_get_descriptor,
	[2]=(syscall_callback_t)_syscall_lockinfo_get_stats,
};



MODULE_INIT(){
	lock_profiling_descriptor_t tmp;
	if (!lock_profiling_get_descriptor(0,&tmp)){
		module_unload(module_self);
		return;
	}
	LOG("Initializing lockinfo syscalls...");
	syscall_create_table("lockinfo",_lockinfo_syscall_functions,sizeof(_lockinfo_syscall_functions)/sizeof(syscall_callback_t));
}
