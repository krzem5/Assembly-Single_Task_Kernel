#include <kernel/log/log.h>
#include <kernel/network/layer3.h>
#include <kernel/partition/partition.h>
#define KERNEL_LOG_NAME "cache"



void cache_flush(void){
	LOG("Flushing cache...");
	network_layer3_flush_cache();
	partition_flush_cache();
}
