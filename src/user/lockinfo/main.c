#include <sys/clock/clock.h>
#include <sys/error/error.h>
#include <sys/heap/heap.h>
#include <sys/io/io.h>
#include <sys/memory/memory.h>
#include <sys/syscall/syscall.h>
#include <sys/types.h>
#include <sys/util/options.h>



#define LOCKINFO_FLAG_PREEMPTIBLE 1

#define SKIP_FAST_LOCKS 1



typedef struct _LOCKINFO_DESCRIPTOR{
	u32 id;
	u32 flags;
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



typedef struct _SYMBOL{
	char module[64];
	char func[128];
	u64 offset;
} symbol_t;



typedef struct _USAGE_DESCRIPTOR{
	symbol_t symbol;
	u64 count;
	u64 avg_time;
	u64 max_time;
} usage_descriptor_t;



typedef struct _DESCRIPTOR{
	symbol_t symbol;
	u32 flags;
	usage_descriptor_t* data;
	u32 length;
} descriptor_t;



static void _quicksort_usage_data(usage_descriptor_t* data,u32 length){
	u32 i=0;
	for (u32 j=0;j<length;j++){
		if ((data+j)->avg_time>(data+length)->avg_time){
			usage_descriptor_t tmp=*(data+i);
			*(data+i)=*(data+j);
			*(data+j)=tmp;
			i++;
		}
	}
	usage_descriptor_t tmp=*(data+i);
	*(data+i)=*(data+length);
	*(data+length)=tmp;
	if (i>1){
		_quicksort_usage_data(data,i-1);
	}
	i++;
	if (i<length){
		_quicksort_usage_data(data+i,length-i);
	}
}



int main(int argc,const char** argv){
	if (!sys_options_parse(argc,argv,NULL)){
		return 1;
	}
	u64 offset=sys_syscall_get_table_offset("lockinfo");
	if (SYS_IS_ERROR(offset)){
		sys_io_print("lockinfo: unable to access lockinfo kernel module\n");
		return 1;
	}
	descriptor_t* data=NULL;
	u32 data_length=0;
	lockinfo_descriptor_t descriptor;
	for (u32 i=0;;i++){
		if (!_sys_syscall2(offset|0x00000001,i,(u64)(&descriptor))||descriptor.id!=i){
			break;
		}
		data_length++;
		data=sys_heap_realloc(NULL,data,data_length*sizeof(descriptor_t));
		sys_memory_copy(descriptor.module,(data+data_length-1)->symbol.module,sizeof(descriptor.module));
		sys_memory_copy(descriptor.func,(data+data_length-1)->symbol.func,sizeof(descriptor.func));
		(data+data_length-1)->symbol.offset=descriptor.offset;
		(data+data_length-1)->flags=descriptor.flags;
		(data+data_length-1)->data=NULL;
		(data+data_length-1)->length=0;
	}
	lockinfo_stats_t stats;
	for (u32 i=0;;i++){
		if (!_sys_syscall2(offset|0x00000002,i,(u64)(&stats))){
			break;
		}
		u64 avg_time=sys_clock_convert_ticks_to_time_ns(stats.ticks/stats.count);
		if (!stats.count||stats.id>=data_length||(SKIP_FAST_LOCKS&&avg_time<1000)){
			continue;
		}
		descriptor_t* desc=data+stats.id;
		desc->length++;
		desc->data=sys_heap_realloc(NULL,desc->data,desc->length*sizeof(usage_descriptor_t));
		sys_memory_copy(stats.module,(desc->data+desc->length-1)->symbol.module,sizeof(stats.module));
		sys_memory_copy(stats.func,(desc->data+desc->length-1)->symbol.func,sizeof(stats.func));
		(desc->data+desc->length-1)->symbol.offset=stats.offset;
		(desc->data+desc->length-1)->count=stats.count;
		(desc->data+desc->length-1)->avg_time=avg_time;
		(desc->data+desc->length-1)->max_time=sys_clock_convert_ticks_to_time_ns(stats.max_ticks);
	}
	for (u32 i=0;i<data_length;){
		if (!(data+i)->length){
			data_length--;
			*(data+i)=*(data+data_length);
			continue;
		}
		_quicksort_usage_data((data+i)->data,(data+i)->length-1);
		i++;
	}
	for (u32 i=0;i<data_length;i++){
		descriptor_t* desc=data+i;
		sys_io_print("\x1b[3m%s:%s+%lu\x1b[0m%s\n",desc->symbol.module,desc->symbol.func,desc->symbol.offset,((desc->flags&LOCKINFO_FLAG_PREEMPTIBLE)?" (preemptible)":""));
		for (u32 j=0;j<desc->length;j++){
			sys_io_print("  \x1b[3m%s:%s+%lu\x1b[0m\n    cnt: \x1b[1m%lu\x1b[0m\n    avg: \x1b[1m%lu\x1b[0mμs\n    max: \x1b[1m%lu\x1b[0mμs\n",(desc->data+j)->symbol.module,(desc->data+j)->symbol.func,(desc->data+j)->symbol.offset,(desc->data+j)->count,(desc->data+j)->avg_time/1000,(desc->data+j)->max_time/1000);
		}
	}
	return 0;
}
