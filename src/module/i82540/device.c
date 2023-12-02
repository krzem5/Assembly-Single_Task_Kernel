#include <i82540/device.h>
#include <i82540/registers.h>
#include <kernel/apic/ioapic.h>
#include <kernel/handle/handle.h>
#include <kernel/isr/isr.h>
#include <kernel/kernel.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/module/module.h>
#include <kernel/mp/thread.h>
#include <kernel/network/layer1.h>
#include <kernel/pci/pci.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "i82540"



#define RX_DESCRIPTOR_COUNT 512
#define TX_DESCRIPTOR_COUNT 512



static pmm_counter_descriptor_t* _i82540_driver_pmm_counter=NULL;
static omm_allocator_t* _i82540_device_allocator=NULL;



static KERNEL_INLINE void _consume_packet(i82540_device_t* device,u16 tail,i82540_rx_descriptor_t* desc){
	desc->status=0;
	if (device->mmio[REG_RDH]!=device->mmio[REG_RDT]){
		device->mmio[REG_RDT]=tail;
	}
}



static void _i82540_tx(void* extra_data,const void* buffer,u16 buffer_length){
	if (buffer_length>PAGE_SIZE){
		buffer_length=PAGE_SIZE;
	}
	i82540_device_t* device=extra_data;
	spinlock_acquire_exclusive(&(device->lock));
	u16 tail=device->mmio[REG_TDT];
	i82540_tx_descriptor_t* desc=I82540_DEVICE_GET_DESCRIPTOR(device,tx,tail);
	memcpy((void*)(desc->address+VMM_HIGHER_HALF_ADDRESS_OFFSET),buffer,buffer_length);
	desc->length=buffer_length;
	desc->cmd=TXDESC_EOP|TXDESC_RS;
	tail++;
	if (tail==TX_DESCRIPTOR_COUNT){
		tail=0;
	}
	device->mmio[REG_TDT]=tail;
	SPINLOOP(!(desc->status&0x0f));
}



static u16 _i82540_rx(void* extra_data,void* buffer,u16 buffer_length){
	i82540_device_t* device=extra_data;
	spinlock_acquire_exclusive(&(device->lock));
	u16 tail=device->mmio[REG_RDT];
	tail++;
	if (tail==RX_DESCRIPTOR_COUNT){
		tail=0;
	}
	i82540_rx_descriptor_t* desc=I82540_DEVICE_GET_DESCRIPTOR(device,rx,tail);
	if (!(desc->status&RDESC_DD)){
		spinlock_release_exclusive(&(device->lock));
		return 0;
	}
	if (desc->length<60||!(desc->status&RDESC_EOP)||desc->errors){
		_consume_packet(device,tail,desc);
		spinlock_release_exclusive(&(device->lock));
		return 0;
	}
	if (desc->length<buffer_length){
		buffer_length=desc->length;
	}
	memcpy(buffer,(void*)(desc->address+VMM_HIGHER_HALF_ADDRESS_OFFSET),buffer_length);
	_consume_packet(device,tail,desc);
	spinlock_release_exclusive(&(device->lock));
	return buffer_length;
}



static void _i82540_wait(void* extra_data){
	i82540_device_t* device=extra_data;
	spinlock_acquire_exclusive(&(device->lock));
	u16 tail=device->mmio[REG_RDT];
	tail++;
	if (tail==RX_DESCRIPTOR_COUNT){
		tail=0;
	}
	i82540_rx_descriptor_t* desc=I82540_DEVICE_GET_DESCRIPTOR(device,rx,tail);
	while (!(desc->status&RDESC_DD)){
		spinlock_release_exclusive(&(device->lock));
		thread_await_event(IRQ_EVENT(device->irq));
		spinlock_acquire_exclusive(&(device->lock));
		u32 icr=device->mmio[REG_ICR];
		if (icr&0x0004){
			device->mmio[REG_CTRL]|=CTRL_SLU;
		}
		else if (icr&0x0080){
			break;
		}
		(void)device->mmio[REG_ICR];
	}
	spinlock_release_exclusive(&(device->lock));
}



static const network_layer1_device_descriptor_t _network_layer1_device_descriptor={
	"i82540",
	_i82540_tx,
	_i82540_rx,
	_i82540_wait
};



static void _i82540_init_device(pci_device_t* device){
	if (device->class!=0x02||device->subclass!=0x00||device->device_id!=0x100e||device->vendor_id!=0x8086){
		return;
	}
	LOG("Attached i82540 driver to PCI device %x:%x:%x",device->address.bus,device->address.slot,device->address.func);
	pci_device_enable_bus_mastering(device);
	pci_device_enable_memory_access(device);
	pci_bar_t pci_bar;
	if (!pci_device_get_bar(device,0,&pci_bar)){
		return;
	}
	i82540_device_t* i82540_device=omm_alloc(_i82540_device_allocator);
	spinlock_init(&(i82540_device->lock));
	i82540_device->mmio=(void*)vmm_identity_map(pci_bar.address,(REG_MAX+1)*sizeof(u32));
	i82540_device->mmio[REG_IMC]=0xffffffff;
	i82540_device->mmio[REG_CTRL]=CTRL_RST;
	COUNTER_SPINLOOP(0xffff);
	i82540_device->mmio[REG_IMC]=0xffffffff;
	i82540_device->mmio[REG_CTRL]|=CTRL_FD;
	i82540_device->mmio[REG_GCR]|=0x00400000;
	i82540_device->mmio[REG_FCAH]=0;
	i82540_device->mmio[REG_FCAL]=0;
	i82540_device->mmio[REG_FCT]=0;
	i82540_device->mmio[REG_FCTTV]=0;
	i82540_device->mmio[REG_CTRL]|=CTRL_SLU;
	for (u64 i=0;!(i82540_device->mmio[REG_STATUS]&2);i++){
		if (i==0xffffff){
			WARN("Unable to establish ethernet link");
			return;
		}
	}
	u64 rx_desc_base=pmm_alloc(pmm_align_up_address(RX_DESCRIPTOR_COUNT*sizeof(i82540_rx_descriptor_t))>>PAGE_SIZE_SHIFT,_i82540_driver_pmm_counter,0);
	i82540_device->rx_desc_base=rx_desc_base+VMM_HIGHER_HALF_ADDRESS_OFFSET;
	for (u16 i=0;i<RX_DESCRIPTOR_COUNT;i++){
		i82540_rx_descriptor_t* desc=I82540_DEVICE_GET_DESCRIPTOR(i82540_device,rx,i);
		desc->address=pmm_alloc(1,_i82540_driver_pmm_counter,0);
		desc->status=0;
	}
	i82540_device->mmio[REG_RDBAH]=rx_desc_base>>32;
	i82540_device->mmio[REG_RDBAL]=rx_desc_base;
	i82540_device->mmio[REG_RDLEN]=RX_DESCRIPTOR_COUNT*sizeof(i82540_rx_descriptor_t);
	i82540_device->mmio[REG_RDH]=0;
	i82540_device->mmio[REG_RDT]=RX_DESCRIPTOR_COUNT-1;
	i82540_device->mmio[REG_RCTL]=RCTL_EN|RCTL_SBP|RCTL_UPE|RCTL_MPE|RCTL_LPE|RCTL_BAM|RCTL_BSIZE_4096|RCTL_PMCF|RCTL_SECRC;
	u64 tx_desc_base=pmm_alloc(pmm_align_up_address(TX_DESCRIPTOR_COUNT*sizeof(i82540_tx_descriptor_t))>>PAGE_SIZE_SHIFT,_i82540_driver_pmm_counter,0);
	i82540_device->tx_desc_base=tx_desc_base+VMM_HIGHER_HALF_ADDRESS_OFFSET;
	for (u16 i=0;i<TX_DESCRIPTOR_COUNT;i++){
		i82540_tx_descriptor_t* desc=I82540_DEVICE_GET_DESCRIPTOR(i82540_device,tx,i);
		desc->address=pmm_alloc(1,_i82540_driver_pmm_counter,0);
		desc->cmd=0;
		desc->status=0;
	}
	i82540_device->mmio[REG_TDBAH]=tx_desc_base>>32;
	i82540_device->mmio[REG_TDBAL]=tx_desc_base;
	i82540_device->mmio[REG_TDLEN]=TX_DESCRIPTOR_COUNT*sizeof(i82540_tx_descriptor_t);
	i82540_device->mmio[REG_TDH]=0;
	i82540_device->mmio[REG_TDT]=1;
	i82540_device->mmio[REG_TCTL]=TCTL_EN|TCTL_PSP;
	i82540_device->pci_irq=device->interrupt_line;
	i82540_device->irq=isr_allocate();
	ioapic_redirect_irq(i82540_device->pci_irq,i82540_device->irq);
	i82540_device->mmio[REG_ITR]=0x0000;
	i82540_device->mmio[REG_IMS]=0x0084;
	(void)i82540_device->mmio[REG_ICR];
	u32 rah=i82540_device->mmio[REG_RAH0];
	u32 ral=i82540_device->mmio[REG_RAL0];
	mac_address_t mac_address={
		ral,
		ral>>8,
		ral>>16,
		ral>>24,
		rah,
		rah>>8
	};
	network_layer1_create_device(&_network_layer1_device_descriptor,&mac_address,i82540_device);
}



void i82540_locate_devices(void){
	_i82540_driver_pmm_counter=pmm_alloc_counter("i82540");
	_i82540_device_allocator=omm_init("i82540_device",sizeof(i82540_device_t),8,1,pmm_alloc_counter("omm_i82540_device"));
	spinlock_init(&(_i82540_device_allocator->lock));
	HANDLE_FOREACH(pci_device_handle_type){
		pci_device_t* device=handle->object;
		_i82540_init_device(device);
	}
}
