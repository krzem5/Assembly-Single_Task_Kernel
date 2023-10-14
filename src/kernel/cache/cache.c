#include <kernel/log/log.h>
#include <kernel/partition/partition.h>
#define KERNEL_LOG_NAME "cache"



void cache_flush(void){
	LOG("Flushing cache...");
	partition_flush_cache();
}
