#include <command.h>
#include <string.h>
#include <user/io.h>
#include <user/memory.h>



#define MEMORY_SHOW_COUNTERS 0
#define MEMORY_SHOW_OBJECT_COUNTERS 1
#define MEMORY_SHOW_RANGES 2



void memory_main(int argc,const char*const* argv){
	u8 command_type=MEMORY_SHOW_COUNTERS;
	for (u32 i=1;i<argc;i++){
		if (string_equal(argv[i],"-l")){
			command_type=MEMORY_SHOW_RANGES;
		}
		else if (string_equal(argv[i],"-o")){
			command_type=MEMORY_SHOW_OBJECT_COUNTERS;
		}
		else{
			printf("memory: unrecognized option '%s'\n",argv[i]);
			return;
		}
	}
	if (command_type==MEMORY_SHOW_OBJECT_COUNTERS){
		u32 counter_count=memory_get_object_counter_count();
		if (!counter_count){
			goto _counter_error;
		}
		for (u32 i=0;i<counter_count;i++){
			memory_object_counter_t counter;
			if (!memory_get_object_counter(i,&counter)){
				goto _counter_error;
			}
			u8 j=0;
			for (;counter.name[j];j++);
			printf("%s:\t%s\x1b[1m%lu\x1b[0m\n",counter.name,(j>6?"":"\t"),counter.allocation_count-counter.deallocation_count);
		}
		return;
	}
	if (command_type==MEMORY_SHOW_RANGES){
		printf("Memory layout:\n");
		u64 total_size=0;
		memory_range_t range;
		for (u32 i=0;memory_get_range(i,&range);i++){
			printf("  %p - %p (\x1b[1m%v\x1b[0m)\n",range.base_address,range.base_address+range.length,range.length);
			total_size+=range.length;
		}
		printf("Total: \x1b[1m%v\x1b[0m (\x1b[1m%lu\x1b[0m B)\n",total_size,total_size);
		return;
	}
	u32 counter_count=memory_get_counter_count();
	if (!counter_count){
		goto _counter_error;
	}
	for (u32 i=0;i<counter_count;i++){
		memory_counter_t counter;
		if (!memory_get_counter(i,&counter)){
			goto _counter_error;
		}
		u8 j=0;
		for (;counter.name[j];j++);
		printf("%s:\t%s\x1b[1m%v\x1b[0m\n",counter.name,(j>6?"":"\t"),counter.count<<12);
	}
	return;
_counter_error:
	printf("memory: unable to read memory counter\n");
}



DECLARE_COMMAND(memory,"memory [-l|-o]");
