#include <kernel/log/log.h>
#include <kernel/types.h>
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
	LOG("Locality count: %u",slit->locality_count);
	for (u64 i=0;i<slit->locality_count;i++){
		for (u64 j=0;j<slit->locality_count;j++){
			INFO("%u -> %u: %u",i,j,slit->data[i*slit->locality_count+j]);
		}
	}
}
