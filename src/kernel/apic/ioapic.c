#include <kernel/cpu/cpu.h>
#include <kernel/io/io.h>
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



typedef struct _IOAPIC_OVERRIDE{
	u8 irq;
	u16 flags;
	u32 gsi;
} ioapic_override_t;



static ioapic_t* _ioapic_data;
static u16 _ioapic_count;
static u16 _ioapic_index;
static ioapic_override_t* _ioapic_override_data;
static u16 _ioapic_override_count;
static u16 _ioapic_override_index;



static u32 _read_register(ioapic_t* ioapic,u32 reg){
	ioapic->registers[0]=reg;
	return ioapic->registers[4];
}



static void _write_register(ioapic_t* ioapic,u32 reg,u32 value){
	ioapic->registers[0]=reg;
	ioapic->registers[4]=value;
}



void ioapic_init(u16 count,u16 override_count){
	LOG("Initializing IOAPIC controller...");
	_ioapic_data=(void*)pmm_alloc(pmm_align_up_address(count*sizeof(ioapic_t))>>PAGE_SIZE_SHIFT,PMM_COUNTER_CPU);
	_ioapic_count=count;
	_ioapic_index=0;
	_ioapic_override_data=(void*)pmm_alloc(pmm_align_up_address(override_count*sizeof(ioapic_override_t))>>PAGE_SIZE_SHIFT,PMM_COUNTER_CPU);
	_ioapic_override_count=override_count;
	_ioapic_override_index=0;
	INFO("Disabling PIC...");
	io_port_out8(0xa1,0xff);
	io_port_out8(0x21,0xff);
}



void ioapic_add(u8 apic_id,u32 address,u32 gsi_base){
	vmm_identity_map((void*)(u64)address,20);
	ioapic_t* ioapic=_ioapic_data+_ioapic_index;
	_ioapic_index++;
	ioapic->apic_id=apic_id;
	ioapic->gsi_base=gsi_base;
	ioapic->registers=(void*)(u64)address;
	ioapic->gsi_count=(_read_register(ioapic,REGISTER_VER)>>16)+1;
	for (u16 i=0;i<ioapic->gsi_count;i++){
		_write_register(ioapic,(i+8)<<1,0x10000);
	}
}



void ioapic_add_override(u8 irq,u32 gsi,u16 flags){
	(_ioapic_override_data+_ioapic_override_index)->irq=irq;
	(_ioapic_override_data+_ioapic_override_index)->flags=flags;
	(_ioapic_override_data+_ioapic_override_index)->gsi=gsi;
	_ioapic_override_index++;
}



void ioapic_redirect_irq(u8 irq,u8 vector){
	u16 flags=0;
	ioapic_override_t* ioapic_override=_ioapic_override_data;
	for (u16 i=0;i<_ioapic_override_count;i++){
		if (ioapic_override->irq==irq){
			irq=ioapic_override->gsi;
			flags=ioapic_override->flags;
			break;
		}
		ioapic_override++;
	}
	ioapic_t* ioapic=_ioapic_data;
	for (u16 i=0;i<_ioapic_count;i++){
		if (ioapic->gsi_base<=irq&&ioapic->gsi_base+ioapic->gsi_count>irq){
			break;
		}
		ioapic++;
	}
	_write_register(ioapic,((irq-ioapic->gsi_base)<<1)+16,vector|((flags&0x0a)<<12));
	_write_register(ioapic,((irq-ioapic->gsi_base)<<1)+17,cpu_bsp_core_id<<24);
}
