#include <sys/clock/clock.h>
#include <sys/error/error.h>
#include <sys/io/io.h>
#include <sys/syscall/syscall.h>
#include <sys/types.h>
#include <sys/util/options.h>



typedef struct _LOCKINFO_DESCRIPTOR{
	u32 id;
	char module[64];
	char func[128];
	u64 offset;
} lockinfo_descriptor_t;



typedef struct _LOCKINFO_STATS{
	u32 id;
	char module[64];
	char func[128];
	u64 offset;
	u64 count;
	u64 ticks;
	u64 max_ticks;
} lockinfo_stats_t;



int main(int argc,const char** argv){
	if (!sys_options_parse(argc,argv,NULL)){
		return 1;
	}
	u64 offset=sys_syscall_get_table_offset("lockinfo");
	if (SYS_IS_ERROR(offset)){
		sys_io_print("lockinfo: unable to access lockinfo kernel module\n");
		return 1;
	}
	lockinfo_descriptor_t descriptor;
	for (u32 i=0;;i++){
		if (!_sys_syscall2(offset|0x00000001,i,(u64)(&descriptor))){
			break;
		}
		sys_io_print("%u: %s:%s+%u\n",descriptor.id,descriptor.module,descriptor.func,descriptor.offset);
	}
	lockinfo_stats_t stats;
	for (u32 i=0;;i++){
		if (!_sys_syscall2(offset|0x00000002,i,(u64)(&stats))){
			break;
		}
		if (!stats.count){
			continue;
		}
		sys_io_print("~ %u: %s:%s+%u: %lu, %lu, %lu\n",stats.id,stats.module,stats.func,stats.offset,stats.count,sys_clock_convert_ticks_to_time_ns(stats.ticks/stats.count),sys_clock_convert_ticks_to_time_ns(stats.max_ticks));
	}
	return 0;
}
