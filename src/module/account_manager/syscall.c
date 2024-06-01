#include <account_manager/database.h>
#include <kernel/error/error.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/module/module.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#define KERNEL_LOG_NAME "account_manager_syscall"



static error_t _syscall_iter_group(u32 gid){
	return account_manager_database_iter_next_group(gid);
}



static error_t _syscall_iter_user(u32 uid){
	return account_manager_database_iter_next_user(uid);
}



static error_t _syscall_iter_user_group(u32 uid,u32 gid){
	return account_manager_database_iter_next_user_subgroup(uid,gid);
}



static error_t _syscall_get_group_data(u32 gid,KERNEL_USER_POINTER account_manager_database_group_data_t* buffer,u32 buffer_length){
	if (buffer_length<sizeof(account_manager_database_group_data_t)){
		return ERROR_INVALID_ARGUMENT(2);
	}
	if (syscall_get_user_pointer_max_length((void*)buffer)<buffer_length){
		return ERROR_INVALID_ARGUMENT(1);
	}
	account_manager_database_group_data_t group_data;
	error_t err=account_manager_database_get_group_data(gid,&group_data);
	if (err!=ERROR_OK){
		return err;
	}
	buffer->gid=group_data.gid;
	return ERROR_OK;
}



static error_t _syscall_get_user_data(u32 uid,KERNEL_USER_POINTER account_manager_database_user_data_t* buffer,u32 buffer_length){
	if (buffer_length<sizeof(account_manager_database_user_data_t)){
		return ERROR_INVALID_ARGUMENT(2);
	}
	if (syscall_get_user_pointer_max_length((void*)buffer)<buffer_length){
		return ERROR_INVALID_ARGUMENT(1);
	}
	account_manager_database_user_data_t user_data;
	error_t err=account_manager_database_get_user_data(uid,&user_data);
	if (err!=ERROR_OK){
		return err;
	}
	buffer->uid=user_data.uid;
	buffer->flags=user_data.flags;
	return ERROR_OK;
}



static error_t _syscall_create_group(u32 gid,KERNEL_USER_POINTER const char* name){
	if (!process_is_root()){
		return ERROR_DENIED;
	}
	u32 name_length=syscall_get_string_length((const void*)name);
	if (!name_length){
		return ERROR_INVALID_ARGUMENT(1);
	}
	char* name_buffer=amm_alloc(name_length+1);
	mem_copy(name_buffer,(const void*)name,name_length);
	name_buffer[name_length]=0;
	error_t out=account_manager_database_create_group(gid,name_buffer);
	amm_dealloc(name_buffer);
	return out;
}



static error_t _syscall_create_user(u32 uid,KERNEL_USER_POINTER const char* name,KERNEL_USER_POINTER const void* password,u32 password_length){
	if (!process_is_root()){
		return ERROR_DENIED;
	}
	u32 name_length=syscall_get_string_length((const void*)name);
	if (!name_length){
		return ERROR_INVALID_ARGUMENT(1);
	}
	if (syscall_get_user_pointer_max_length((const void*)password)<password_length){
		return ERROR_INVALID_ARGUMENT(2);
	}
	char* name_buffer=amm_alloc(name_length+1);
	mem_copy(name_buffer,(const void*)name,name_length);
	name_buffer[name_length]=0;
	char* password_buffer=amm_alloc(password_length+1);
	mem_copy(password_buffer,(const void*)password,password_length);
	password_buffer[password_length]=0;
	error_t out=account_manager_database_create_user(uid,name_buffer,password_buffer,password_length);
	amm_dealloc(name_buffer);
	amm_dealloc(password_buffer);
	return out;
}



static error_t _syscall_delete_group(u32 gid){
	if (!process_is_root()){
		return ERROR_DENIED;
	}
	ERROR("_syscall_delete_group");
	return ERROR_DENIED;
}



static error_t _syscall_delete_user(u32 uid){
	if (!process_is_root()){
		return ERROR_DENIED;
	}
	ERROR("_syscall_delete_user");
	return ERROR_DENIED;
}



static error_t _syscall_join_group(u32 uid,u32 gid){
	if (!process_is_root()){
		return ERROR_DENIED;
	}
	ERROR("_syscall_join_group");
	return ERROR_DENIED;
}



static error_t _syscall_leave_group(u32 uid,u32 gid){
	if (!process_is_root()){
		return ERROR_DENIED;
	}
	ERROR("_syscall_leave_group");
	return ERROR_DENIED;
}



static error_t _syscall_switch_user(handle_id_t process_handle,u32 uid,KERNEL_USER_POINTER const void* password,u32 password_length){
	if (!process_handle){
		process_handle=THREAD_DATA->process->handle.rb_node.key;
	}
	handle_t* handle=handle_lookup_and_acquire(process_handle,process_handle_type);
	if (!handle){
		return ERROR_INVALID_HANDLE;
	}
	if (!handle->acl||!(acl_get(handle->acl,THREAD_DATA->process)&PROCESS_ACL_FLAG_SWITCH_USER)){
		handle_release(handle);
		return ERROR_DENIED;
	}
	process_t* process=KERNEL_CONTAINEROF(handle,process_t,handle);
	ERROR("_syscall_switch_user");(void)process;
	handle_release(handle);
	return ERROR_DENIED;
}



static syscall_callback_t const _account_manager_syscall_functions[]={
	[1]=(syscall_callback_t)_syscall_iter_group,
	[2]=(syscall_callback_t)_syscall_iter_user,
	[3]=(syscall_callback_t)_syscall_iter_user_group,
	[4]=(syscall_callback_t)_syscall_get_group_data,
	[5]=(syscall_callback_t)_syscall_get_user_data,
	[6]=(syscall_callback_t)_syscall_create_group,
	[7]=(syscall_callback_t)_syscall_create_user,
	[8]=(syscall_callback_t)_syscall_delete_group,
	[9]=(syscall_callback_t)_syscall_delete_user,
	[10]=(syscall_callback_t)_syscall_join_group,
	[11]=(syscall_callback_t)_syscall_leave_group,
	[12]=(syscall_callback_t)_syscall_switch_user,
};



MODULE_POSTPOSTINIT(){
	LOG("Initializing account manager syscalls...");
	syscall_create_table("account_manager",_account_manager_syscall_functions,sizeof(_account_manager_syscall_functions)/sizeof(syscall_callback_t));
}
