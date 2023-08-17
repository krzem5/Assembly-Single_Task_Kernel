#include <kernel/log/log.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "hmat"



typedef struct __attribute__((packed)) _HMAT{
	u32 signature;
	u32 length;
	u8 _padding[28];
} hmat_t;



void acpi_hmat_load(const void* hmat_ptr){
	LOG("Loading HMAT...");
	const hmat_t* hmat=hmat_ptr;
	(void)hmat;
}
