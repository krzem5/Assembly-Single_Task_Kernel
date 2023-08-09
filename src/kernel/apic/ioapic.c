#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "ioapic"



#define REGISTER_VER 0x01



typedef struct _IOAPIC{
	u8 apic_id;
	u16 gsi_count;
	u32 gsi_base;
	volatile u32* registers;
} ioapic_t;



static ioapic_t* _ioapic_data;
static u16 _ioapic_count;
static u16 _ioapic_index;



static u32 _read_register(ioapic_t* ioapic,u32 reg){
	ioapic->registers[0]=reg;
	return ioapic->registers[4];
}



void ioapic_init(u16 count){
	LOG("Initializing IOAPIC controller...");
	_ioapic_data=VMM_TRANSLATE_ADDRESS(pmm_alloc(pmm_align_up_address(count*sizeof(ioapic_t))>>PAGE_SIZE_SHIFT,PMM_COUNTER_CPU));
	_ioapic_count=count;
	_ioapic_index=0;
}



void ioapic_add(u8 apic_id,u32 address,u32 gsi_base){
	(_ioapic_data+_ioapic_index)->apic_id=apic_id;
	(_ioapic_data+_ioapic_index)->gsi_base=gsi_base;
	(_ioapic_data+_ioapic_index)->registers=VMM_TRANSLATE_ADDRESS(address);
	(_ioapic_data+_ioapic_index)->gsi_count=(_read_register(_ioapic_data+_ioapic_index,REGISTER_VER)>>16)+1;
	_ioapic_index++;
}
