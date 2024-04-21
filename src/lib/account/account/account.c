#include <account/account.h>
#include <sys/error/error.h>
#include <sys/id/group.h>
#include <sys/id/user.h>
#include <sys/syscall/syscall.h>
#include <sys/types.h>



static u64 _account_syscall_offset=0;



static void SYS_CONSTRUCTOR _init(void){
	_account_syscall_offset=sys_syscall_get_table_offset("account_manager");
}



SYS_PUBLIC sys_gid_t account_iter_group_start(void){
	return _sys_syscall1(_account_syscall_offset|0x00000001,0);
}



SYS_PUBLIC sys_gid_t account_iter_group_next(sys_gid_t gid){
	return _sys_syscall1(_account_syscall_offset|0x00000001,gid);
}



SYS_PUBLIC sys_uid_t account_iter_user_start(void){
	return _sys_syscall1(_account_syscall_offset|0x00000002,0);
}



SYS_PUBLIC sys_uid_t account_iter_user_next(sys_uid_t uid){
	return _sys_syscall1(_account_syscall_offset|0x00000002,uid);
}



SYS_PUBLIC sys_uid_t account_iter_user_group_start(sys_uid_t uid){
	return _sys_syscall2(_account_syscall_offset|0x00000003,uid,0);
}



SYS_PUBLIC sys_uid_t account_iter_user_group_next(sys_uid_t uid,sys_gid_t gid){
	return _sys_syscall2(_account_syscall_offset|0x00000003,uid,gid);
}



SYS_PUBLIC sys_error_t account_get_group_data(sys_gid_t gid,account_group_t* out){
	return _sys_syscall3(_account_syscall_offset|0x00000004,gid,(u64)out,sizeof(account_group_t));
}



SYS_PUBLIC sys_error_t account_get_user_data(sys_uid_t uid,account_user_t* out){
	return _sys_syscall3(_account_syscall_offset|0x00000005,uid,(u64)out,sizeof(account_user_t));
}
