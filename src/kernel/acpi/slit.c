#include <kernel/log/log.h>
#include <kernel/numa/numa.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "slit"



typedef struct __attribute__((packed)) _SLIT{
	u32 signature;
	u32 length;
	u8 _padding[28];
	u64 locality_count;
	u8 data[];
} slit_t;



void acpi_slit_load(const void* slit_ptr){
	LOG("Loading SLIT...");
	const slit_t* slit=slit_ptr;
	if (numa_node_count!=slit->locality_count){
		panic("SLIT: numa domain count mismatch");
	}
	for (u64 i=0;i<slit->locality_count;i++){
		for (u64 j=0;j<slit->locality_count;j++){
			numa_set_locality(i,j,slit->data[i*slit->locality_count+j]);
		}
	}
}
