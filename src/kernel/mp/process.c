#include <kernel/acl/acl.h>
#include <kernel/aslr/aslr.h>
#include <kernel/cpu/cpu.h>
#include <kernel/elf/elf.h>
#include <kernel/error/error.h>
#include <kernel/event/process.h>
#include <kernel/fd/fd.h>
#include <kernel/format/format.h>
#include <kernel/handle/handle.h>
#include <kernel/handle/handle_list.h>
#include <kernel/id/flags.h>
#include <kernel/id/user.h>
#include <kernel/kernel.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/mmap/mmap.h>
#include <kernel/mp/event.h>
#include <kernel/mp/process.h>
#include <kernel/mp/process_group.h>
#include <kernel/mp/thread.h>
#include <kernel/mp/thread_list.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/signal/signal.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/util/util.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "process"



#define KERNELSPACE_LOWEST_ADDRESS 0xfffff00000000000ull

#define ASLR_KERNEL_MMAP_TOP_MIN 0xffffffff80000000ull // -2 GB
#define ASLR_KERNEL_MMAP_TOP_MAX 0xffffffffc0000000ull // -1 GB



typedef struct _SYSCALL_PROCESS_START_EXTRA_DATA{
	u64 stdin;
	u64 stdout;
	u64 stderr;
} syscall_process_start_extra_data_t;



static omm_allocator_t* KERNEL_INIT_WRITE _process_allocator=NULL;

KERNEL_PUBLIC handle_type_t KERNEL_INIT_WRITE process_handle_type;
KERNEL_PUBLIC process_t* KERNEL_INIT_WRITE process_kernel;



static void _process_handle_destructor(handle_t* handle){
	process_t* process=KERNEL_CONTAINEROF(handle,process_t,handle);
	if (process->thread_list.head){
		panic("Unterminated process not referenced");
	}
	event_dispatch_process_delete_notification(process);
	vfs_node_unref(process->vfs_root);
	vfs_node_unref(process->vfs_cwd);
	fd_unref(process->fd_stdin);
	fd_unref(process->fd_stdout);
	fd_unref(process->fd_stderr);
	process_group_leave(process->process_group,process);
	handle_list_destroy(&(process->handle_list));
	mmap_deinit(process->mmap);
	vmm_pagemap_deinit(&(process->pagemap));
	omm_dealloc(_process_allocator,process);
}



KERNEL_EARLY_INIT(){
	LOG("Creating kernel process...");
	_process_allocator=omm_init("kernel.process",sizeof(process_t),8,2);
	rwlock_init(&(_process_allocator->lock));
	process_handle_type=handle_alloc("kernel.process",HANDLE_DESCRIPTOR_FLAG_ALLOW_CONTAINER,_process_handle_destructor);
	process_kernel=omm_alloc(_process_allocator);
	handle_new(process_handle_type,&(process_kernel->handle));
	handle_acquire(&(process_kernel->handle));
	process_kernel->handle.acl=acl_create();
	rwlock_init(&(process_kernel->lock));
	vmm_pagemap_init(&(process_kernel->pagemap));
	u64 mmap_top=aslr_generate_address(ASLR_KERNEL_MMAP_TOP_MIN,ASLR_KERNEL_MMAP_TOP_MAX);
	INFO("Kernel memory map range: %p - %p",KERNELSPACE_LOWEST_ADDRESS+mmap_top-ASLR_KERNEL_MMAP_TOP_MIN,mmap_top);
	process_kernel->mmap=mmap_init(&vmm_kernel_pagemap,KERNELSPACE_LOWEST_ADDRESS+mmap_top-ASLR_KERNEL_MMAP_TOP_MIN,mmap_top);
	thread_list_init(&(process_kernel->thread_list));
	process_kernel->name=smm_alloc("kernel",0);
	process_kernel->image=smm_alloc("/boot/kernel",0);
	process_kernel->uid=0;
	process_kernel->gid=0;
	process_kernel->event=event_create("kernel.process.termination",NULL);
	handle_list_init(&(process_kernel->handle_list));
	process_kernel->vfs_root=vfs_get_root_node();
	process_kernel->vfs_cwd=vfs_get_root_node();
	process_kernel->fd_stdin=0;
	process_kernel->fd_stdout=0;
	process_kernel->fd_stderr=0;
	process_kernel->parent=process_kernel;
	process_kernel->main_thread=NULL;
	process_kernel->return_value=NULL;
	process_kernel->process_group=NULL;
	process_group_create(process_kernel);
	signal_process_state_init(&(process_kernel->signal_state));
}



KERNEL_PUBLIC process_t* process_create(const char* image,const char* name,u64 mmap_bottom_address,u64 mmap_top_address){
	process_t* out=omm_alloc(_process_allocator);
	handle_new(process_handle_type,&(out->handle));
	handle_acquire(&(out->handle));
	out->handle.acl=acl_create();
	if (CPU_HEADER_DATA->current_thread){
		acl_set(out->handle.acl,THREAD_DATA->process,0,PROCESS_ACL_FLAG_CREATE_THREAD|PROCESS_ACL_FLAG_TERMINATE|PROCESS_ACL_FLAG_SWITCH_USER);
	}
	acl_set(out->handle.acl,out,0,PROCESS_ACL_FLAG_CREATE_THREAD|PROCESS_ACL_FLAG_TERMINATE|PROCESS_ACL_FLAG_SWITCH_USER);
	rwlock_init(&(out->lock));
	vmm_pagemap_init(&(out->pagemap));
	out->mmap=mmap_init(&(out->pagemap),mmap_bottom_address,mmap_top_address);
	thread_list_init(&(out->thread_list));
	out->name=smm_alloc(name,0);
	out->image=smm_alloc(image,0);
	out->event=event_create("kernel.process.termination",NULL);
	handle_list_init(&(out->handle_list));
	if (THREAD_DATA->header.current_thread){
		out->vfs_root=THREAD_DATA->process->vfs_root;
		out->vfs_cwd=THREAD_DATA->process->vfs_cwd;
		vfs_node_ref(THREAD_DATA->process->vfs_root);
		vfs_node_ref(THREAD_DATA->process->vfs_cwd);
	}
	else{
		process_kernel->vfs_root=vfs_get_root_node();
		process_kernel->vfs_cwd=vfs_get_root_node();
	}
	out->fd_stdin=0;
	out->fd_stdout=0;
	out->fd_stderr=0;
	out->parent=(THREAD_DATA->header.current_thread?THREAD_DATA->process:process_kernel);
	out->uid=out->parent->uid;
	out->gid=out->parent->gid;
	out->main_thread=NULL;
	out->return_value=NULL;
	out->process_group=NULL;
	if (THREAD_DATA->header.current_thread&&THREAD_DATA->process!=process_kernel){
		process_group_join(THREAD_DATA->process->process_group,out);
	}
	else{
		handle_release(&(process_group_create(out)->handle));
	}
	signal_process_state_init(&(process_kernel->signal_state));
	event_dispatch_process_create_notification(out);
	return out;
}



KERNEL_PUBLIC bool process_is_root(void){
	return !THREAD_DATA->process->uid||!THREAD_DATA->process->gid||uid_has_group(THREAD_DATA->process->uid,0)==1;
}



KERNEL_PUBLIC id_flags_t process_get_id_flags(void){
	return uid_get_flags(THREAD_DATA->process->uid)|gid_get_flags(THREAD_DATA->process->gid);
}



error_t syscall_process_get_pid(void){
	return THREAD_DATA->process->handle.rb_node.key;
}



error_t syscall_process_start(KERNEL_USER_POINTER const char* path,u32 argc,KERNEL_USER_POINTER const char*const* argv,KERNEL_USER_POINTER const char*const* environ,u32 flags,KERNEL_USER_POINTER const syscall_process_start_extra_data_t* user_extra_data){
	if (!syscall_get_string_length((const char*)path)){
		return ERROR_INVALID_ARGUMENT(0);
	}
	if (argc*sizeof(const char*)>syscall_get_user_pointer_max_length((const char*const*)argv)){
		return ERROR_INVALID_ARGUMENT(1);
	}
	if (user_extra_data&&syscall_get_user_pointer_max_length((const void*)user_extra_data)<sizeof(syscall_process_start_extra_data_t)){
		return ERROR_INVALID_ARGUMENT(5);
	}
	syscall_process_start_extra_data_t extra_data={
		0,
		0,
		0
	};
	if (user_extra_data){
		extra_data=*user_extra_data;
	}
	char** kernel_argv=amm_alloc(argc*sizeof(char*));
	char** kernel_environ=NULL;
	u64 kernel_environ_length=0;
	error_t out=ERROR_OK;
	for (u64 i=0;i<argc;i++){
		u64 length=syscall_get_string_length((const char*)(argv[i]));
		if (!length){
			argc=i;
			out=ERROR_INVALID_ARGUMENT(2);
			goto _cleanup;
		}
		kernel_argv[i]=amm_alloc(length+1);
		mem_copy(kernel_argv[i],(const char*)(argv[i]),length);
		kernel_argv[i][length]=0;
	}
	if (environ){
		u64 max_length=syscall_get_user_pointer_max_length(environ)/sizeof(const char*);
		for (;environ[kernel_environ_length]&&kernel_environ_length<max_length;kernel_environ_length++){
			u64 length=syscall_get_string_length((const char*)(environ[kernel_environ_length]));
			if (!length){
				out=ERROR_INVALID_ARGUMENT(3);
				goto _cleanup;
			}
			kernel_environ=amm_realloc(kernel_environ,(kernel_environ_length+1)*sizeof(char*));
			kernel_environ[kernel_environ_length]=amm_alloc(length+1);
			mem_copy(kernel_environ[kernel_environ_length],(const char*)(environ[kernel_environ_length]),length);
			kernel_environ[kernel_environ_length][length]=0;
		}
		if (kernel_environ_length==max_length){
			out=ERROR_INVALID_ARGUMENT(3);
			goto _cleanup;
		}
	}
	out=elf_load((const char*)path,argc,(const char*const*)kernel_argv,kernel_environ_length,(const char*const*)kernel_environ,ELF_LOAD_FLAG_PAUSE_THREAD);
	if (IS_ERROR(out)){
		goto _cleanup;
	}
	handle_t* handle=handle_lookup_and_acquire(out,process_handle_type);
	if (!handle){
		out=ERROR_INVALID_HANDLE;
		goto _cleanup;
	}
	process_t* process=KERNEL_CONTAINEROF(handle,process_t,handle);
	process->fd_stdin=(extra_data.stdin?extra_data.stdin:THREAD_DATA->process->fd_stdin);
	process->fd_stdout=(extra_data.stdout?extra_data.stdout:THREAD_DATA->process->fd_stdout);
	process->fd_stderr=(extra_data.stderr?extra_data.stderr:THREAD_DATA->process->fd_stderr);
	fd_ref(process->fd_stdin);
	fd_ref(process->fd_stdout);
	fd_ref(process->fd_stderr);
	fd_allow_dup(process->fd_stdin,process);
	fd_allow_dup(process->fd_stdout,process);
	fd_allow_dup(process->fd_stderr,process);
	if (!(flags&ELF_LOAD_FLAG_PAUSE_THREAD)){
		scheduler_enqueue_thread(process->main_thread);
	}
	handle_release(handle);
_cleanup:
	for (u64 i=0;i<argc;i++){
		amm_dealloc(kernel_argv[i]);
	}
	amm_dealloc(kernel_argv);
	for (u64 i=0;i<kernel_environ_length;i++){
		amm_dealloc(kernel_environ[i]);
	}
	amm_dealloc(kernel_environ);
	return out;
}



error_t syscall_process_get_event(handle_id_t process_handle){
	if (!process_handle){
		process_handle=THREAD_DATA->process->handle.rb_node.key;
	}
	handle_t* handle=handle_lookup_and_acquire(process_handle,process_handle_type);
	if (!handle){
		return ERROR_INVALID_HANDLE;
	}
	process_t* process=KERNEL_CONTAINEROF(handle,process_t,handle);
	u64 out=process->event->handle.rb_node.key;
	handle_release(handle);
	return out;
}



error_t syscall_process_set_cwd(handle_id_t process_handle,handle_id_t fd){
	if (!process_handle){
		process_handle=THREAD_DATA->process->handle.rb_node.key;
	}
	handle_t* handle=handle_lookup_and_acquire(process_handle,process_handle_type);
	if (!handle){
		return ERROR_INVALID_HANDLE;
	}
	process_t* process=KERNEL_CONTAINEROF(handle,process_t,handle);
	u64 acl;
	vfs_node_t* new_cwd=fd_get_node(fd,&acl);
	if (!new_cwd){
		handle_release(handle);
		return ERROR_INVALID_HANDLE;
	}
	if (!(acl&FD_ACL_FLAG_STAT)){
		vfs_node_unref(new_cwd);
		handle_release(handle);
		return ERROR_DENIED;
	}
	vfs_node_t* old_cwd=process->vfs_cwd;
	process->vfs_cwd=new_cwd;
	vfs_node_unref(old_cwd);
	handle_release(handle);
	return ERROR_OK;
}



error_t syscall_process_get_parent(handle_id_t process_handle){
	if (!process_handle){
		process_handle=THREAD_DATA->process->handle.rb_node.key;
	}
	handle_t* handle=handle_lookup_and_acquire(process_handle,process_handle_type);
	if (!handle){
		return ERROR_INVALID_HANDLE;
	}
	process_t* process=KERNEL_CONTAINEROF(handle,process_t,handle);
	u64 out=(process->parent?process->parent->handle.rb_node.key:0);
	handle_release(handle);
	return out;
}



error_t syscall_process_set_root(handle_id_t process_handle,handle_id_t fd){
	if (!process_handle){
		process_handle=THREAD_DATA->process->handle.rb_node.key;
	}
	handle_t* handle=handle_lookup_and_acquire(process_handle,process_handle_type);
	if (!handle){
		return ERROR_INVALID_HANDLE;
	}
	process_t* process=KERNEL_CONTAINEROF(handle,process_t,handle);
	u64 acl;
	vfs_node_t* new_root=fd_get_node(fd,&acl);
	if (!new_root){
		handle_release(handle);
		return ERROR_INVALID_HANDLE;
	}
	if (!(acl&FD_ACL_FLAG_STAT)){
		vfs_node_unref(new_root);
		handle_release(handle);
		return ERROR_DENIED;
	}
	vfs_node_t* old_root=process->vfs_root;
	process->vfs_root=new_root;
	vfs_node_unref(old_root);
	handle_release(handle);
	return ERROR_OK;
}



error_t syscall_process_get_main_thread(handle_id_t process_handle){
	if (!process_handle){
		process_handle=THREAD_DATA->process->handle.rb_node.key;
	}
	handle_t* handle=handle_lookup_and_acquire(process_handle,process_handle_type);
	if (!handle){
		return ERROR_INVALID_HANDLE;
	}
	process_t* process=KERNEL_CONTAINEROF(handle,process_t,handle);
	error_t out=(process->main_thread?process->main_thread->handle.rb_node.key:0);
	handle_release(handle);
	return out;
}



error_t syscall_process_get_return_value(handle_id_t process_handle){
	handle_t* handle=handle_lookup_and_acquire(process_handle,process_handle_type);
	if (!handle){
		return ERROR_INVALID_HANDLE;
	}
	u64 out=(u64)(KERNEL_CONTAINEROF(handle,process_t,handle)->return_value);
	handle_release(handle);
	return out;
}
