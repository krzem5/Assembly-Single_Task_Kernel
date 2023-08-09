#include <user/partition.h>
#include <user/syscall.h>
#include <user/types.h>



partition_t __attribute__((section(".bss.partitions"))) partitions[MAX_PARTITIONS];
u32 partition_count;
u32 partition_boot_index;



void partition_init(void){
	for (u32 i=0;i<MAX_PARTITIONS;i++){
		partitions[i].flags=0;
	}
	partition_count=_syscall_partition_count();
	if (partition_count>MAX_PARTITIONS){
		partition_count=MAX_PARTITIONS;
	}
	for (u32 i=0;i<partition_count;i++){
		if (_syscall_partition_get(i,partitions+i,sizeof(partition_t))<0){
			partitions[i].flags=0;
		}
		if (partitions[i].flags&PARTITION_FLAG_BOOT){
			partition_boot_index=i;
		}
	}
}
