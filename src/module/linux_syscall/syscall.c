#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "linux_syscall"



static u64 _syscall_linux_read(void){
	ERROR("_syscall_linux_read: unimplemented");
	return 0;
}



static u64 _syscall_linux_write(void){
	ERROR("_syscall_linux_write: unimplemented");
	return 0;
}



static u64 _syscall_linux_open(void){
	ERROR("_syscall_linux_open: unimplemented");
	return 0;
}



static u64 _syscall_linux_close(void){
	ERROR("_syscall_linux_close: unimplemented");
	return 0;
}



static u64 _syscall_linux_stat(void){
	ERROR("_syscall_linux_stat: unimplemented");
	return 0;
}



static u64 _syscall_linux_fstat(void){
	ERROR("_syscall_linux_fstat: unimplemented");
	return 0;
}



static u64 _syscall_linux_lstat(void){
	ERROR("_syscall_linux_lstat: unimplemented");
	return 0;
}



static u64 _syscall_linux_poll(void){
	ERROR("_syscall_linux_poll: unimplemented");
	return 0;
}



static u64 _syscall_linux_lseek(void){
	ERROR("_syscall_linux_lseek: unimplemented");
	return 0;
}



static u64 _syscall_linux_mmap(void){
	ERROR("_syscall_linux_mmap: unimplemented");
	return 0;
}



static u64 _syscall_linux_mprotect(void){
	ERROR("_syscall_linux_mprotect: unimplemented");
	return 0;
}



static u64 _syscall_linux_munmap(void){
	ERROR("_syscall_linux_munmap: unimplemented");
	return 0;
}



static u64 _syscall_linux_brk(void){
	ERROR("_syscall_linux_brk: unimplemented");
	return 0;
}



static u64 _syscall_linux_rt_sigaction(void){
	ERROR("_syscall_linux_rt_sigaction: unimplemented");
	return 0;
}



static u64 _syscall_linux_rt_sigprocmask(void){
	ERROR("_syscall_linux_rt_sigprocmask: unimplemented");
	return 0;
}



static u64 _syscall_linux_rt_sigreturn(void){
	ERROR("_syscall_linux_rt_sigreturn: unimplemented");
	return 0;
}



static u64 _syscall_linux_ioctl(void){
	ERROR("_syscall_linux_ioctl: unimplemented");
	return 0;
}



static u64 _syscall_linux_pread64(void){
	ERROR("_syscall_linux_pread64: unimplemented");
	return 0;
}



static u64 _syscall_linux_pwrite64(void){
	ERROR("_syscall_linux_pwrite64: unimplemented");
	return 0;
}



static u64 _syscall_linux_readv(void){
	ERROR("_syscall_linux_readv: unimplemented");
	return 0;
}



static u64 _syscall_linux_writev(void){
	ERROR("_syscall_linux_writev: unimplemented");
	return 0;
}



static u64 _syscall_linux_access(void){
	ERROR("_syscall_linux_access: unimplemented");
	return 0;
}



static u64 _syscall_linux_pipe(void){
	ERROR("_syscall_linux_pipe: unimplemented");
	return 0;
}



static u64 _syscall_linux_select(void){
	ERROR("_syscall_linux_select: unimplemented");
	return 0;
}



static u64 _syscall_linux_sched_yield(void){
	ERROR("_syscall_linux_sched_yield: unimplemented");
	return 0;
}



static u64 _syscall_linux_mremap(void){
	ERROR("_syscall_linux_mremap: unimplemented");
	return 0;
}



static u64 _syscall_linux_msync(void){
	ERROR("_syscall_linux_msync: unimplemented");
	return 0;
}



static u64 _syscall_linux_mincore(void){
	ERROR("_syscall_linux_mincore: unimplemented");
	return 0;
}



static u64 _syscall_linux_madvise(void){
	ERROR("_syscall_linux_madvise: unimplemented");
	return 0;
}



static u64 _syscall_linux_shmget(void){
	ERROR("_syscall_linux_shmget: unimplemented");
	return 0;
}



static u64 _syscall_linux_shmat(void){
	ERROR("_syscall_linux_shmat: unimplemented");
	return 0;
}



static u64 _syscall_linux_shmctl(void){
	ERROR("_syscall_linux_shmctl: unimplemented");
	return 0;
}



static u64 _syscall_linux_dup(void){
	ERROR("_syscall_linux_dup: unimplemented");
	return 0;
}



static u64 _syscall_linux_dup2(void){
	ERROR("_syscall_linux_dup2: unimplemented");
	return 0;
}



static u64 _syscall_linux_pause(void){
	ERROR("_syscall_linux_pause: unimplemented");
	return 0;
}



static u64 _syscall_linux_nanosleep(void){
	ERROR("_syscall_linux_nanosleep: unimplemented");
	return 0;
}



static u64 _syscall_linux_getitimer(void){
	ERROR("_syscall_linux_getitimer: unimplemented");
	return 0;
}



static u64 _syscall_linux_alarm(void){
	ERROR("_syscall_linux_alarm: unimplemented");
	return 0;
}



static u64 _syscall_linux_setitimer(void){
	ERROR("_syscall_linux_setitimer: unimplemented");
	return 0;
}



static u64 _syscall_linux_getpid(void){
	ERROR("_syscall_linux_getpid: unimplemented");
	return 0;
}



static u64 _syscall_linux_sendfile(void){
	ERROR("_syscall_linux_sendfile: unimplemented");
	return 0;
}



static u64 _syscall_linux_socket(void){
	ERROR("_syscall_linux_socket: unimplemented");
	return 0;
}



static u64 _syscall_linux_connect(void){
	ERROR("_syscall_linux_connect: unimplemented");
	return 0;
}



static u64 _syscall_linux_accept(void){
	ERROR("_syscall_linux_accept: unimplemented");
	return 0;
}



static u64 _syscall_linux_sendto(void){
	ERROR("_syscall_linux_sendto: unimplemented");
	return 0;
}



static u64 _syscall_linux_recvfrom(void){
	ERROR("_syscall_linux_recvfrom: unimplemented");
	return 0;
}



static u64 _syscall_linux_sendmsg(void){
	ERROR("_syscall_linux_sendmsg: unimplemented");
	return 0;
}



static u64 _syscall_linux_recvmsg(void){
	ERROR("_syscall_linux_recvmsg: unimplemented");
	return 0;
}



static u64 _syscall_linux_shutdown(void){
	ERROR("_syscall_linux_shutdown: unimplemented");
	return 0;
}



static u64 _syscall_linux_bind(void){
	ERROR("_syscall_linux_bind: unimplemented");
	return 0;
}



static u64 _syscall_linux_listen(void){
	ERROR("_syscall_linux_listen: unimplemented");
	return 0;
}



static u64 _syscall_linux_getsockname(void){
	ERROR("_syscall_linux_getsockname: unimplemented");
	return 0;
}



static u64 _syscall_linux_getpeername(void){
	ERROR("_syscall_linux_getpeername: unimplemented");
	return 0;
}



static u64 _syscall_linux_socketpair(void){
	ERROR("_syscall_linux_socketpair: unimplemented");
	return 0;
}



static u64 _syscall_linux_setsockopt(void){
	ERROR("_syscall_linux_setsockopt: unimplemented");
	return 0;
}



static u64 _syscall_linux_getsockopt(void){
	ERROR("_syscall_linux_getsockopt: unimplemented");
	return 0;
}



static u64 _syscall_linux_clone(void){
	ERROR("_syscall_linux_clone: unimplemented");
	return 0;
}



static u64 _syscall_linux_fork(void){
	ERROR("_syscall_linux_fork: unimplemented");
	return 0;
}



static u64 _syscall_linux_vfork(void){
	ERROR("_syscall_linux_vfork: unimplemented");
	return 0;
}



static u64 _syscall_linux_execve(void){
	ERROR("_syscall_linux_execve: unimplemented");
	return 0;
}



static u64 _syscall_linux_exit(void){
	ERROR("_syscall_linux_exit: unimplemented");
	return 0;
}



static u64 _syscall_linux_wait4(void){
	ERROR("_syscall_linux_wait4: unimplemented");
	return 0;
}



static u64 _syscall_linux_kill(void){
	ERROR("_syscall_linux_kill: unimplemented");
	return 0;
}



static u64 _syscall_linux_uname(void){
	ERROR("_syscall_linux_uname: unimplemented");
	return 0;
}



static u64 _syscall_linux_semget(void){
	ERROR("_syscall_linux_semget: unimplemented");
	return 0;
}



static u64 _syscall_linux_semop(void){
	ERROR("_syscall_linux_semop: unimplemented");
	return 0;
}



static u64 _syscall_linux_semctl(void){
	ERROR("_syscall_linux_semctl: unimplemented");
	return 0;
}



static u64 _syscall_linux_shmdt(void){
	ERROR("_syscall_linux_shmdt: unimplemented");
	return 0;
}



static u64 _syscall_linux_msgget(void){
	ERROR("_syscall_linux_msgget: unimplemented");
	return 0;
}



static u64 _syscall_linux_msgsnd(void){
	ERROR("_syscall_linux_msgsnd: unimplemented");
	return 0;
}



static u64 _syscall_linux_msgrcv(void){
	ERROR("_syscall_linux_msgrcv: unimplemented");
	return 0;
}



static u64 _syscall_linux_msgctl(void){
	ERROR("_syscall_linux_msgctl: unimplemented");
	return 0;
}



static u64 _syscall_linux_fcntl(void){
	ERROR("_syscall_linux_fcntl: unimplemented");
	return 0;
}



static u64 _syscall_linux_flock(void){
	ERROR("_syscall_linux_flock: unimplemented");
	return 0;
}



static u64 _syscall_linux_fsync(void){
	ERROR("_syscall_linux_fsync: unimplemented");
	return 0;
}



static u64 _syscall_linux_fdatasync(void){
	ERROR("_syscall_linux_fdatasync: unimplemented");
	return 0;
}



static u64 _syscall_linux_truncate(void){
	ERROR("_syscall_linux_truncate: unimplemented");
	return 0;
}



static u64 _syscall_linux_ftruncate(void){
	ERROR("_syscall_linux_ftruncate: unimplemented");
	return 0;
}



static u64 _syscall_linux_getdents(void){
	ERROR("_syscall_linux_getdents: unimplemented");
	return 0;
}



static u64 _syscall_linux_getcwd(void){
	ERROR("_syscall_linux_getcwd: unimplemented");
	return 0;
}



static u64 _syscall_linux_chdir(void){
	ERROR("_syscall_linux_chdir: unimplemented");
	return 0;
}



static u64 _syscall_linux_fchdir(void){
	ERROR("_syscall_linux_fchdir: unimplemented");
	return 0;
}



static u64 _syscall_linux_rename(void){
	ERROR("_syscall_linux_rename: unimplemented");
	return 0;
}



static u64 _syscall_linux_mkdir(void){
	ERROR("_syscall_linux_mkdir: unimplemented");
	return 0;
}



static u64 _syscall_linux_rmdir(void){
	ERROR("_syscall_linux_rmdir: unimplemented");
	return 0;
}



static u64 _syscall_linux_creat(void){
	ERROR("_syscall_linux_creat: unimplemented");
	return 0;
}



static u64 _syscall_linux_link(void){
	ERROR("_syscall_linux_link: unimplemented");
	return 0;
}



static u64 _syscall_linux_unlink(void){
	ERROR("_syscall_linux_unlink: unimplemented");
	return 0;
}



static u64 _syscall_linux_symlink(void){
	ERROR("_syscall_linux_symlink: unimplemented");
	return 0;
}



static u64 _syscall_linux_readlink(void){
	ERROR("_syscall_linux_readlink: unimplemented");
	return 0;
}



static u64 _syscall_linux_chmod(void){
	ERROR("_syscall_linux_chmod: unimplemented");
	return 0;
}



static u64 _syscall_linux_fchmod(void){
	ERROR("_syscall_linux_fchmod: unimplemented");
	return 0;
}



static u64 _syscall_linux_chown(void){
	ERROR("_syscall_linux_chown: unimplemented");
	return 0;
}



static u64 _syscall_linux_fchown(void){
	ERROR("_syscall_linux_fchown: unimplemented");
	return 0;
}



static u64 _syscall_linux_lchown(void){
	ERROR("_syscall_linux_lchown: unimplemented");
	return 0;
}



static u64 _syscall_linux_umask(void){
	ERROR("_syscall_linux_umask: unimplemented");
	return 0;
}



static u64 _syscall_linux_gettimeofday(void){
	ERROR("_syscall_linux_gettimeofday: unimplemented");
	return 0;
}



static u64 _syscall_linux_getrlimit(void){
	ERROR("_syscall_linux_getrlimit: unimplemented");
	return 0;
}



static u64 _syscall_linux_getrusage(void){
	ERROR("_syscall_linux_getrusage: unimplemented");
	return 0;
}



static u64 _syscall_linux_sysinfo(void){
	ERROR("_syscall_linux_sysinfo: unimplemented");
	return 0;
}



static u64 _syscall_linux_times(void){
	ERROR("_syscall_linux_times: unimplemented");
	return 0;
}



static u64 _syscall_linux_ptrace(void){
	ERROR("_syscall_linux_ptrace: unimplemented");
	return 0;
}



static u64 _syscall_linux_getuid(void){
	ERROR("_syscall_linux_getuid: unimplemented");
	return 0;
}



static u64 _syscall_linux_syslog(void){
	ERROR("_syscall_linux_syslog: unimplemented");
	return 0;
}



static u64 _syscall_linux_getgid(void){
	ERROR("_syscall_linux_getgid: unimplemented");
	return 0;
}



static u64 _syscall_linux_setuid(void){
	ERROR("_syscall_linux_setuid: unimplemented");
	return 0;
}



static u64 _syscall_linux_setgid(void){
	ERROR("_syscall_linux_setgid: unimplemented");
	return 0;
}



static u64 _syscall_linux_geteuid(void){
	ERROR("_syscall_linux_geteuid: unimplemented");
	return 0;
}



static u64 _syscall_linux_getegid(void){
	ERROR("_syscall_linux_getegid: unimplemented");
	return 0;
}



static u64 _syscall_linux_setpgid(void){
	ERROR("_syscall_linux_setpgid: unimplemented");
	return 0;
}



static u64 _syscall_linux_getppid(void){
	ERROR("_syscall_linux_getppid: unimplemented");
	return 0;
}



static u64 _syscall_linux_getpgrp(void){
	ERROR("_syscall_linux_getpgrp: unimplemented");
	return 0;
}



static u64 _syscall_linux_setsid(void){
	ERROR("_syscall_linux_setsid: unimplemented");
	return 0;
}



static u64 _syscall_linux_setreuid(void){
	ERROR("_syscall_linux_setreuid: unimplemented");
	return 0;
}



static u64 _syscall_linux_setregid(void){
	ERROR("_syscall_linux_setregid: unimplemented");
	return 0;
}



static u64 _syscall_linux_getgroups(void){
	ERROR("_syscall_linux_getgroups: unimplemented");
	return 0;
}



static u64 _syscall_linux_setgroups(void){
	ERROR("_syscall_linux_setgroups: unimplemented");
	return 0;
}



static u64 _syscall_linux_setresuid(void){
	ERROR("_syscall_linux_setresuid: unimplemented");
	return 0;
}



static u64 _syscall_linux_getresuid(void){
	ERROR("_syscall_linux_getresuid: unimplemented");
	return 0;
}



static u64 _syscall_linux_setresgid(void){
	ERROR("_syscall_linux_setresgid: unimplemented");
	return 0;
}



static u64 _syscall_linux_getresgid(void){
	ERROR("_syscall_linux_getresgid: unimplemented");
	return 0;
}



static u64 _syscall_linux_getpgid(void){
	ERROR("_syscall_linux_getpgid: unimplemented");
	return 0;
}



static u64 _syscall_linux_setfsuid(void){
	ERROR("_syscall_linux_setfsuid: unimplemented");
	return 0;
}



static u64 _syscall_linux_setfsgid(void){
	ERROR("_syscall_linux_setfsgid: unimplemented");
	return 0;
}



static u64 _syscall_linux_getsid(void){
	ERROR("_syscall_linux_getsid: unimplemented");
	return 0;
}



static u64 _syscall_linux_capget(void){
	ERROR("_syscall_linux_capget: unimplemented");
	return 0;
}



static u64 _syscall_linux_capset(void){
	ERROR("_syscall_linux_capset: unimplemented");
	return 0;
}



static u64 _syscall_linux_rt_sigpending(void){
	ERROR("_syscall_linux_rt_sigpending: unimplemented");
	return 0;
}



static u64 _syscall_linux_rt_sigtimedwait(void){
	ERROR("_syscall_linux_rt_sigtimedwait: unimplemented");
	return 0;
}



static u64 _syscall_linux_rt_sigqueueinfo(void){
	ERROR("_syscall_linux_rt_sigqueueinfo: unimplemented");
	return 0;
}



static u64 _syscall_linux_rt_sigsuspend(void){
	ERROR("_syscall_linux_rt_sigsuspend: unimplemented");
	return 0;
}



static u64 _syscall_linux_sigaltstack(void){
	ERROR("_syscall_linux_sigaltstack: unimplemented");
	return 0;
}



static u64 _syscall_linux_utime(void){
	ERROR("_syscall_linux_utime: unimplemented");
	return 0;
}



static u64 _syscall_linux_mknod(void){
	ERROR("_syscall_linux_mknod: unimplemented");
	return 0;
}



static u64 _syscall_linux_uselib(void){
	ERROR("_syscall_linux_uselib: unimplemented");
	return 0;
}



static u64 _syscall_linux_personality(void){
	ERROR("_syscall_linux_personality: unimplemented");
	return 0;
}



static u64 _syscall_linux_ustat(void){
	ERROR("_syscall_linux_ustat: unimplemented");
	return 0;
}



static u64 _syscall_linux_statfs(void){
	ERROR("_syscall_linux_statfs: unimplemented");
	return 0;
}



static u64 _syscall_linux_fstatfs(void){
	ERROR("_syscall_linux_fstatfs: unimplemented");
	return 0;
}



static u64 _syscall_linux_sysfs(void){
	ERROR("_syscall_linux_sysfs: unimplemented");
	return 0;
}



static u64 _syscall_linux_getpriority(void){
	ERROR("_syscall_linux_getpriority: unimplemented");
	return 0;
}



static u64 _syscall_linux_setpriority(void){
	ERROR("_syscall_linux_setpriority: unimplemented");
	return 0;
}



static u64 _syscall_linux_sched_setparam(void){
	ERROR("_syscall_linux_sched_setparam: unimplemented");
	return 0;
}



static u64 _syscall_linux_sched_getparam(void){
	ERROR("_syscall_linux_sched_getparam: unimplemented");
	return 0;
}



static u64 _syscall_linux_sched_setscheduler(void){
	ERROR("_syscall_linux_sched_setscheduler: unimplemented");
	return 0;
}



static u64 _syscall_linux_sched_getscheduler(void){
	ERROR("_syscall_linux_sched_getscheduler: unimplemented");
	return 0;
}



static u64 _syscall_linux_sched_get_priority_max(void){
	ERROR("_syscall_linux_sched_get_priority_max: unimplemented");
	return 0;
}



static u64 _syscall_linux_sched_get_priority_min(void){
	ERROR("_syscall_linux_sched_get_priority_min: unimplemented");
	return 0;
}



static u64 _syscall_linux_sched_rr_get_interval(void){
	ERROR("_syscall_linux_sched_rr_get_interval: unimplemented");
	return 0;
}



static u64 _syscall_linux_mlock(void){
	ERROR("_syscall_linux_mlock: unimplemented");
	return 0;
}



static u64 _syscall_linux_munlock(void){
	ERROR("_syscall_linux_munlock: unimplemented");
	return 0;
}



static u64 _syscall_linux_mlockall(void){
	ERROR("_syscall_linux_mlockall: unimplemented");
	return 0;
}



static u64 _syscall_linux_munlockall(void){
	ERROR("_syscall_linux_munlockall: unimplemented");
	return 0;
}



static u64 _syscall_linux_vhangup(void){
	ERROR("_syscall_linux_vhangup: unimplemented");
	return 0;
}



static u64 _syscall_linux_modify_ldt(void){
	ERROR("_syscall_linux_modify_ldt: unimplemented");
	return 0;
}



static u64 _syscall_linux_pivot_root(void){
	ERROR("_syscall_linux_pivot_root: unimplemented");
	return 0;
}



static u64 _syscall_linux__sysctl(void){
	ERROR("_syscall_linux__sysctl: unimplemented");
	return 0;
}



static u64 _syscall_linux_prctl(void){
	ERROR("_syscall_linux_prctl: unimplemented");
	return 0;
}



static u64 _syscall_linux_arch_prctl(void){
	ERROR("_syscall_linux_arch_prctl: unimplemented");
	return 0;
}



static u64 _syscall_linux_adjtimex(void){
	ERROR("_syscall_linux_adjtimex: unimplemented");
	return 0;
}



static u64 _syscall_linux_setrlimit(void){
	ERROR("_syscall_linux_setrlimit: unimplemented");
	return 0;
}



static u64 _syscall_linux_chroot(void){
	ERROR("_syscall_linux_chroot: unimplemented");
	return 0;
}



static u64 _syscall_linux_sync(void){
	ERROR("_syscall_linux_sync: unimplemented");
	return 0;
}



static u64 _syscall_linux_acct(void){
	ERROR("_syscall_linux_acct: unimplemented");
	return 0;
}



static u64 _syscall_linux_settimeofday(void){
	ERROR("_syscall_linux_settimeofday: unimplemented");
	return 0;
}



static u64 _syscall_linux_mount(void){
	ERROR("_syscall_linux_mount: unimplemented");
	return 0;
}



static u64 _syscall_linux_umount2(void){
	ERROR("_syscall_linux_umount2: unimplemented");
	return 0;
}



static u64 _syscall_linux_swapon(void){
	ERROR("_syscall_linux_swapon: unimplemented");
	return 0;
}



static u64 _syscall_linux_swapoff(void){
	ERROR("_syscall_linux_swapoff: unimplemented");
	return 0;
}



static u64 _syscall_linux_reboot(void){
	ERROR("_syscall_linux_reboot: unimplemented");
	return 0;
}



static u64 _syscall_linux_sethostname(void){
	ERROR("_syscall_linux_sethostname: unimplemented");
	return 0;
}



static u64 _syscall_linux_setdomainname(void){
	ERROR("_syscall_linux_setdomainname: unimplemented");
	return 0;
}



static u64 _syscall_linux_iopl(void){
	ERROR("_syscall_linux_iopl: unimplemented");
	return 0;
}



static u64 _syscall_linux_ioperm(void){
	ERROR("_syscall_linux_ioperm: unimplemented");
	return 0;
}



static u64 _syscall_linux_create_module(void){
	ERROR("_syscall_linux_create_module: unimplemented");
	return 0;
}



static u64 _syscall_linux_init_module(void){
	ERROR("_syscall_linux_init_module: unimplemented");
	return 0;
}



static u64 _syscall_linux_delete_module(void){
	ERROR("_syscall_linux_delete_module: unimplemented");
	return 0;
}



static u64 _syscall_linux_get_kernel_syms(void){
	ERROR("_syscall_linux_get_kernel_syms: unimplemented");
	return 0;
}



static u64 _syscall_linux_query_module(void){
	ERROR("_syscall_linux_query_module: unimplemented");
	return 0;
}



static u64 _syscall_linux_quotactl(void){
	ERROR("_syscall_linux_quotactl: unimplemented");
	return 0;
}



static u64 _syscall_linux_nfsservctl(void){
	ERROR("_syscall_linux_nfsservctl: unimplemented");
	return 0;
}



static u64 _syscall_linux_getpmsg(void){
	ERROR("_syscall_linux_getpmsg: unimplemented");
	return 0;
}



static u64 _syscall_linux_putpmsg(void){
	ERROR("_syscall_linux_putpmsg: unimplemented");
	return 0;
}



static u64 _syscall_linux_afs_syscall(void){
	ERROR("_syscall_linux_afs_syscall: unimplemented");
	return 0;
}



static u64 _syscall_linux_tuxcall(void){
	ERROR("_syscall_linux_tuxcall: unimplemented");
	return 0;
}



static u64 _syscall_linux_security(void){
	ERROR("_syscall_linux_security: unimplemented");
	return 0;
}



static u64 _syscall_linux_gettid(void){
	ERROR("_syscall_linux_gettid: unimplemented");
	return 0;
}



static u64 _syscall_linux_readahead(void){
	ERROR("_syscall_linux_readahead: unimplemented");
	return 0;
}



static u64 _syscall_linux_setxattr(void){
	ERROR("_syscall_linux_setxattr: unimplemented");
	return 0;
}



static u64 _syscall_linux_lsetxattr(void){
	ERROR("_syscall_linux_lsetxattr: unimplemented");
	return 0;
}



static u64 _syscall_linux_fsetxattr(void){
	ERROR("_syscall_linux_fsetxattr: unimplemented");
	return 0;
}



static u64 _syscall_linux_getxattr(void){
	ERROR("_syscall_linux_getxattr: unimplemented");
	return 0;
}



static u64 _syscall_linux_lgetxattr(void){
	ERROR("_syscall_linux_lgetxattr: unimplemented");
	return 0;
}



static u64 _syscall_linux_fgetxattr(void){
	ERROR("_syscall_linux_fgetxattr: unimplemented");
	return 0;
}



static u64 _syscall_linux_listxattr(void){
	ERROR("_syscall_linux_listxattr: unimplemented");
	return 0;
}



static u64 _syscall_linux_llistxattr(void){
	ERROR("_syscall_linux_llistxattr: unimplemented");
	return 0;
}



static u64 _syscall_linux_flistxattr(void){
	ERROR("_syscall_linux_flistxattr: unimplemented");
	return 0;
}



static u64 _syscall_linux_removexattr(void){
	ERROR("_syscall_linux_removexattr: unimplemented");
	return 0;
}



static u64 _syscall_linux_lremovexattr(void){
	ERROR("_syscall_linux_lremovexattr: unimplemented");
	return 0;
}



static u64 _syscall_linux_fremovexattr(void){
	ERROR("_syscall_linux_fremovexattr: unimplemented");
	return 0;
}



static u64 _syscall_linux_tkill(void){
	ERROR("_syscall_linux_tkill: unimplemented");
	return 0;
}



static u64 _syscall_linux_time(void){
	ERROR("_syscall_linux_time: unimplemented");
	return 0;
}



static u64 _syscall_linux_futex(void){
	ERROR("_syscall_linux_futex: unimplemented");
	return 0;
}



static u64 _syscall_linux_sched_setaffinity(void){
	ERROR("_syscall_linux_sched_setaffinity: unimplemented");
	return 0;
}



static u64 _syscall_linux_sched_getaffinity(void){
	ERROR("_syscall_linux_sched_getaffinity: unimplemented");
	return 0;
}



static u64 _syscall_linux_set_thread_area(void){
	ERROR("_syscall_linux_set_thread_area: unimplemented");
	return 0;
}



static u64 _syscall_linux_io_setup(void){
	ERROR("_syscall_linux_io_setup: unimplemented");
	return 0;
}



static u64 _syscall_linux_io_destroy(void){
	ERROR("_syscall_linux_io_destroy: unimplemented");
	return 0;
}



static u64 _syscall_linux_io_getevents(void){
	ERROR("_syscall_linux_io_getevents: unimplemented");
	return 0;
}



static u64 _syscall_linux_io_submit(void){
	ERROR("_syscall_linux_io_submit: unimplemented");
	return 0;
}



static u64 _syscall_linux_io_cancel(void){
	ERROR("_syscall_linux_io_cancel: unimplemented");
	return 0;
}



static u64 _syscall_linux_get_thread_area(void){
	ERROR("_syscall_linux_get_thread_area: unimplemented");
	return 0;
}



static u64 _syscall_linux_lookup_dcookie(void){
	ERROR("_syscall_linux_lookup_dcookie: unimplemented");
	return 0;
}



static u64 _syscall_linux_epoll_create(void){
	ERROR("_syscall_linux_epoll_create: unimplemented");
	return 0;
}



static u64 _syscall_linux_epoll_ctl_old(void){
	ERROR("_syscall_linux_epoll_ctl_old: unimplemented");
	return 0;
}



static u64 _syscall_linux_epoll_wait_old(void){
	ERROR("_syscall_linux_epoll_wait_old: unimplemented");
	return 0;
}



static u64 _syscall_linux_remap_file_pages(void){
	ERROR("_syscall_linux_remap_file_pages: unimplemented");
	return 0;
}



static u64 _syscall_linux_getdents64(void){
	ERROR("_syscall_linux_getdents64: unimplemented");
	return 0;
}



static u64 _syscall_linux_set_tid_address(void){
	ERROR("_syscall_linux_set_tid_address: unimplemented");
	return 0;
}



static u64 _syscall_linux_restart_syscall(void){
	ERROR("_syscall_linux_restart_syscall: unimplemented");
	return 0;
}



static u64 _syscall_linux_semtimedop(void){
	ERROR("_syscall_linux_semtimedop: unimplemented");
	return 0;
}



static u64 _syscall_linux_fadvise64(void){
	ERROR("_syscall_linux_fadvise64: unimplemented");
	return 0;
}



static u64 _syscall_linux_timer_create(void){
	ERROR("_syscall_linux_timer_create: unimplemented");
	return 0;
}



static u64 _syscall_linux_timer_settime(void){
	ERROR("_syscall_linux_timer_settime: unimplemented");
	return 0;
}



static u64 _syscall_linux_timer_gettime(void){
	ERROR("_syscall_linux_timer_gettime: unimplemented");
	return 0;
}



static u64 _syscall_linux_timer_getoverrun(void){
	ERROR("_syscall_linux_timer_getoverrun: unimplemented");
	return 0;
}



static u64 _syscall_linux_timer_delete(void){
	ERROR("_syscall_linux_timer_delete: unimplemented");
	return 0;
}



static u64 _syscall_linux_clock_settime(void){
	ERROR("_syscall_linux_clock_settime: unimplemented");
	return 0;
}



static u64 _syscall_linux_clock_gettime(void){
	ERROR("_syscall_linux_clock_gettime: unimplemented");
	return 0;
}



static u64 _syscall_linux_clock_getres(void){
	ERROR("_syscall_linux_clock_getres: unimplemented");
	return 0;
}



static u64 _syscall_linux_clock_nanosleep(void){
	ERROR("_syscall_linux_clock_nanosleep: unimplemented");
	return 0;
}



static u64 _syscall_linux_exit_group(void){
	ERROR("_syscall_linux_exit_group: unimplemented");
	return 0;
}



static u64 _syscall_linux_epoll_wait(void){
	ERROR("_syscall_linux_epoll_wait: unimplemented");
	return 0;
}



static u64 _syscall_linux_epoll_ctl(void){
	ERROR("_syscall_linux_epoll_ctl: unimplemented");
	return 0;
}



static u64 _syscall_linux_tgkill(void){
	ERROR("_syscall_linux_tgkill: unimplemented");
	return 0;
}



static u64 _syscall_linux_utimes(void){
	ERROR("_syscall_linux_utimes: unimplemented");
	return 0;
}



static u64 _syscall_linux_vserver(void){
	ERROR("_syscall_linux_vserver: unimplemented");
	return 0;
}



static u64 _syscall_linux_mbind(void){
	ERROR("_syscall_linux_mbind: unimplemented");
	return 0;
}



static u64 _syscall_linux_set_mempolicy(void){
	ERROR("_syscall_linux_set_mempolicy: unimplemented");
	return 0;
}



static u64 _syscall_linux_get_mempolicy(void){
	ERROR("_syscall_linux_get_mempolicy: unimplemented");
	return 0;
}



static u64 _syscall_linux_mq_open(void){
	ERROR("_syscall_linux_mq_open: unimplemented");
	return 0;
}



static u64 _syscall_linux_mq_unlink(void){
	ERROR("_syscall_linux_mq_unlink: unimplemented");
	return 0;
}



static u64 _syscall_linux_mq_timedsend(void){
	ERROR("_syscall_linux_mq_timedsend: unimplemented");
	return 0;
}



static u64 _syscall_linux_mq_timedreceive(void){
	ERROR("_syscall_linux_mq_timedreceive: unimplemented");
	return 0;
}



static u64 _syscall_linux_mq_notify(void){
	ERROR("_syscall_linux_mq_notify: unimplemented");
	return 0;
}



static u64 _syscall_linux_mq_getsetattr(void){
	ERROR("_syscall_linux_mq_getsetattr: unimplemented");
	return 0;
}



static u64 _syscall_linux_kexec_load(void){
	ERROR("_syscall_linux_kexec_load: unimplemented");
	return 0;
}



static u64 _syscall_linux_waitid(void){
	ERROR("_syscall_linux_waitid: unimplemented");
	return 0;
}



static u64 _syscall_linux_add_key(void){
	ERROR("_syscall_linux_add_key: unimplemented");
	return 0;
}



static u64 _syscall_linux_request_key(void){
	ERROR("_syscall_linux_request_key: unimplemented");
	return 0;
}



static u64 _syscall_linux_keyctl(void){
	ERROR("_syscall_linux_keyctl: unimplemented");
	return 0;
}



static u64 _syscall_linux_ioprio_set(void){
	ERROR("_syscall_linux_ioprio_set: unimplemented");
	return 0;
}



static u64 _syscall_linux_ioprio_get(void){
	ERROR("_syscall_linux_ioprio_get: unimplemented");
	return 0;
}



static u64 _syscall_linux_inotify_init(void){
	ERROR("_syscall_linux_inotify_init: unimplemented");
	return 0;
}



static u64 _syscall_linux_inotify_add_watch(void){
	ERROR("_syscall_linux_inotify_add_watch: unimplemented");
	return 0;
}



static u64 _syscall_linux_inotify_rm_watch(void){
	ERROR("_syscall_linux_inotify_rm_watch: unimplemented");
	return 0;
}



static u64 _syscall_linux_migrate_pages(void){
	ERROR("_syscall_linux_migrate_pages: unimplemented");
	return 0;
}



static u64 _syscall_linux_openat(void){
	ERROR("_syscall_linux_openat: unimplemented");
	return 0;
}



static u64 _syscall_linux_mkdirat(void){
	ERROR("_syscall_linux_mkdirat: unimplemented");
	return 0;
}



static u64 _syscall_linux_mknodat(void){
	ERROR("_syscall_linux_mknodat: unimplemented");
	return 0;
}



static u64 _syscall_linux_fchownat(void){
	ERROR("_syscall_linux_fchownat: unimplemented");
	return 0;
}



static u64 _syscall_linux_futimesat(void){
	ERROR("_syscall_linux_futimesat: unimplemented");
	return 0;
}



static u64 _syscall_linux_newfstatat(void){
	ERROR("_syscall_linux_newfstatat: unimplemented");
	return 0;
}



static u64 _syscall_linux_unlinkat(void){
	ERROR("_syscall_linux_unlinkat: unimplemented");
	return 0;
}



static u64 _syscall_linux_renameat(void){
	ERROR("_syscall_linux_renameat: unimplemented");
	return 0;
}



static u64 _syscall_linux_linkat(void){
	ERROR("_syscall_linux_linkat: unimplemented");
	return 0;
}



static u64 _syscall_linux_symlinkat(void){
	ERROR("_syscall_linux_symlinkat: unimplemented");
	return 0;
}



static u64 _syscall_linux_readlinkat(void){
	ERROR("_syscall_linux_readlinkat: unimplemented");
	return 0;
}



static u64 _syscall_linux_fchmodat(void){
	ERROR("_syscall_linux_fchmodat: unimplemented");
	return 0;
}



static u64 _syscall_linux_faccessat(void){
	ERROR("_syscall_linux_faccessat: unimplemented");
	return 0;
}



static u64 _syscall_linux_pselect6(void){
	ERROR("_syscall_linux_pselect6: unimplemented");
	return 0;
}



static u64 _syscall_linux_ppoll(void){
	ERROR("_syscall_linux_ppoll: unimplemented");
	return 0;
}



static u64 _syscall_linux_unshare(void){
	ERROR("_syscall_linux_unshare: unimplemented");
	return 0;
}



static u64 _syscall_linux_set_robust_list(void){
	ERROR("_syscall_linux_set_robust_list: unimplemented");
	return 0;
}



static u64 _syscall_linux_get_robust_list(void){
	ERROR("_syscall_linux_get_robust_list: unimplemented");
	return 0;
}



static u64 _syscall_linux_splice(void){
	ERROR("_syscall_linux_splice: unimplemented");
	return 0;
}



static u64 _syscall_linux_tee(void){
	ERROR("_syscall_linux_tee: unimplemented");
	return 0;
}



static u64 _syscall_linux_sync_file_range(void){
	ERROR("_syscall_linux_sync_file_range: unimplemented");
	return 0;
}



static u64 _syscall_linux_vmsplice(void){
	ERROR("_syscall_linux_vmsplice: unimplemented");
	return 0;
}



static u64 _syscall_linux_move_pages(void){
	ERROR("_syscall_linux_move_pages: unimplemented");
	return 0;
}



static u64 _syscall_linux_utimensat(void){
	ERROR("_syscall_linux_utimensat: unimplemented");
	return 0;
}



static u64 _syscall_linux_epoll_pwait(void){
	ERROR("_syscall_linux_epoll_pwait: unimplemented");
	return 0;
}



static u64 _syscall_linux_signalfd(void){
	ERROR("_syscall_linux_signalfd: unimplemented");
	return 0;
}



static u64 _syscall_linux_timerfd_create(void){
	ERROR("_syscall_linux_timerfd_create: unimplemented");
	return 0;
}



static u64 _syscall_linux_eventfd(void){
	ERROR("_syscall_linux_eventfd: unimplemented");
	return 0;
}



static u64 _syscall_linux_fallocate(void){
	ERROR("_syscall_linux_fallocate: unimplemented");
	return 0;
}



static u64 _syscall_linux_timerfd_settime(void){
	ERROR("_syscall_linux_timerfd_settime: unimplemented");
	return 0;
}



static u64 _syscall_linux_timerfd_gettime(void){
	ERROR("_syscall_linux_timerfd_gettime: unimplemented");
	return 0;
}



static u64 _syscall_linux_accept4(void){
	ERROR("_syscall_linux_accept4: unimplemented");
	return 0;
}



static u64 _syscall_linux_signalfd4(void){
	ERROR("_syscall_linux_signalfd4: unimplemented");
	return 0;
}



static u64 _syscall_linux_eventfd2(void){
	ERROR("_syscall_linux_eventfd2: unimplemented");
	return 0;
}



static u64 _syscall_linux_epoll_create1(void){
	ERROR("_syscall_linux_epoll_create1: unimplemented");
	return 0;
}



static u64 _syscall_linux_dup3(void){
	ERROR("_syscall_linux_dup3: unimplemented");
	return 0;
}



static u64 _syscall_linux_pipe2(void){
	ERROR("_syscall_linux_pipe2: unimplemented");
	return 0;
}



static u64 _syscall_linux_inotify_init1(void){
	ERROR("_syscall_linux_inotify_init1: unimplemented");
	return 0;
}



static u64 _syscall_linux_preadv(void){
	ERROR("_syscall_linux_preadv: unimplemented");
	return 0;
}



static u64 _syscall_linux_pwritev(void){
	ERROR("_syscall_linux_pwritev: unimplemented");
	return 0;
}



static u64 _syscall_linux_rt_tgsigqueueinfo(void){
	ERROR("_syscall_linux_rt_tgsigqueueinfo: unimplemented");
	return 0;
}



static u64 _syscall_linux_perf_event_open(void){
	ERROR("_syscall_linux_perf_event_open: unimplemented");
	return 0;
}



static u64 _syscall_linux_recvmmsg(void){
	ERROR("_syscall_linux_recvmmsg: unimplemented");
	return 0;
}



static u64 _syscall_linux_fanotify_init(void){
	ERROR("_syscall_linux_fanotify_init: unimplemented");
	return 0;
}



static u64 _syscall_linux_fanotify_mark(void){
	ERROR("_syscall_linux_fanotify_mark: unimplemented");
	return 0;
}



static u64 _syscall_linux_prlimit64(void){
	ERROR("_syscall_linux_prlimit64: unimplemented");
	return 0;
}



static u64 _syscall_linux_name_to_handle_at(void){
	ERROR("_syscall_linux_name_to_handle_at: unimplemented");
	return 0;
}



static u64 _syscall_linux_open_by_handle_at(void){
	ERROR("_syscall_linux_open_by_handle_at: unimplemented");
	return 0;
}



static u64 _syscall_linux_clock_adjtime(void){
	ERROR("_syscall_linux_clock_adjtime: unimplemented");
	return 0;
}



static u64 _syscall_linux_syncfs(void){
	ERROR("_syscall_linux_syncfs: unimplemented");
	return 0;
}



static u64 _syscall_linux_sendmmsg(void){
	ERROR("_syscall_linux_sendmmsg: unimplemented");
	return 0;
}



static u64 _syscall_linux_setns(void){
	ERROR("_syscall_linux_setns: unimplemented");
	return 0;
}



static u64 _syscall_linux_getcpu(void){
	ERROR("_syscall_linux_getcpu: unimplemented");
	return 0;
}



static u64 _syscall_linux_process_vm_readv(void){
	ERROR("_syscall_linux_process_vm_readv: unimplemented");
	return 0;
}



static u64 _syscall_linux_process_vm_writev(void){
	ERROR("_syscall_linux_process_vm_writev: unimplemented");
	return 0;
}



static u64 _syscall_linux_kcmp(void){
	ERROR("_syscall_linux_kcmp: unimplemented");
	return 0;
}



static u64 _syscall_linux_finit_module(void){
	ERROR("_syscall_linux_finit_module: unimplemented");
	return 0;
}



static u64 _syscall_linux_sched_setattr(void){
	ERROR("_syscall_linux_sched_setattr: unimplemented");
	return 0;
}



static u64 _syscall_linux_sched_getattr(void){
	ERROR("_syscall_linux_sched_getattr: unimplemented");
	return 0;
}



static u64 _syscall_linux_renameat2(void){
	ERROR("_syscall_linux_renameat2: unimplemented");
	return 0;
}



static u64 _syscall_linux_seccomp(void){
	ERROR("_syscall_linux_seccomp: unimplemented");
	return 0;
}



static u64 _syscall_linux_getrandom(void){
	ERROR("_syscall_linux_getrandom: unimplemented");
	return 0;
}



static u64 _syscall_linux_memfd_create(void){
	ERROR("_syscall_linux_memfd_create: unimplemented");
	return 0;
}



static u64 _syscall_linux_kexec_file_load(void){
	ERROR("_syscall_linux_kexec_file_load: unimplemented");
	return 0;
}



static u64 _syscall_linux_bpf(void){
	ERROR("_syscall_linux_bpf: unimplemented");
	return 0;
}



static u64 _syscall_linux_execveat(void){
	ERROR("_syscall_linux_execveat: unimplemented");
	return 0;
}



static u64 _syscall_linux_userfaultfd(void){
	ERROR("_syscall_linux_userfaultfd: unimplemented");
	return 0;
}



static u64 _syscall_linux_membarrier(void){
	ERROR("_syscall_linux_membarrier: unimplemented");
	return 0;
}



static u64 _syscall_linux_mlock2(void){
	ERROR("_syscall_linux_mlock2: unimplemented");
	return 0;
}



static u64 _syscall_linux_copy_file_range(void){
	ERROR("_syscall_linux_copy_file_range: unimplemented");
	return 0;
}



static u64 _syscall_linux_preadv2(void){
	ERROR("_syscall_linux_preadv2: unimplemented");
	return 0;
}



static u64 _syscall_linux_pwritev2(void){
	ERROR("_syscall_linux_pwritev2: unimplemented");
	return 0;
}



static u64 _syscall_linux_pkey_mprotect(void){
	ERROR("_syscall_linux_pkey_mprotect: unimplemented");
	return 0;
}



static u64 _syscall_linux_pkey_alloc(void){
	ERROR("_syscall_linux_pkey_alloc: unimplemented");
	return 0;
}



static u64 _syscall_linux_pkey_free(void){
	ERROR("_syscall_linux_pkey_free: unimplemented");
	return 0;
}



static u64 _syscall_linux_statx(void){
	ERROR("_syscall_linux_statx: unimplemented");
	return 0;
}



static u64 _syscall_linux_io_pgetevents(void){
	ERROR("_syscall_linux_io_pgetevents: unimplemented");
	return 0;
}



static u64 _syscall_linux_rseq(void){
	ERROR("_syscall_linux_rseq: unimplemented");
	return 0;
}



static u64 _syscall_linux_pidfd_send_signal(void){
	ERROR("_syscall_linux_pidfd_send_signal: unimplemented");
	return 0;
}



static u64 _syscall_linux_io_uring_setup(void){
	ERROR("_syscall_linux_io_uring_setup: unimplemented");
	return 0;
}



static u64 _syscall_linux_io_uring_enter(void){
	ERROR("_syscall_linux_io_uring_enter: unimplemented");
	return 0;
}



static u64 _syscall_linux_io_uring_register(void){
	ERROR("_syscall_linux_io_uring_register: unimplemented");
	return 0;
}



static u64 _syscall_linux_open_tree(void){
	ERROR("_syscall_linux_open_tree: unimplemented");
	return 0;
}



static u64 _syscall_linux_move_mount(void){
	ERROR("_syscall_linux_move_mount: unimplemented");
	return 0;
}



static u64 _syscall_linux_fsopen(void){
	ERROR("_syscall_linux_fsopen: unimplemented");
	return 0;
}



static u64 _syscall_linux_fsconfig(void){
	ERROR("_syscall_linux_fsconfig: unimplemented");
	return 0;
}



static u64 _syscall_linux_fsmount(void){
	ERROR("_syscall_linux_fsmount: unimplemented");
	return 0;
}



static u64 _syscall_linux_fspick(void){
	ERROR("_syscall_linux_fspick: unimplemented");
	return 0;
}



static u64 _syscall_linux_pidfd_open(void){
	ERROR("_syscall_linux_pidfd_open: unimplemented");
	return 0;
}



static u64 _syscall_linux_clone3(void){
	ERROR("_syscall_linux_clone3: unimplemented");
	return 0;
}



static u64 _syscall_linux_close_range(void){
	ERROR("_syscall_linux_close_range: unimplemented");
	return 0;
}



static u64 _syscall_linux_openat2(void){
	ERROR("_syscall_linux_openat2: unimplemented");
	return 0;
}



static u64 _syscall_linux_pidfd_getfd(void){
	ERROR("_syscall_linux_pidfd_getfd: unimplemented");
	return 0;
}



static u64 _syscall_linux_faccessat2(void){
	ERROR("_syscall_linux_faccessat2: unimplemented");
	return 0;
}



static u64 _syscall_linux_process_madvise(void){
	ERROR("_syscall_linux_process_madvise: unimplemented");
	return 0;
}



static u64 _syscall_linux_epoll_pwait2(void){
	ERROR("_syscall_linux_epoll_pwait2: unimplemented");
	return 0;
}



static u64 _syscall_linux_mount_setattr(void){
	ERROR("_syscall_linux_mount_setattr: unimplemented");
	return 0;
}



static u64 _syscall_linux_quotactl_fd(void){
	ERROR("_syscall_linux_quotactl_fd: unimplemented");
	return 0;
}



static u64 _syscall_linux_landlock_create_ruleset(void){
	ERROR("_syscall_linux_landlock_create_ruleset: unimplemented");
	return 0;
}



static u64 _syscall_linux_landlock_add_rule(void){
	ERROR("_syscall_linux_landlock_add_rule: unimplemented");
	return 0;
}



static u64 _syscall_linux_landlock_restrict_self(void){
	ERROR("_syscall_linux_landlock_restrict_self: unimplemented");
	return 0;
}



static u64 _syscall_linux_memfd_secret(void){
	ERROR("_syscall_linux_memfd_secret: unimplemented");
	return 0;
}



static u64 _syscall_linux_process_mrelease(void){
	ERROR("_syscall_linux_process_mrelease: unimplemented");
	return 0;
}



static u64 _syscall_linux_futex_waitv(void){
	ERROR("_syscall_linux_futex_waitv: unimplemented");
	return 0;
}



static u64 _syscall_linux_set_mempolicy_home_node(void){
	ERROR("_syscall_linux_set_mempolicy_home_node: unimplemented");
	return 0;
}



static u64 _syscall_linux_cachestat(void){
	ERROR("_syscall_linux_cachestat: unimplemented");
	return 0;
}



static u64 _syscall_linux_fchmodat2(void){
	ERROR("_syscall_linux_fchmodat2: unimplemented");
	return 0;
}



static u64 _syscall_linux_map_shadow_stack(void){
	ERROR("_syscall_linux_map_shadow_stack: unimplemented");
	return 0;
}



static u64 _syscall_linux_futex_wake(void){
	ERROR("_syscall_linux_futex_wake: unimplemented");
	return 0;
}



static u64 _syscall_linux_futex_wait(void){
	ERROR("_syscall_linux_futex_wait: unimplemented");
	return 0;
}



static u64 _syscall_linux_futex_requeue(void){
	ERROR("_syscall_linux_futex_requeue: unimplemented");
	return 0;
}



static syscall_callback_t const _linux_syscall_functions[]={
	[0]=(syscall_callback_t)_syscall_linux_read,
	[1]=(syscall_callback_t)_syscall_linux_write,
	[2]=(syscall_callback_t)_syscall_linux_open,
	[3]=(syscall_callback_t)_syscall_linux_close,
	[4]=(syscall_callback_t)_syscall_linux_stat,
	[5]=(syscall_callback_t)_syscall_linux_fstat,
	[6]=(syscall_callback_t)_syscall_linux_lstat,
	[7]=(syscall_callback_t)_syscall_linux_poll,
	[8]=(syscall_callback_t)_syscall_linux_lseek,
	[9]=(syscall_callback_t)_syscall_linux_mmap,
	[10]=(syscall_callback_t)_syscall_linux_mprotect,
	[11]=(syscall_callback_t)_syscall_linux_munmap,
	[12]=(syscall_callback_t)_syscall_linux_brk,
	[13]=(syscall_callback_t)_syscall_linux_rt_sigaction,
	[14]=(syscall_callback_t)_syscall_linux_rt_sigprocmask,
	[15]=(syscall_callback_t)_syscall_linux_rt_sigreturn,
	[16]=(syscall_callback_t)_syscall_linux_ioctl,
	[17]=(syscall_callback_t)_syscall_linux_pread64,
	[18]=(syscall_callback_t)_syscall_linux_pwrite64,
	[19]=(syscall_callback_t)_syscall_linux_readv,
	[20]=(syscall_callback_t)_syscall_linux_writev,
	[21]=(syscall_callback_t)_syscall_linux_access,
	[22]=(syscall_callback_t)_syscall_linux_pipe,
	[23]=(syscall_callback_t)_syscall_linux_select,
	[24]=(syscall_callback_t)_syscall_linux_sched_yield,
	[25]=(syscall_callback_t)_syscall_linux_mremap,
	[26]=(syscall_callback_t)_syscall_linux_msync,
	[27]=(syscall_callback_t)_syscall_linux_mincore,
	[28]=(syscall_callback_t)_syscall_linux_madvise,
	[29]=(syscall_callback_t)_syscall_linux_shmget,
	[30]=(syscall_callback_t)_syscall_linux_shmat,
	[31]=(syscall_callback_t)_syscall_linux_shmctl,
	[32]=(syscall_callback_t)_syscall_linux_dup,
	[33]=(syscall_callback_t)_syscall_linux_dup2,
	[34]=(syscall_callback_t)_syscall_linux_pause,
	[35]=(syscall_callback_t)_syscall_linux_nanosleep,
	[36]=(syscall_callback_t)_syscall_linux_getitimer,
	[37]=(syscall_callback_t)_syscall_linux_alarm,
	[38]=(syscall_callback_t)_syscall_linux_setitimer,
	[39]=(syscall_callback_t)_syscall_linux_getpid,
	[40]=(syscall_callback_t)_syscall_linux_sendfile,
	[41]=(syscall_callback_t)_syscall_linux_socket,
	[42]=(syscall_callback_t)_syscall_linux_connect,
	[43]=(syscall_callback_t)_syscall_linux_accept,
	[44]=(syscall_callback_t)_syscall_linux_sendto,
	[45]=(syscall_callback_t)_syscall_linux_recvfrom,
	[46]=(syscall_callback_t)_syscall_linux_sendmsg,
	[47]=(syscall_callback_t)_syscall_linux_recvmsg,
	[48]=(syscall_callback_t)_syscall_linux_shutdown,
	[49]=(syscall_callback_t)_syscall_linux_bind,
	[50]=(syscall_callback_t)_syscall_linux_listen,
	[51]=(syscall_callback_t)_syscall_linux_getsockname,
	[52]=(syscall_callback_t)_syscall_linux_getpeername,
	[53]=(syscall_callback_t)_syscall_linux_socketpair,
	[54]=(syscall_callback_t)_syscall_linux_setsockopt,
	[55]=(syscall_callback_t)_syscall_linux_getsockopt,
	[56]=(syscall_callback_t)_syscall_linux_clone,
	[57]=(syscall_callback_t)_syscall_linux_fork,
	[58]=(syscall_callback_t)_syscall_linux_vfork,
	[59]=(syscall_callback_t)_syscall_linux_execve,
	[60]=(syscall_callback_t)_syscall_linux_exit,
	[61]=(syscall_callback_t)_syscall_linux_wait4,
	[62]=(syscall_callback_t)_syscall_linux_kill,
	[63]=(syscall_callback_t)_syscall_linux_uname,
	[64]=(syscall_callback_t)_syscall_linux_semget,
	[65]=(syscall_callback_t)_syscall_linux_semop,
	[66]=(syscall_callback_t)_syscall_linux_semctl,
	[67]=(syscall_callback_t)_syscall_linux_shmdt,
	[68]=(syscall_callback_t)_syscall_linux_msgget,
	[69]=(syscall_callback_t)_syscall_linux_msgsnd,
	[70]=(syscall_callback_t)_syscall_linux_msgrcv,
	[71]=(syscall_callback_t)_syscall_linux_msgctl,
	[72]=(syscall_callback_t)_syscall_linux_fcntl,
	[73]=(syscall_callback_t)_syscall_linux_flock,
	[74]=(syscall_callback_t)_syscall_linux_fsync,
	[75]=(syscall_callback_t)_syscall_linux_fdatasync,
	[76]=(syscall_callback_t)_syscall_linux_truncate,
	[77]=(syscall_callback_t)_syscall_linux_ftruncate,
	[78]=(syscall_callback_t)_syscall_linux_getdents,
	[79]=(syscall_callback_t)_syscall_linux_getcwd,
	[80]=(syscall_callback_t)_syscall_linux_chdir,
	[81]=(syscall_callback_t)_syscall_linux_fchdir,
	[82]=(syscall_callback_t)_syscall_linux_rename,
	[83]=(syscall_callback_t)_syscall_linux_mkdir,
	[84]=(syscall_callback_t)_syscall_linux_rmdir,
	[85]=(syscall_callback_t)_syscall_linux_creat,
	[86]=(syscall_callback_t)_syscall_linux_link,
	[87]=(syscall_callback_t)_syscall_linux_unlink,
	[88]=(syscall_callback_t)_syscall_linux_symlink,
	[89]=(syscall_callback_t)_syscall_linux_readlink,
	[90]=(syscall_callback_t)_syscall_linux_chmod,
	[91]=(syscall_callback_t)_syscall_linux_fchmod,
	[92]=(syscall_callback_t)_syscall_linux_chown,
	[93]=(syscall_callback_t)_syscall_linux_fchown,
	[94]=(syscall_callback_t)_syscall_linux_lchown,
	[95]=(syscall_callback_t)_syscall_linux_umask,
	[96]=(syscall_callback_t)_syscall_linux_gettimeofday,
	[97]=(syscall_callback_t)_syscall_linux_getrlimit,
	[98]=(syscall_callback_t)_syscall_linux_getrusage,
	[99]=(syscall_callback_t)_syscall_linux_sysinfo,
	[100]=(syscall_callback_t)_syscall_linux_times,
	[101]=(syscall_callback_t)_syscall_linux_ptrace,
	[102]=(syscall_callback_t)_syscall_linux_getuid,
	[103]=(syscall_callback_t)_syscall_linux_syslog,
	[104]=(syscall_callback_t)_syscall_linux_getgid,
	[105]=(syscall_callback_t)_syscall_linux_setuid,
	[106]=(syscall_callback_t)_syscall_linux_setgid,
	[107]=(syscall_callback_t)_syscall_linux_geteuid,
	[108]=(syscall_callback_t)_syscall_linux_getegid,
	[109]=(syscall_callback_t)_syscall_linux_setpgid,
	[110]=(syscall_callback_t)_syscall_linux_getppid,
	[111]=(syscall_callback_t)_syscall_linux_getpgrp,
	[112]=(syscall_callback_t)_syscall_linux_setsid,
	[113]=(syscall_callback_t)_syscall_linux_setreuid,
	[114]=(syscall_callback_t)_syscall_linux_setregid,
	[115]=(syscall_callback_t)_syscall_linux_getgroups,
	[116]=(syscall_callback_t)_syscall_linux_setgroups,
	[117]=(syscall_callback_t)_syscall_linux_setresuid,
	[118]=(syscall_callback_t)_syscall_linux_getresuid,
	[119]=(syscall_callback_t)_syscall_linux_setresgid,
	[120]=(syscall_callback_t)_syscall_linux_getresgid,
	[121]=(syscall_callback_t)_syscall_linux_getpgid,
	[122]=(syscall_callback_t)_syscall_linux_setfsuid,
	[123]=(syscall_callback_t)_syscall_linux_setfsgid,
	[124]=(syscall_callback_t)_syscall_linux_getsid,
	[125]=(syscall_callback_t)_syscall_linux_capget,
	[126]=(syscall_callback_t)_syscall_linux_capset,
	[127]=(syscall_callback_t)_syscall_linux_rt_sigpending,
	[128]=(syscall_callback_t)_syscall_linux_rt_sigtimedwait,
	[129]=(syscall_callback_t)_syscall_linux_rt_sigqueueinfo,
	[130]=(syscall_callback_t)_syscall_linux_rt_sigsuspend,
	[131]=(syscall_callback_t)_syscall_linux_sigaltstack,
	[132]=(syscall_callback_t)_syscall_linux_utime,
	[133]=(syscall_callback_t)_syscall_linux_mknod,
	[134]=(syscall_callback_t)_syscall_linux_uselib,
	[135]=(syscall_callback_t)_syscall_linux_personality,
	[136]=(syscall_callback_t)_syscall_linux_ustat,
	[137]=(syscall_callback_t)_syscall_linux_statfs,
	[138]=(syscall_callback_t)_syscall_linux_fstatfs,
	[139]=(syscall_callback_t)_syscall_linux_sysfs,
	[140]=(syscall_callback_t)_syscall_linux_getpriority,
	[141]=(syscall_callback_t)_syscall_linux_setpriority,
	[142]=(syscall_callback_t)_syscall_linux_sched_setparam,
	[143]=(syscall_callback_t)_syscall_linux_sched_getparam,
	[144]=(syscall_callback_t)_syscall_linux_sched_setscheduler,
	[145]=(syscall_callback_t)_syscall_linux_sched_getscheduler,
	[146]=(syscall_callback_t)_syscall_linux_sched_get_priority_max,
	[147]=(syscall_callback_t)_syscall_linux_sched_get_priority_min,
	[148]=(syscall_callback_t)_syscall_linux_sched_rr_get_interval,
	[149]=(syscall_callback_t)_syscall_linux_mlock,
	[150]=(syscall_callback_t)_syscall_linux_munlock,
	[151]=(syscall_callback_t)_syscall_linux_mlockall,
	[152]=(syscall_callback_t)_syscall_linux_munlockall,
	[153]=(syscall_callback_t)_syscall_linux_vhangup,
	[154]=(syscall_callback_t)_syscall_linux_modify_ldt,
	[155]=(syscall_callback_t)_syscall_linux_pivot_root,
	[156]=(syscall_callback_t)_syscall_linux__sysctl,
	[157]=(syscall_callback_t)_syscall_linux_prctl,
	[158]=(syscall_callback_t)_syscall_linux_arch_prctl,
	[159]=(syscall_callback_t)_syscall_linux_adjtimex,
	[160]=(syscall_callback_t)_syscall_linux_setrlimit,
	[161]=(syscall_callback_t)_syscall_linux_chroot,
	[162]=(syscall_callback_t)_syscall_linux_sync,
	[163]=(syscall_callback_t)_syscall_linux_acct,
	[164]=(syscall_callback_t)_syscall_linux_settimeofday,
	[165]=(syscall_callback_t)_syscall_linux_mount,
	[166]=(syscall_callback_t)_syscall_linux_umount2,
	[167]=(syscall_callback_t)_syscall_linux_swapon,
	[168]=(syscall_callback_t)_syscall_linux_swapoff,
	[169]=(syscall_callback_t)_syscall_linux_reboot,
	[170]=(syscall_callback_t)_syscall_linux_sethostname,
	[171]=(syscall_callback_t)_syscall_linux_setdomainname,
	[172]=(syscall_callback_t)_syscall_linux_iopl,
	[173]=(syscall_callback_t)_syscall_linux_ioperm,
	[174]=(syscall_callback_t)_syscall_linux_create_module,
	[175]=(syscall_callback_t)_syscall_linux_init_module,
	[176]=(syscall_callback_t)_syscall_linux_delete_module,
	[177]=(syscall_callback_t)_syscall_linux_get_kernel_syms,
	[178]=(syscall_callback_t)_syscall_linux_query_module,
	[179]=(syscall_callback_t)_syscall_linux_quotactl,
	[180]=(syscall_callback_t)_syscall_linux_nfsservctl,
	[181]=(syscall_callback_t)_syscall_linux_getpmsg,
	[182]=(syscall_callback_t)_syscall_linux_putpmsg,
	[183]=(syscall_callback_t)_syscall_linux_afs_syscall,
	[184]=(syscall_callback_t)_syscall_linux_tuxcall,
	[185]=(syscall_callback_t)_syscall_linux_security,
	[186]=(syscall_callback_t)_syscall_linux_gettid,
	[187]=(syscall_callback_t)_syscall_linux_readahead,
	[188]=(syscall_callback_t)_syscall_linux_setxattr,
	[189]=(syscall_callback_t)_syscall_linux_lsetxattr,
	[190]=(syscall_callback_t)_syscall_linux_fsetxattr,
	[191]=(syscall_callback_t)_syscall_linux_getxattr,
	[192]=(syscall_callback_t)_syscall_linux_lgetxattr,
	[193]=(syscall_callback_t)_syscall_linux_fgetxattr,
	[194]=(syscall_callback_t)_syscall_linux_listxattr,
	[195]=(syscall_callback_t)_syscall_linux_llistxattr,
	[196]=(syscall_callback_t)_syscall_linux_flistxattr,
	[197]=(syscall_callback_t)_syscall_linux_removexattr,
	[198]=(syscall_callback_t)_syscall_linux_lremovexattr,
	[199]=(syscall_callback_t)_syscall_linux_fremovexattr,
	[200]=(syscall_callback_t)_syscall_linux_tkill,
	[201]=(syscall_callback_t)_syscall_linux_time,
	[202]=(syscall_callback_t)_syscall_linux_futex,
	[203]=(syscall_callback_t)_syscall_linux_sched_setaffinity,
	[204]=(syscall_callback_t)_syscall_linux_sched_getaffinity,
	[205]=(syscall_callback_t)_syscall_linux_set_thread_area,
	[206]=(syscall_callback_t)_syscall_linux_io_setup,
	[207]=(syscall_callback_t)_syscall_linux_io_destroy,
	[208]=(syscall_callback_t)_syscall_linux_io_getevents,
	[209]=(syscall_callback_t)_syscall_linux_io_submit,
	[210]=(syscall_callback_t)_syscall_linux_io_cancel,
	[211]=(syscall_callback_t)_syscall_linux_get_thread_area,
	[212]=(syscall_callback_t)_syscall_linux_lookup_dcookie,
	[213]=(syscall_callback_t)_syscall_linux_epoll_create,
	[214]=(syscall_callback_t)_syscall_linux_epoll_ctl_old,
	[215]=(syscall_callback_t)_syscall_linux_epoll_wait_old,
	[216]=(syscall_callback_t)_syscall_linux_remap_file_pages,
	[217]=(syscall_callback_t)_syscall_linux_getdents64,
	[218]=(syscall_callback_t)_syscall_linux_set_tid_address,
	[219]=(syscall_callback_t)_syscall_linux_restart_syscall,
	[220]=(syscall_callback_t)_syscall_linux_semtimedop,
	[221]=(syscall_callback_t)_syscall_linux_fadvise64,
	[222]=(syscall_callback_t)_syscall_linux_timer_create,
	[223]=(syscall_callback_t)_syscall_linux_timer_settime,
	[224]=(syscall_callback_t)_syscall_linux_timer_gettime,
	[225]=(syscall_callback_t)_syscall_linux_timer_getoverrun,
	[226]=(syscall_callback_t)_syscall_linux_timer_delete,
	[227]=(syscall_callback_t)_syscall_linux_clock_settime,
	[228]=(syscall_callback_t)_syscall_linux_clock_gettime,
	[229]=(syscall_callback_t)_syscall_linux_clock_getres,
	[230]=(syscall_callback_t)_syscall_linux_clock_nanosleep,
	[231]=(syscall_callback_t)_syscall_linux_exit_group,
	[232]=(syscall_callback_t)_syscall_linux_epoll_wait,
	[233]=(syscall_callback_t)_syscall_linux_epoll_ctl,
	[234]=(syscall_callback_t)_syscall_linux_tgkill,
	[235]=(syscall_callback_t)_syscall_linux_utimes,
	[236]=(syscall_callback_t)_syscall_linux_vserver,
	[237]=(syscall_callback_t)_syscall_linux_mbind,
	[238]=(syscall_callback_t)_syscall_linux_set_mempolicy,
	[239]=(syscall_callback_t)_syscall_linux_get_mempolicy,
	[240]=(syscall_callback_t)_syscall_linux_mq_open,
	[241]=(syscall_callback_t)_syscall_linux_mq_unlink,
	[242]=(syscall_callback_t)_syscall_linux_mq_timedsend,
	[243]=(syscall_callback_t)_syscall_linux_mq_timedreceive,
	[244]=(syscall_callback_t)_syscall_linux_mq_notify,
	[245]=(syscall_callback_t)_syscall_linux_mq_getsetattr,
	[246]=(syscall_callback_t)_syscall_linux_kexec_load,
	[247]=(syscall_callback_t)_syscall_linux_waitid,
	[248]=(syscall_callback_t)_syscall_linux_add_key,
	[249]=(syscall_callback_t)_syscall_linux_request_key,
	[250]=(syscall_callback_t)_syscall_linux_keyctl,
	[251]=(syscall_callback_t)_syscall_linux_ioprio_set,
	[252]=(syscall_callback_t)_syscall_linux_ioprio_get,
	[253]=(syscall_callback_t)_syscall_linux_inotify_init,
	[254]=(syscall_callback_t)_syscall_linux_inotify_add_watch,
	[255]=(syscall_callback_t)_syscall_linux_inotify_rm_watch,
	[256]=(syscall_callback_t)_syscall_linux_migrate_pages,
	[257]=(syscall_callback_t)_syscall_linux_openat,
	[258]=(syscall_callback_t)_syscall_linux_mkdirat,
	[259]=(syscall_callback_t)_syscall_linux_mknodat,
	[260]=(syscall_callback_t)_syscall_linux_fchownat,
	[261]=(syscall_callback_t)_syscall_linux_futimesat,
	[262]=(syscall_callback_t)_syscall_linux_newfstatat,
	[263]=(syscall_callback_t)_syscall_linux_unlinkat,
	[264]=(syscall_callback_t)_syscall_linux_renameat,
	[265]=(syscall_callback_t)_syscall_linux_linkat,
	[266]=(syscall_callback_t)_syscall_linux_symlinkat,
	[267]=(syscall_callback_t)_syscall_linux_readlinkat,
	[268]=(syscall_callback_t)_syscall_linux_fchmodat,
	[269]=(syscall_callback_t)_syscall_linux_faccessat,
	[270]=(syscall_callback_t)_syscall_linux_pselect6,
	[271]=(syscall_callback_t)_syscall_linux_ppoll,
	[272]=(syscall_callback_t)_syscall_linux_unshare,
	[273]=(syscall_callback_t)_syscall_linux_set_robust_list,
	[274]=(syscall_callback_t)_syscall_linux_get_robust_list,
	[275]=(syscall_callback_t)_syscall_linux_splice,
	[276]=(syscall_callback_t)_syscall_linux_tee,
	[277]=(syscall_callback_t)_syscall_linux_sync_file_range,
	[278]=(syscall_callback_t)_syscall_linux_vmsplice,
	[279]=(syscall_callback_t)_syscall_linux_move_pages,
	[280]=(syscall_callback_t)_syscall_linux_utimensat,
	[281]=(syscall_callback_t)_syscall_linux_epoll_pwait,
	[282]=(syscall_callback_t)_syscall_linux_signalfd,
	[283]=(syscall_callback_t)_syscall_linux_timerfd_create,
	[284]=(syscall_callback_t)_syscall_linux_eventfd,
	[285]=(syscall_callback_t)_syscall_linux_fallocate,
	[286]=(syscall_callback_t)_syscall_linux_timerfd_settime,
	[287]=(syscall_callback_t)_syscall_linux_timerfd_gettime,
	[288]=(syscall_callback_t)_syscall_linux_accept4,
	[289]=(syscall_callback_t)_syscall_linux_signalfd4,
	[290]=(syscall_callback_t)_syscall_linux_eventfd2,
	[291]=(syscall_callback_t)_syscall_linux_epoll_create1,
	[292]=(syscall_callback_t)_syscall_linux_dup3,
	[293]=(syscall_callback_t)_syscall_linux_pipe2,
	[294]=(syscall_callback_t)_syscall_linux_inotify_init1,
	[295]=(syscall_callback_t)_syscall_linux_preadv,
	[296]=(syscall_callback_t)_syscall_linux_pwritev,
	[297]=(syscall_callback_t)_syscall_linux_rt_tgsigqueueinfo,
	[298]=(syscall_callback_t)_syscall_linux_perf_event_open,
	[299]=(syscall_callback_t)_syscall_linux_recvmmsg,
	[300]=(syscall_callback_t)_syscall_linux_fanotify_init,
	[301]=(syscall_callback_t)_syscall_linux_fanotify_mark,
	[302]=(syscall_callback_t)_syscall_linux_prlimit64,
	[303]=(syscall_callback_t)_syscall_linux_name_to_handle_at,
	[304]=(syscall_callback_t)_syscall_linux_open_by_handle_at,
	[305]=(syscall_callback_t)_syscall_linux_clock_adjtime,
	[306]=(syscall_callback_t)_syscall_linux_syncfs,
	[307]=(syscall_callback_t)_syscall_linux_sendmmsg,
	[308]=(syscall_callback_t)_syscall_linux_setns,
	[309]=(syscall_callback_t)_syscall_linux_getcpu,
	[310]=(syscall_callback_t)_syscall_linux_process_vm_readv,
	[311]=(syscall_callback_t)_syscall_linux_process_vm_writev,
	[312]=(syscall_callback_t)_syscall_linux_kcmp,
	[313]=(syscall_callback_t)_syscall_linux_finit_module,
	[314]=(syscall_callback_t)_syscall_linux_sched_setattr,
	[315]=(syscall_callback_t)_syscall_linux_sched_getattr,
	[316]=(syscall_callback_t)_syscall_linux_renameat2,
	[317]=(syscall_callback_t)_syscall_linux_seccomp,
	[318]=(syscall_callback_t)_syscall_linux_getrandom,
	[319]=(syscall_callback_t)_syscall_linux_memfd_create,
	[320]=(syscall_callback_t)_syscall_linux_kexec_file_load,
	[321]=(syscall_callback_t)_syscall_linux_bpf,
	[322]=(syscall_callback_t)_syscall_linux_execveat,
	[323]=(syscall_callback_t)_syscall_linux_userfaultfd,
	[324]=(syscall_callback_t)_syscall_linux_membarrier,
	[325]=(syscall_callback_t)_syscall_linux_mlock2,
	[326]=(syscall_callback_t)_syscall_linux_copy_file_range,
	[327]=(syscall_callback_t)_syscall_linux_preadv2,
	[328]=(syscall_callback_t)_syscall_linux_pwritev2,
	[329]=(syscall_callback_t)_syscall_linux_pkey_mprotect,
	[330]=(syscall_callback_t)_syscall_linux_pkey_alloc,
	[331]=(syscall_callback_t)_syscall_linux_pkey_free,
	[332]=(syscall_callback_t)_syscall_linux_statx,
	[333]=(syscall_callback_t)_syscall_linux_io_pgetevents,
	[334]=(syscall_callback_t)_syscall_linux_rseq,
	[424]=(syscall_callback_t)_syscall_linux_pidfd_send_signal,
	[425]=(syscall_callback_t)_syscall_linux_io_uring_setup,
	[426]=(syscall_callback_t)_syscall_linux_io_uring_enter,
	[427]=(syscall_callback_t)_syscall_linux_io_uring_register,
	[428]=(syscall_callback_t)_syscall_linux_open_tree,
	[429]=(syscall_callback_t)_syscall_linux_move_mount,
	[430]=(syscall_callback_t)_syscall_linux_fsopen,
	[431]=(syscall_callback_t)_syscall_linux_fsconfig,
	[432]=(syscall_callback_t)_syscall_linux_fsmount,
	[433]=(syscall_callback_t)_syscall_linux_fspick,
	[434]=(syscall_callback_t)_syscall_linux_pidfd_open,
	[435]=(syscall_callback_t)_syscall_linux_clone3,
	[436]=(syscall_callback_t)_syscall_linux_close_range,
	[437]=(syscall_callback_t)_syscall_linux_openat2,
	[438]=(syscall_callback_t)_syscall_linux_pidfd_getfd,
	[439]=(syscall_callback_t)_syscall_linux_faccessat2,
	[440]=(syscall_callback_t)_syscall_linux_process_madvise,
	[441]=(syscall_callback_t)_syscall_linux_epoll_pwait2,
	[442]=(syscall_callback_t)_syscall_linux_mount_setattr,
	[443]=(syscall_callback_t)_syscall_linux_quotactl_fd,
	[444]=(syscall_callback_t)_syscall_linux_landlock_create_ruleset,
	[445]=(syscall_callback_t)_syscall_linux_landlock_add_rule,
	[446]=(syscall_callback_t)_syscall_linux_landlock_restrict_self,
	[447]=(syscall_callback_t)_syscall_linux_memfd_secret,
	[448]=(syscall_callback_t)_syscall_linux_process_mrelease,
	[449]=(syscall_callback_t)_syscall_linux_futex_waitv,
	[450]=(syscall_callback_t)_syscall_linux_set_mempolicy_home_node,
	[451]=(syscall_callback_t)_syscall_linux_cachestat,
	[452]=(syscall_callback_t)_syscall_linux_fchmodat2,
	[453]=(syscall_callback_t)_syscall_linux_map_shadow_stack,
	[454]=(syscall_callback_t)_syscall_linux_futex_wake,
	[455]=(syscall_callback_t)_syscall_linux_futex_wait,
	[456]=(syscall_callback_t)_syscall_linux_futex_requeue,
};



MODULE_INIT(){
	LOG("Initializing linux syscalls...");
	syscall_update_table(0,_linux_syscall_functions,sizeof(_linux_syscall_functions)/sizeof(syscall_callback_t));
}



MODULE_DEINIT(){
	LOG("Removing linux syscalls...");
	syscall_update_table(0,NULL,0);
}
