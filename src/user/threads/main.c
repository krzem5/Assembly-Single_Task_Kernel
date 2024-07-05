#include <sys/error/error.h>
#include <sys/io/io.h>
#include <sys/mp/event.h>
#include <sys/mp/process.h>
#include <sys/mp/thread.h>
#include <sys/types.h>



int main(int argc,const char** argv){
	for (sys_thread_t thread=sys_thread_iter_start(SYS_THREAD_ITER_ALL_PROCESSES);thread;thread=sys_thread_iter_next(SYS_THREAD_ITER_ALL_PROCESSES,thread)){
		sys_thread_query_result_t thread_query_result;
		sys_process_query_result_t process_query_result;
		if (SYS_IS_ERROR(sys_thread_query(thread,&thread_query_result))||SYS_IS_ERROR(sys_process_query(thread_query_result.pid,&process_query_result))){
			sys_io_print("\x1b[1m<%p>\x1b[0m %d %d\n",thread,(sys_thread_query(thread,&thread_query_result)),(sys_process_query(thread_query_result.pid,&process_query_result)));
			continue;
		}
		const char* state="<unknown>";
		if (thread_query_result.state==SYS_THREAD_STATE_TYPE_NONE){
			state="<none>";
		}
		else if (thread_query_result.state==SYS_THREAD_STATE_TYPE_QUEUED){
			state="queued";
		}
		else if (thread_query_result.state==SYS_THREAD_STATE_TYPE_RUNNING){
			state="running";
		}
		else if (thread_query_result.state==SYS_THREAD_STATE_TYPE_AWAITING_EVENT){
			state="waiting";
		}
		else if (thread_query_result.state==SYS_THREAD_STATE_TYPE_TERMINATED){
			state="terminated";
		}
		const char* priority="<unknown>";
		if (thread_query_result.priority==SYS_THREAD_PRIORITY_BACKGROUND){
			priority="background";
		}
		else if (thread_query_result.priority==SYS_THREAD_PRIORITY_LOW){
			priority="low";
		}
		else if (thread_query_result.priority==SYS_THREAD_PRIORITY_NORMAL){
			priority="normal";
		}
		else if (thread_query_result.priority==SYS_THREAD_PRIORITY_HIGH){
			priority="high";
		}
		else if (thread_query_result.priority==SYS_THREAD_PRIORITY_REALTIME){
			priority="realtime";
		}
		else if (thread_query_result.priority==SYS_THREAD_PRIORITY_TERMINATED){
			priority="terminated";
		}
		sys_io_print("%s: \x1b[1m%s\x1b[0m: %s/%u, %s\n",process_query_result.name,thread_query_result.name,priority,thread_query_result.scheduler_priority,state);
	}
	return 0;
}
