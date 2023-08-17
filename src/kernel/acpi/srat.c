#include <kernel/log/log.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "srat"



typedef struct __attribute__((packed)) _SRAT{
	u32 signature;
	u32 length;
	u8 _padding[28];
} srat_t;



void acpi_srat_load(const void* srat_ptr){
	LOG("Loading SRAT...");
	const srat_t* srat=srat_ptr;
	(void)srat;
}
