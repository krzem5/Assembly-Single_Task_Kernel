#include <kernel/id/group.h>
#include <kernel/id/user.h>
#include <kernel/mp/thread.h>
#include <kernel/syscall/syscall.h>



u64 syscall_uid_get(void){
	return THREAD_DATA->process->uid;
}



u64 syscall_gid_get(void){
	return THREAD_DATA->process->gid;
}



u64 syscall_uid_set(u64 uid){
	if (!THREAD_DATA->process->uid||!THREAD_DATA->process->gid||uid_has_group(THREAD_DATA->process->uid,0)){
		THREAD_DATA->process->uid=uid;
		return 1;
	}
	return 0;
}



u64 syscall_gid_set(u64 gid){
	if (!THREAD_DATA->process->uid||!THREAD_DATA->process->gid||uid_has_group(THREAD_DATA->process->uid,0)){
		THREAD_DATA->process->gid=gid;
		return 1;
	}
	return 0;
}



u64 syscall_uid_get_name(u64 uid,char* buffer,u32 buffer_length){
	if (buffer_length>syscall_get_user_pointer_max_length((u64)buffer)){
		return 0;
	}
	return uid_get_name(uid,buffer,buffer_length);
}



u64 syscall_gid_get_name(u64 gid,char* buffer,u32 buffer_length){
	if (buffer_length>syscall_get_user_pointer_max_length((u64)buffer)){
		return 0;
	}
	return gid_get_name(gid,buffer,buffer_length);
}
