#include <kernel/log/log.h>
#include <kernel/memory/vmm.h>
#include <kernel/msr/msr.h>
#include <kernel/pci/msix.h>
#include <kernel/pci/pci.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "pci_msix"



static u32 KERNEL_INIT_WRITE _pci_msix_address;


KERNEL_INIT(){
	LOG("Initializing MSI-x...");
	_pci_msix_address=0xfee00000|(msr_get_apic_id()<<12);
}



KERNEL_PUBLIC _Bool pci_msix_load(const pci_device_t* device,msix_table_t* out){
	u8 offset=pci_device_get_cap(device,PCI_CAP_ID_MSIX,0);
	if (!offset){
		return 0;
	}
	u32 table_offset_and_bar=pci_device_read_data(device,offset+4);
	pci_bar_t pci_bar;
	if ((table_offset_and_bar&7)>5||!pci_device_get_bar(device,table_offset_and_bar&7,&pci_bar)||!(pci_bar.flags&PCI_BAR_FLAG_MEMORY)){
		WARN("Invalid MSI-x config");
		return 0;
	}
	u32 message_control=pci_device_read_data(device,offset);
	pci_device_write_data(device,offset,message_control|0x80000000);
	out->data=(void*)(vmm_identity_map(pci_bar.address,pci_bar.size)+(table_offset_and_bar&0xfffffff8));
	out->length=((message_control>>16)&0x7ff)+1;
	return 1;
}



KERNEL_PUBLIC _Bool pci_msix_redirect_entry(const msix_table_t* table,u16 vector,u8 irq){
	if (vector>=table->length||!((table->data+vector)->control&MSIX_TABLE_ENTRY_CONTROL_FLAG_MASKED)){
		return 0;
	}
	(table->data+vector)->msg_address=_pci_msix_address;
	(table->data+vector)->msg_data=irq|0x00004000;
	(table->data+vector)->control&=~MSIX_TABLE_ENTRY_CONTROL_FLAG_MASKED;
	return 1;
}
