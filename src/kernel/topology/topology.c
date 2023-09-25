#include <kernel/log/log.h>
#include <kernel/topology/topology.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "topology"



void topology_compute(u8 apic_id,topology_t* out){
	u64 bits=topology_get_cpu_bits();
	u32 thread_bits=bits;
	u32 core_bits=bits>>32;
	out->thread=apic_id&((1<<thread_bits)-1);
	out->core=(apic_id>>thread_bits)&((1<<core_bits)-1);
	out->chip=apic_id>>(thread_bits+core_bits);
	out->domain=0;
}
