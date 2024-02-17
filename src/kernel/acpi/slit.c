#include <kernel/acpi/structures.h>
#include <kernel/log/log.h>
#include <kernel/numa/numa.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "slit"



void KERNEL_EARLY_EXEC acpi_slit_load(const acpi_slit_t* slit){
	LOG("Loading SLIT...");
	u32 size=(slit->locality_count<numa_node_count?slit->locality_count:numa_node_count);
	for (u64 i=0;i<size;i++){
		for (u64 j=0;j<size;j++){
			numa_set_locality(i,j,slit->data[i*slit->locality_count+j]);
		}
	}
}
