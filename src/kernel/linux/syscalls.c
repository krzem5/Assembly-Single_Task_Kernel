#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



void syscall_linux_getpid(syscall_reg_state_t* regs){
	panic("syscall_linux_getpid");
}



void syscall_linux_exit(syscall_reg_state_t* regs){
	panic("syscall_linux_exit");
}



void syscall_linux_wait4(syscall_reg_state_t* regs){
	panic("syscall_linux_wait4");
}



void syscall_linux_kill(syscall_reg_state_t* regs){
	panic("syscall_linux_kill");
}



void syscall_linux_uname(syscall_reg_state_t* regs){
	panic("syscall_linux_uname");
}



void syscall_linux_semget(syscall_reg_state_t* regs){
	panic("syscall_linux_semget");
}



void syscall_linux_semop(syscall_reg_state_t* regs){
	panic("syscall_linux_semop");
}



void syscall_linux_semctl(syscall_reg_state_t* regs){
	panic("syscall_linux_semctl");
}



void syscall_linux_fcntl(syscall_reg_state_t* regs){
	panic("syscall_linux_fcntl");
}



void syscall_linux_flock(syscall_reg_state_t* regs){
	panic("syscall_linux_flock");
}



void syscall_linux_truncate(syscall_reg_state_t* regs){
	panic("syscall_linux_truncate");
}



void syscall_linux_ftruncate(syscall_reg_state_t* regs){
	panic("syscall_linux_ftruncate");
}



void syscall_linux_getdents(syscall_reg_state_t* regs){
	panic("syscall_linux_getdents");
}



void syscall_linux_getcwd(syscall_reg_state_t* regs){
	panic("syscall_linux_getcwd");
}



void syscall_linux_chdir(syscall_reg_state_t* regs){
	panic("syscall_linux_chdir");
}



void syscall_linux_fchdir(syscall_reg_state_t* regs){
	panic("syscall_linux_fchdir");
}



void syscall_linux_rename(syscall_reg_state_t* regs){
	panic("syscall_linux_rename");
}



void syscall_linux_mkdir(syscall_reg_state_t* regs){
	panic("syscall_linux_mkdir");
}



void syscall_linux_rmdir(syscall_reg_state_t* regs){
	panic("syscall_linux_rmdir");
}



void syscall_linux_link(syscall_reg_state_t* regs){
	panic("syscall_linux_link");
}



void syscall_linux_unlink(syscall_reg_state_t* regs){
	panic("syscall_linux_unlink");
}



void syscall_linux_symlink(syscall_reg_state_t* regs){
	panic("syscall_linux_symlink");
}



void syscall_linux_readlink(syscall_reg_state_t* regs){
	panic("syscall_linux_readlink");
}



void syscall_linux_chmod(syscall_reg_state_t* regs){
	panic("syscall_linux_chmod");
}



void syscall_linux_fchmod(syscall_reg_state_t* regs){
	panic("syscall_linux_fchmod");
}



void syscall_linux_chown(syscall_reg_state_t* regs){
	panic("syscall_linux_chown");
}



void syscall_linux_fchown(syscall_reg_state_t* regs){
	panic("syscall_linux_fchown");
}



void syscall_linux_lchown(syscall_reg_state_t* regs){
	panic("syscall_linux_lchown");
}



void syscall_linux_umask(syscall_reg_state_t* regs){
	panic("syscall_linux_umask");
}



void syscall_linux_sysinfo(syscall_reg_state_t* regs){
	panic("syscall_linux_sysinfo");
}



void syscall_linux_times(syscall_reg_state_t* regs){
	panic("syscall_linux_times");
}



void syscall_linux_getuid(syscall_reg_state_t* regs){
	panic("syscall_linux_getuid");
}



void syscall_linux_getgid(syscall_reg_state_t* regs){
	panic("syscall_linux_getgid");
}



void syscall_linux_setuid(syscall_reg_state_t* regs){
	panic("syscall_linux_setuid");
}



void syscall_linux_setgid(syscall_reg_state_t* regs){
	panic("syscall_linux_setgid");
}



void syscall_linux_getppid(syscall_reg_state_t* regs){
	panic("syscall_linux_getppid");
}



void syscall_linux_getpgid(syscall_reg_state_t* regs){
	panic("syscall_linux_getpgid");
}



void syscall_linux_utime(syscall_reg_state_t* regs){
	panic("syscall_linux_utime");
}



void syscall_linux_gettid(syscall_reg_state_t* regs){
	panic("syscall_linux_gettid");
}



void syscall_linux_getdents64(syscall_reg_state_t* regs){
	panic("syscall_linux_getdents64");
}



void syscall_linux_fadvise64(syscall_reg_state_t* regs){
	panic("syscall_linux_fadvise64");
}



void syscall_linux_mkdirat(syscall_reg_state_t* regs){
	panic("syscall_linux_mkdirat");
}



void syscall_linux_fchownat(syscall_reg_state_t* regs){
	panic("syscall_linux_fchownat");
}



void syscall_linux_futimesat(syscall_reg_state_t* regs){
	panic("syscall_linux_futimesat");
}



void syscall_linux_unlinkat(syscall_reg_state_t* regs){
	panic("syscall_linux_unlinkat");
}



void syscall_linux_renameat(syscall_reg_state_t* regs){
	panic("syscall_linux_renameat");
}



void syscall_linux_linkat(syscall_reg_state_t* regs){
	panic("syscall_linux_linkat");
}



void syscall_linux_symlinkat(syscall_reg_state_t* regs){
	panic("syscall_linux_symlinkat");
}



void syscall_linux_readlinkat(syscall_reg_state_t* regs){
	panic("syscall_linux_readlinkat");
}



void syscall_linux_fchmodat(syscall_reg_state_t* regs){
	panic("syscall_linux_fchmodat");
}



void syscall_linux_splice(syscall_reg_state_t* regs){
	panic("syscall_linux_splice");
}



void syscall_linux_tee(syscall_reg_state_t* regs){
	panic("syscall_linux_tee");
}



void syscall_linux_renameat2(syscall_reg_state_t* regs){
	panic("syscall_linux_renameat2");
}

// TODO APIs:
// better memory api (alloc + counters + locks)
// drive buffering (internal)
// process_get_acl(handle), thread_get_acl(handle)
// ACLs -> acl_get_permissions(handle), acl_set_permissions(handle,clear,set)
// sockets -> socket_create(domain,type,protocol), socket_create_pair(out[2]), socket_connect(handle,address,address_length), socket_bind(handle,address,address_length), socket_listen(handle,max_connections), socket_accept(handle), socket_getsockname, socket_getpeername, socket_recvfrom, socket_recvmsg, socket_sento, socket_sendmsg, socket_shutdown
