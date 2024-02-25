#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/random/random.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "aslr"



#define KERNEL_ASLR_KERNEL_END (kernel_get_offset()+0x8000000)
#define KERNEL_ASLR_MODULE_START (kernel_get_offset()+0xc000000)

volatile const u64* KERNEL_EARLY_READ __kernel_relocation_data=NULL;



void KERNEL_EARLY_EXEC aslr_reloc_kernel(void){
	LOG("Relocating kernel...");
	u64 kernel_size=pmm_align_up_address(kernel_data.first_free_address);
	u64 base;
	random_generate(&base,sizeof(u64));
	base=kernel_data.first_free_address+pmm_align_down_address(base%(KERNEL_ASLR_KERNEL_END-kernel_size-kernel_get_offset()-kernel_data.first_free_address-1));
	INFO("Kernel base: %p",base);
	for (u64 i=0;__kernel_relocation_data[i];i++){
		(void)__kernel_relocation_data[i];
	}
	random_generate(&base,sizeof(u64));
	kernel_data.first_free_address=KERNEL_ASLR_KERNEL_END+pmm_align_down_address(base%(KERNEL_ASLR_MODULE_START-KERNEL_ASLR_KERNEL_END))-kernel_get_offset();
	INFO("Module base: %p",kernel_data.first_free_address+kernel_get_offset());
}
