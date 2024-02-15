#include <sys/error/error.h>
#include <sys/io/io.h>
#include <sys/mp/thread.h>
#include <sys/syscall/syscall.h>
#include <sys/types.h>



typedef struct _UI_PERMISSION_REQUEST{
	u64 id;
	char process[256];
	char handle[64];
	u64 flags;
} ui_permission_request_t;



static void _ui_permission_thread(void* arg){
	u64 offset=sys_syscall_get_table_offset("ui_permission_request");
	if (SYS_IS_ERROR(offset)){
		return;
	}
	while (1){
		ui_permission_request_t request;
		if (SYS_IS_ERROR(_sys_syscall2(offset|0x00000001,(u64)(&request),sizeof(ui_permission_request_t)))){
			continue;
		}
		sys_io_print("Permission request #%u: %s/%s:%x\n",request.id,request.process,request.handle,request.flags);
		_sys_syscall2(offset|0x00000002,request.id,1);
	}
}



void ui_permission_thread_start(void){
	sys_thread_create(_ui_permission_thread,NULL,0);
}
