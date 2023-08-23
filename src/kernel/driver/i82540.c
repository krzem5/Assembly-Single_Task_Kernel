#include <kernel/apic/ioapic.h>
#include <kernel/driver/i82540.h>
#include <kernel/isr/isr.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/network/layer1.h>
#include <kernel/pci/pci.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "i82540"



#define GET_DESCRIPTOR(device,type,index) ((void*)((device)->type##_desc_base+((index)<<4)))



// Registers
#define REG_CTRL 0x0000
#define REG_STATUS 0x0002
#define REG_FCAL 0x000a
#define REG_FCAH 0x000b
#define REG_FCT 0x000c
#define REG_ICR 0x0030
#define REG_ITR 0x0031
#define REG_IMS 0x0034
#define REG_IMC 0x0036
#define REG_RCTL 0x0040
#define REG_FCTTV 0x005c
#define REG_TCTL 0x0100
#define REG_RDBAL 0x0a00
#define REG_RDBAH 0x0a01
#define REG_RDLEN 0x0a02
#define REG_RDH 0x0a04
#define REG_RDT 0x0a06
#define REG_TDBAL 0x0e00
#define REG_TDBAH 0x0e01
#define REG_TDLEN 0x0e02
#define REG_TDH 0x0e04
#define REG_TDT 0x0e06
#define REG_RAL0 0x1500
#define REG_RAH0 0x1501
#define REG_GCR 0x16c0

#define REG_MAX REG_GCR

// CTRL flags
#define CTRL_FD 0x00000001
#define CTRL_SLU 0x00000040
#define CTRL_RST 0x04000000

// RCTL flags
#define RCTL_EN 0x00000002
#define RCTL_SBP 0x00000004
#define RCTL_UPE 0x00000008
#define RCTL_MPE 0x00000010
#define RCTL_LPE 0x00000020
#define RCTL_BAM 0x00008000
#define RCTL_BSIZE_4096 0x02030000
#define RCTL_PMCF 0x00800000
#define RCTL_SECRC 0x04000000

// TCTL flags
#define TCTL_EN 0x02
#define TCTL_PSP 0x08

// RDESC status flafs
#define RDESC_DD 0x01
#define RDESC_EOP 0x02

// TXDESC command flags
#define TXDESC_EOP 0x01
#define TXDESC_RS 0x08

// TXDESC status flags
#define TXDESC_DD 0x01



static inline void _consume_packet(i82540_device_t* device,u16 tail,i82540_rx_descriptor_t* desc){
	desc->status=0;
	if (device->mmio[REG_RDH]!=device->mmio[REG_RDT]){
		device->mmio[REG_RDT]=tail;
	}
}



static void _i82540_tx(void* extra_data,u64 packet,u16 length){
	i82540_device_t* device=extra_data;
	u16 tail=device->mmio[REG_TDT];
	i82540_tx_descriptor_t* desc=GET_DESCRIPTOR(device,tx,tail);
	desc->address=packet;
	desc->length=length;
	desc->cmd=TXDESC_EOP|TXDESC_RS;
	tail++;
	if (tail==NUM_TX_DESCRIPTORS){
		tail=0;
	}
	device->mmio[REG_TDT]=tail;
	while (!(desc->status&0x0f)){
		__pause();
	}
}



static u16 _i82540_rx(void* extra_data,void* buffer,u16 buffer_length){
	i82540_device_t* device=extra_data;
	u16 tail=device->mmio[REG_RDT];
	tail++;
	if (tail==NUM_RX_DESCRIPTORS){
		tail=0;
	}
	i82540_rx_descriptor_t* desc=GET_DESCRIPTOR(device,rx,tail);
	if (!(desc->status&RDESC_DD)){
		return 0;
	}
	if (desc->length<60||!(desc->status&RDESC_EOP)||desc->errors){
		_consume_packet(device,tail,desc);
		return 0;
	}
	if (desc->length<buffer_length){
		buffer_length=desc->length;
	}
	memcpy(buffer,(void*)(desc->address),buffer_length);
	_consume_packet(device,tail,desc);
	return buffer_length;
}



static void _i82540_wait(void* extra_data){
	i82540_device_t* device=extra_data;
	u16 tail=device->mmio[REG_RDT];
	tail++;
	if (tail==NUM_RX_DESCRIPTORS){
		tail=0;
	}
	i82540_rx_descriptor_t* desc=GET_DESCRIPTOR(device,rx,tail);
	while (!(desc->status&RDESC_DD)){
		if (!isr_wait(device->irq)){
			break;
		}
		u32 icr=device->mmio[REG_ICR];
		if (icr&0x0004){
			device->mmio[REG_CTRL]|=CTRL_SLU;
		}
		else if (icr&0x0080){
			break;
		}
		(void)device->mmio[REG_ICR];
	}
}



static void _i82540_irq_init(void* extra_data){
	i82540_device_t* device=extra_data;
	device->irq=isr_allocate();
	ioapic_redirect_irq(device->pci_irq,device->irq);
	device->mmio[REG_ITR]=0x0000;
	device->mmio[REG_IMS]=0x0084;
	(void)device->mmio[REG_ICR];
}



void driver_i82540_init_device(pci_device_t* device){
	if (device->class!=0x02||device->subclass!=0x00||device->device_id!=0x100e||device->vendor_id!=0x8086){
		return;
	}
	LOG_CORE("Attached i82540 driver to PCI device %x:%x:%x",device->bus,device->slot,device->func);
	pci_device_enable_bus_mastering(device);
	pci_device_enable_memory_access(device);
	pci_bar_t pci_bar;
	if (!pci_device_get_bar(device,0,&pci_bar)){
		return;
	}
	vmm_identity_map(pci_bar.address,(REG_MAX+1)*sizeof(u32));
	i82540_device_t* i82540_device=kmm_allocate(sizeof(i82540_device_t));
	i82540_device->mmio=pci_bar.address;
	i82540_device->mmio[REG_IMC]=0xffffffff;
	i82540_device->mmio[REG_CTRL]=CTRL_RST;
	for (u64 i=0;i<0xfffff;i++){
		__pause();
	}
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
			WARN_CORE("Unable to establish ethernet link");
			return;
		}
	}
	i82540_device->rx_desc_base=pmm_alloc(pmm_align_up_address(NUM_RX_DESCRIPTORS*sizeof(i82540_rx_descriptor_t))>>PAGE_SIZE_SHIFT,PMM_COUNTER_DRIVER_I82540);
	for (u16 i=0;i<NUM_RX_DESCRIPTORS;i++){
		i82540_rx_descriptor_t* desc=GET_DESCRIPTOR(i82540_device,rx,i);
		desc->address=pmm_alloc(1,PMM_COUNTER_DRIVER_I82540);
		desc->status=0;
	}
	i82540_device->mmio[REG_RDBAH]=i82540_device->rx_desc_base>>32;
	i82540_device->mmio[REG_RDBAL]=i82540_device->rx_desc_base;
	i82540_device->mmio[REG_RDLEN]=NUM_RX_DESCRIPTORS*sizeof(i82540_rx_descriptor_t);
	i82540_device->mmio[REG_RDH]=0;
	i82540_device->mmio[REG_RDT]=NUM_RX_DESCRIPTORS-1;
	i82540_device->mmio[REG_RCTL]=RCTL_EN|RCTL_SBP|RCTL_UPE|RCTL_MPE|RCTL_LPE|RCTL_BAM|RCTL_BSIZE_4096|RCTL_PMCF|RCTL_SECRC;
	i82540_device->tx_desc_base=pmm_alloc(pmm_align_up_address(NUM_TX_DESCRIPTORS*sizeof(i82540_tx_descriptor_t))>>PAGE_SIZE_SHIFT,PMM_COUNTER_DRIVER_I82540);
	for (u16 i=0;i<NUM_TX_DESCRIPTORS;i++){
		i82540_tx_descriptor_t* desc=GET_DESCRIPTOR(i82540_device,tx,i);
		desc->address=0;
		desc->cmd=0;
		desc->status=0;
	}
	i82540_device->mmio[REG_TDBAH]=i82540_device->tx_desc_base>>32;
	i82540_device->mmio[REG_TDBAL]=i82540_device->tx_desc_base;
	i82540_device->mmio[REG_TDLEN]=NUM_TX_DESCRIPTORS*sizeof(i82540_tx_descriptor_t);
	i82540_device->mmio[REG_TDH]=0;
	i82540_device->mmio[REG_TDT]=1;
	i82540_device->mmio[REG_TCTL]=TCTL_EN|TCTL_PSP;
	i82540_device->pci_irq=device->interrupt_line;
	i82540_device->irq=0;
	u32 rah=i82540_device->mmio[REG_RAH0];
	u32 ral=i82540_device->mmio[REG_RAL0];
	network_layer1_device_t layer1_device={
		"i82540",
		{
			ral,
			ral>>8,
			ral>>16,
			ral>>24,
			rah,
			rah>>8
		},
		_i82540_tx,
		_i82540_rx,
		_i82540_wait,
		_i82540_irq_init,
		i82540_device
	};
	network_layer1_set_device(&layer1_device);
}
