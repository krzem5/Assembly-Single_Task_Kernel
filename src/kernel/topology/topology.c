#include <kernel/log/log.h>
#include <kernel/topology/topology.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "topology"



static u32 _topology_thread_bits=0xffffffff;
static u32 _topology_core_bits;



void topology_compute(u8 apic_id,topology_t* out){
	if (_topology_thread_bits==0xffffffff){
		u64 bits=topology_get_cpu_bits();
		_topology_thread_bits=bits;
		_topology_core_bits=bits>>32;
	}
	out->thread=apic_id&((1<<_topology_thread_bits)-1);
	out->core=(apic_id>>_topology_thread_bits)&((1<<_topology_core_bits)-1);
	out->chip=apic_id>>(_topology_thread_bits+_topology_core_bits);
	ERROR("%u:%u:%u",out->chip,out->core,out->thread);
}
