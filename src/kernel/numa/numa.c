#include <kernel/cpu/cpu.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/numa/numa.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "numa"



static pmm_counter_descriptor_t _numa_pmm_counter=PMM_COUNTER_INIT_STRUCT("numa");



static numa_cpu_t* KERNEL_INIT_WRITE _numa_cpus;
static numa_memory_range_t* KERNEL_INIT_WRITE _numa_memory_ranges;
static u32 KERNEL_INIT_WRITE _numa_remaining_cpus;
static u32 KERNEL_INIT_WRITE _numa_remaining_memory_ranges;

u32 KERNEL_INIT_WRITE numa_node_count=0;
numa_node_t* KERNEL_INIT_WRITE numa_nodes;
u8* KERNEL_INIT_WRITE numa_node_locality_matrix;



void numa_init(u32 proximity_domain_count,u32 cpu_count,u32 memory_range_count){
	LOG("Initializing NUMA...");
	INFO("Proximity domain count: %u",proximity_domain_count);
	void* buffer=(void*)(pmm_alloc(pmm_align_up_address(cpu_count*sizeof(numa_cpu_t)+memory_range_count*sizeof(numa_memory_range_t)+numa_node_count*sizeof(numa_node_t)+numa_node_count*numa_node_count*sizeof(u8))>>PAGE_SIZE_SHIFT,&_numa_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	_numa_cpus=buffer;
	buffer+=cpu_count*sizeof(numa_cpu_t);
	_numa_memory_ranges=buffer;
	buffer+=memory_range_count*sizeof(numa_memory_range_t);
	_numa_remaining_cpus=cpu_count;
	_numa_remaining_memory_ranges=memory_range_count;
	numa_node_count=proximity_domain_count;
	numa_nodes=buffer;
	buffer+=numa_node_count*sizeof(numa_node_t);
	numa_node_locality_matrix=buffer;
	for (u32 i=0;i<numa_node_count;i++){
		(numa_nodes+i)->index=i;
		(numa_nodes+i)->cpu_count=0;
		(numa_nodes+i)->memory_range_count=0;
		(numa_nodes+i)->cpus=NULL;
		(numa_nodes+i)->memory_ranges=NULL;
		for (u32 j=0;j<numa_node_count;j++){
			numa_node_locality_matrix[NUMA_LOCALITY_INDEX(i,j)]=0;
		}
	}
}



void numa_set_locality(u32 from,u32 to,u8 value){
	if (from>=numa_node_count||to>=numa_node_count){
		panic("NUMA index out-of-range");
	}
	numa_node_locality_matrix[NUMA_LOCALITY_INDEX(from,to)]=value;
}



void numa_add_cpu(u32 node_index,u8 apic_id,u32 sapic_eid){
	if (node_index>=numa_node_count||!_numa_remaining_cpus){
		panic("NUMA index out-of-range");
	}
	_numa_remaining_cpus--;
	numa_cpu_t* cpu=_numa_cpus+_numa_remaining_cpus;
	cpu->next=(numa_nodes+node_index)->cpus;
	cpu->apic_id=apic_id;
	cpu->sapic_eid=sapic_eid;
	(numa_nodes+node_index)->cpu_count++;
	(numa_nodes+node_index)->cpus=cpu;
	(cpu_extra_data+apic_id)->topology.domain=node_index;
}



void numa_add_memory_range(u32 node_index,u64 base_address,u64 length,_Bool hot_pluggable){
	if (node_index>=numa_node_count||!_numa_remaining_memory_ranges){
		panic("NUMA index out-of-range");
	}
	_numa_remaining_memory_ranges--;
	numa_memory_range_t* memory_range=_numa_memory_ranges+_numa_remaining_memory_ranges;
	memory_range->next=(numa_nodes+node_index)->memory_ranges;
	memory_range->base_address=base_address;
	memory_range->length=length;
	memory_range->hot_pluggable=hot_pluggable;
	(numa_nodes+node_index)->memory_range_count++;
	(numa_nodes+node_index)->memory_ranges=memory_range;
}
