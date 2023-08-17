#include <kernel/log/log.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "slit"



typedef struct __attribute__((packed)) _SLIT{
	u32 signature;
	u32 length;
	u8 _padding[28];
} slit_t;



void acpi_slit_load(const void* slit_ptr){
	LOG("Loading SLIT...");
	const slit_t* slit=slit_ptr;
	(void)slit;
}
