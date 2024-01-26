#include <sys/clock/clock.h>
#include <sys/error/error.h>
#include <sys/io/io.h>
#include <sys/syscall/syscall.h>
#include <sys/types.h>
#include <sys/util/options.h>



typedef struct _LOCKINFO_DATA_DESCRIPTOR{
	u32 type_line;
	u32 line;
	char func[64];
	char type_func[64];
	char name[128];
	char type_name[128];
	u64 count;
	u64 ticks;
	u64 max_ticks;
} lockinfo_data_descriptor_t;



int main(int argc,const char** argv){
	if (!sys_options_parse(argc,argv,NULL)){
		return 1;
	}
	u64 offset=sys_syscall_get_table_offset("lockinfo");
	if (SYS_IS_ERROR(offset)){
		sys_io_print("lockinfo: unable to access lockinfo kernel module\n");
		return 1;
	}
	lockinfo_data_descriptor_t data;
	for (u32 i=0;;i++){
		if (!_sys_syscall3(offset|0x00000001,i,0,(u64)(&data))){
			break;
		}
		sys_io_print("%s:%u \x1b[2;3m%s\x1b[0m\n",data.func,data.line,data.name);
		for (u32 j=0;;j++){
			if (!_sys_syscall3(offset|0x00000001,i,j,(u64)(&data))){
				break;
			}
			if (!data.count){
				continue;
			}
			sys_io_print("    %s:%u \x1b[2;3m%s\x1b[0m\n        cnt: \x1b[1m%lu\x1b[0m\n        avg: \x1b[1m%lu\x1b[0m ns\n        max: \x1b[1m%lu\x1b[0m ns\n",data.type_func,data.type_line,data.type_name,data.count,sys_clock_convert_ticks_to_time_ns(data.ticks/data.count),sys_clock_convert_ticks_to_time_ns(data.max_ticks));
		}
	}
	return 0;
}
