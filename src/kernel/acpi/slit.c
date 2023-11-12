#include <kernel/acpi/structures.h>
#include <kernel/log/log.h>
#include <kernel/numa/numa.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "slit"



void acpi_slit_load(const acpi_slit_t* slit){
	LOG("Loading SLIT...");
	if (numa_node_count!=slit->locality_count){
		panic("SLIT: numa domain count mismatch");
	}
	for (u64 i=0;i<slit->locality_count;i++){
		for (u64 j=0;j<slit->locality_count;j++){
			numa_set_locality(i,j,slit->data[i*slit->locality_count+j]);
		}
	}
}
