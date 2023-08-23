#include <command.h>
#include <cwd.h>
#include <user/io.h>
#include <user/memory.h>



void memory_main(int argc,const char*const* argv){
	if (argc>1){
		printf("memory: unrecognized option '%s'\n",argv[2]);
		return;
	}
	memory_stats_t stats;
	if (!memory_stats(&stats)){
		printf("memory: unable to read memory stats\n");
		return;
	}
	printf("Total:\t\t\x1b[1m%v\x1b[0m\n",stats.counter_total<<12);
	printf("Free:\t\t\x1b[1m%v\x1b[0m\n",(stats.counter_total-stats.counter_driver_ahci-stats.counter_driver_i82540-stats.counter_kernel_stack-stats.counter_kfs-stats.counter_kmm-stats.counter_network-stats.counter_node_allocator-stats.counter_pmm-stats.counter_umm-stats.counter_user_stack-stats.counter_user-stats.counter_vmm)<<12);
	printf("AHCI:\t\t\x1b[1m%v\x1b[0m\n",stats.counter_driver_ahci<<12);
	printf("CPU:\t\t\x1b[1m%v\x1b[0m\n",stats.counter_cpu<<12);
	printf("i82540:\t\t\x1b[1m%v\x1b[0m\n",stats.counter_driver_i82540<<12);
	printf("Kernel stack:\t\x1b[1m%v\x1b[0m\n",stats.counter_kernel_stack<<12);
	printf("KFS:\t\t\x1b[1m%v\x1b[0m\n",stats.counter_kfs<<12);
	printf("KMM:\t\t\x1b[1m%v\x1b[0m\n",stats.counter_kmm<<12);
	printf("Network:\t\x1b[1m%v\x1b[0m\n",stats.counter_network<<12);
	printf("Node allocator:\t\x1b[1m%v\x1b[0m\n",stats.counter_node_allocator<<12);
	printf("PMM:\t\t\x1b[1m%v\x1b[0m\n",stats.counter_pmm<<12);
	printf("UMM:\t\t\x1b[1m%v\x1b[0m\n",stats.counter_umm<<12);
	printf("User stack:\t\x1b[1m%v\x1b[0m\n",stats.counter_user_stack<<12);
	printf("User:\t\t\x1b[1m%v\x1b[0m\n",stats.counter_user<<12);
	printf("VMM:\t\t\x1b[1m%v\x1b[0m\n",stats.counter_vmm<<12);
}



DECLARE_COMMAND(memory,"memory");
