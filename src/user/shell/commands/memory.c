#include <command.h>
#include <string.h>
#include <user/handle.h>
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
		handle_iterator_t iterator;
		HANDLE_FOREACH(&iterator,"omm_allocator"){
			// memory_object_allocator_data_t data;
			// if (!memory_object_allocator_get_data(iterator.handle,&data)){
			// 	continue;
			// }
			// printf("%s:\t\x1b[1m%lu\x1b[0m\n",data.name,data.allocation_count-data.deallocation_count);
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
	handle_iterator_t iterator;
	HANDLE_FOREACH(&iterator,"pmm_counter"){
		// memory_counter_data_t data;
		// if (!memory_counter_get_data(iterator.handle,&data)){
		// 	continue;
		// }
		// printf("%s:\t\x1b[1m%v\x1b[0m\n",data.name,data.count<<12);
	}
}



DECLARE_COMMAND(memory,"memory [-l|-o]");
