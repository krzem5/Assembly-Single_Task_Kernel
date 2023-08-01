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
	printf("Total: \x1b[1m%v\x1b[0m\n",stats.counter_total<<12);
	printf("Drive:\n  Drive list: \x1b[1m%v\x1b[0m\n  AHCI: \x1b[1m%v\x1b[0m\n",
		stats.counter_drive_list<<12,
		stats.counter_driver_ahci<<12
	);
	printf("FS:\n  FS: \x1b[1m%v\x1b[0m\n  FD: \x1b[1m%v\x1b[0m\n  Node allocator: \x1b[1m%v\x1b[0m\n  KFS: \x1b[1m%v\x1b[0m\n",
		stats.counter_fs<<12,
		stats.counter_fd<<12,
		stats.counter_kfs<<12
	);
	printf("Network:\n  Network: \x1b[1m%v\x1b[0m\n  i82540: \x1b[1m%v\x1b[0m\n",
		stats.counter_network<<12,
		stats.counter_driver_i82540<<12
	);
	printf("Memory:\n  PMM: \x1b[1m%v\x1b[0m\n  VMM: \x1b[1m%v\x1b[0m\n",
		stats.counter_pmm<<12,
		stats.counter_vmm<<12
	);
	printf("User:\n  ELF: \x1b[1m%v\x1b[0m\n  MMAP: \x1b[1m%v\x1b[0m\n  Stack: \x1b[1m%v\x1b[0m\n",
		stats.counter_elf<<12,
		stats.counter_mmap<<12,
		stats.counter_user_stack<<12
	);
	printf("Kernel:\n  Core: \x1b[1m%v\x1b[0m\n  Full: \x1b[1m%v\x1b[0m\n  Stack: \x1b[1m%v\x1b[0m\n",
		0,
		0,
		stats.counter_kernel_stack<<12
	);
}



DECLARE_COMMAND(memory,"memory <directory>");
