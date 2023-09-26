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
		if (!memory_range_count){
			printf("memory: unable to access memory ranges\n");
			return;
		}
		printf("Memory layout: (\x1b[1m%u\x1b[0m range%s)\n",memory_range_count,(memory_range_count==1?"":"s"));
		u64 total_size=0;
		u64 total_usable_size=0;
		const memory_range_t* range=memory_ranges;
		for (u32 i=0;i<memory_range_count;i++){
			const char* type=" [Unusable]";
			switch (range->type){
				case MEMORY_RANGE_TYPE_NORMAL:
					type="";
					break;
				case MEMORY_RANGE_TYPE_ACPI_TABLES:
					type=" [ACPI tables]";
					break;
				case MEMORY_RANGE_TYPE_ACPI_NVS:
					type=" [ACPI NVS]";
					break;
				case MEMORY_RANGE_TYPE_BAD_MEMORY:
					type=" [Bad memory]";
					break;
			}
			printf("  %p - %p (\x1b[1m%v\x1b[0m)%s\n",range->base_address,range->base_address+range->length,range->length,type);
			total_size+=range->length;
			if (range->type==MEMORY_RANGE_TYPE_NORMAL){
				total_usable_size+=range->length;
			}
			range++;
		}
		printf("Total: \x1b[1m%v\x1b[0m (\x1b[1m%v\x1b[0m usable)\n",total_size,total_usable_size);
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
