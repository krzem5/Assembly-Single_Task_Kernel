#ifndef _SYS2_MP_PROCESS_H_
#define _SYS2_MP_PROCESS_H_ 1
#include <sys2/mp/event.h>
#include <sys2/types.h>



#define SYS2_PROCESS_START_FLAG_PAUSE_THREAD 1



typedef u64 sys2_process_t;



sys2_event_t sys2_process_get_termination_event(sys2_process_t process);



sys2_process_t sys2_process_get_handle(void);



sys2_process_t sys2_process_start(const char* path,u32 argc,const char*const* argv,const char*const* environ,u32 flags);



#endif
