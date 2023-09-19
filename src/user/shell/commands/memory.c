#include <command.h>
#include <cwd.h>
#include <string.h>
#include <user/io.h>
#include <user/memory.h>



void memory_main(int argc,const char*const* argv){
	_Bool show_layout=0;
	for (u32 i=1;i<argc;i++){
		if (string_equal(argv[i],"-l")){
			show_layout=1;
		}
		else{
			printf("memory: unrecognized option '%s'\n",argv[i]);
			return;
		}
	}
	if (show_layout){
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
	memory_stats_t stats;
	if (!memory_stats(&stats)){
		printf("memory: unable to read memory stats\n");
		return;
	}
	printf("Total:\t\t\x1b[1m%v\x1b[0m\n",stats.counter_total<<12);
	printf("Free:\t\t\x1b[1m%v\x1b[0m\n",stats.counter_free<<12);
	printf("AHCI:\t\t\x1b[1m%v\x1b[0m\n",stats.counter_driver_ahci<<12);
	printf("i82540:\t\t\x1b[1m%v\x1b[0m\n",stats.counter_driver_i82540<<12);
	printf("Image:\t\t\x1b[1m%v\x1b[0m\n",stats.counter_image<<12);
	printf("Kernel image:\t\x1b[1m%v\x1b[0m\n",stats.counter_kernel_image<<12);
	printf("Kernel stack:\t\x1b[1m%v\x1b[0m\n",stats.counter_kernel_stack<<12);
	printf("KFS:\t\t\x1b[1m%v\x1b[0m\n",stats.counter_kfs<<12);
	printf("KMM:\t\t\x1b[1m%v\x1b[0m\n",stats.counter_kmm<<12);
	printf("Network:\t\x1b[1m%v\x1b[0m\n",stats.counter_network<<12);
	printf("UMM:\t\t\x1b[1m%v\x1b[0m\n",stats.counter_umm<<12);
	printf("User stack:\t\x1b[1m%v\x1b[0m\n",stats.counter_user_stack<<12);
	printf("User:\t\t\x1b[1m%v\x1b[0m\n",stats.counter_user<<12);
	printf("VMM:\t\t\x1b[1m%v\x1b[0m\n",stats.counter_vmm<<12);
}



DECLARE_COMMAND(memory,"memory [-l]");
