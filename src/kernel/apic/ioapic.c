#include <kernel/io/io.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/msr/msr.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
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



static ioapic_t* KERNEL_INIT_WRITE _ioapic_data;
static u16 KERNEL_INIT_WRITE _ioapic_count;
static u16 KERNEL_EARLY_WRITE _ioapic_index;
static ioapic_override_t* KERNEL_INIT_WRITE _ioapic_override_data;
static u16 KERNEL_INIT_WRITE _ioapic_override_count;
static u16 KERNEL_EARLY_WRITE _ioapic_override_index;
static u16 KERNEL_INIT_WRITE _ioapic_irq_destination_cpu;



static KERNEL_INLINE u32 _read_register(const ioapic_t* ioapic,u32 reg){
	ioapic->registers[0]=reg;
	return ioapic->registers[4];
}



static KERNEL_INLINE void _write_register(const ioapic_t* ioapic,u32 reg,u32 value){
	ioapic->registers[0]=reg;
	ioapic->registers[4]=value;
}



void KERNEL_EARLY_EXEC ioapic_init(u16 count,u16 override_count){
	LOG("Initializing IOAPIC controllers...");
	if (!count){
		panic("No IOAPIC controllers found");
	}
	void* buffer=(void*)(pmm_alloc(pmm_align_up_address(count*sizeof(ioapic_t)+override_count*sizeof(ioapic_override_t))>>PAGE_SIZE_SHIFT,pmm_alloc_counter("ioapic"),0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	_ioapic_data=buffer;
	_ioapic_count=count;
	_ioapic_index=0;
	_ioapic_override_data=buffer+count*sizeof(ioapic_t);
	_ioapic_override_count=override_count;
	_ioapic_override_index=0;
	_ioapic_irq_destination_cpu=msr_get_apic_id();
	INFO("Disabling PIC...");
	io_port_out8(0xa1,0xff);
	io_port_out8(0x21,0xff);
}



void KERNEL_EARLY_EXEC ioapic_add(u8 apic_id,u32 address,u32 gsi_base){
	ioapic_t* ioapic=_ioapic_data+_ioapic_index;
	_ioapic_index++;
	ioapic->apic_id=apic_id;
	ioapic->gsi_base=gsi_base;
	ioapic->registers=(void*)vmm_identity_map(address,20);
	ioapic->gsi_count=(_read_register(ioapic,REGISTER_VER)>>16)+1;
	for (u16 i=0;i<ioapic->gsi_count;i++){
		_write_register(ioapic,(i+8)<<1,0x10000);
	}
}



void KERNEL_EARLY_EXEC ioapic_add_override(u8 irq,u32 gsi,u16 flags){
	(_ioapic_override_data+_ioapic_override_index)->irq=irq;
	(_ioapic_override_data+_ioapic_override_index)->flags=flags;
	(_ioapic_override_data+_ioapic_override_index)->gsi=gsi;
	_ioapic_override_index++;
}



KERNEL_PUBLIC void ioapic_redirect_irq(u8 irq,u8 vector){
	u16 flags=0;
	for (const ioapic_override_t* ioapic_override=_ioapic_override_data;ioapic_override<_ioapic_override_data+_ioapic_override_count;ioapic_override++){
		if (ioapic_override->irq==irq){
			irq=ioapic_override->gsi;
			flags=ioapic_override->flags;
			break;
		}
	}
	const ioapic_t* ioapic=_ioapic_data;
	for (;ioapic->gsi_base>irq||ioapic->gsi_base+ioapic->gsi_count<=irq;ioapic++){
		if (ioapic==_ioapic_data+_ioapic_count){
			panic("ioapic_redirect_irq: unable to find matching IOAPIC controller");
		}
	}
	_write_register(ioapic,((irq-ioapic->gsi_base)<<1)+16,vector|((flags&0x0a)<<12));
	_write_register(ioapic,((irq-ioapic->gsi_base)<<1)+17,_ioapic_irq_destination_cpu<<24);
}
