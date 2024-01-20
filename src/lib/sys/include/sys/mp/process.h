#ifndef _SYS_MP_PROCESS_H_
#define _SYS_MP_PROCESS_H_ 1
#include <sys/fd/fd.h>
#include <sys/mp/event.h>
#include <sys/types.h>



#define SYS_PROCESS_START_FLAG_PAUSE_THREAD 1



typedef u64 sys_process_t;



sys_event_t sys_process_get_termination_event(sys_process_t process);



sys_process_t sys_process_get_handle(void);



sys_process_t __attribute__((access(read_only,1),access(read_only,3,2),access(read_only,4))) sys_process_start(const char* path,u32 argc,const char*const* argv,const char*const* environ,u32 flags);



sys_error_t sys_process_set_cwd(sys_process_t process,sys_fd_t fd);



#endif
