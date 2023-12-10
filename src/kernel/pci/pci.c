#include <kernel/io/io.h>
#include <kernel/log/log.h>
#include <kernel/pci/pci.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/omm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "pci"



static omm_allocator_t* KERNEL_INIT_WRITE _pci_device_allocator=NULL;

KERNEL_PUBLIC handle_type_t KERNEL_INIT_WRITE pci_device_handle_type=0;



KERNEL_INIT(){
	LOG("Scanning PCI devices...");
	pci_device_handle_type=handle_alloc("pci_device",NULL);
	_pci_device_allocator=omm_init("pci_device",sizeof(pci_device_t),8,1,pmm_alloc_counter("omm_pci_device"));
	spinlock_init(&(_pci_device_allocator->lock));
	pci_device_address_t device_address={
		0,
		0,
		0
	};
	u8 max_bus_index=((pci_device_read_data_raw(&device_address,0xc)&0x800000)?8:1);
	for (u8 bus=0;bus<max_bus_index;bus++){
		device_address.bus=0;
		device_address.slot=0;
		device_address.func=bus;
		if (pci_device_read_data_raw(&device_address,0)==0xffffffff){
			continue;
		}
		device_address.bus=bus;
		for (u16 slot=0;slot<256;slot++){
			device_address.slot=slot;
			for (u8 func=0;func<8;func++){
				device_address.func=func;
				u32 data[4]={pci_device_read_data_raw(&device_address,0)};
				if (data[0]==0xffffffff){
					continue;
				}
				data[1]=pci_device_read_data_raw(&device_address,4);
				data[2]=pci_device_read_data_raw(&device_address,8);
				data[3]=pci_device_read_data_raw(&device_address,12);
				if ((data[3]>>16)&0xff){ // PCI-to-XXX bridge
					continue;
				}
				pci_device_t* device=omm_alloc(_pci_device_allocator);
				handle_new(device,pci_device_handle_type,&(device->handle));
				device->address=device_address;
				device->device_id=data[0]>>16;
				device->vendor_id=data[0];
				device->class=data[2]>>24;
				device->subclass=data[2]>>16;
				device->progif=data[2]>>8;
				device->revision_id=data[2];
				device->header_type=data[3]>>16;
				device->interrupt_line=pci_device_read_data(device,60);
				device->interrupt_state.state=PCI_INTERRUPT_STATE_NONE;
				if (data[1]&0x100000){
					u8 offset=pci_device_read_data(device,52);
					while (offset){
						u32 cap=pci_device_read_data(device,offset);
						if ((cap&0xff)==5){
							device->interrupt_state.state=PCI_INTERRUPT_STATE_MSI;
							device->interrupt_state.msi.offset=offset;
							break;
						}
						if ((cap&0xff)==17){
							device->interrupt_state.state=PCI_INTERRUPT_STATE_MSIX;
							device->interrupt_state.msix.offset=offset;
							device->interrupt_state.msix.next_table_index=0;
							device->interrupt_state.msix.table_size=(cap>>16)&0x1ff;
							break;
						}
						offset=(cap>>8);
					}
				}
				handle_finish_setup(&(device->handle));
				INFO("Found PCI device at [%X:%X:%X]: %X/%X/%X/%X/%X%X:%X%X",device->address.bus,device->address.slot,device->address.func,device->class,device->subclass,device->progif,device->revision_id,device->device_id>>8,device->device_id,device->vendor_id>>8,device->vendor_id);
			}
		}
	}
}



KERNEL_PUBLIC _Bool pci_device_get_bar(const pci_device_t* device,u8 bar_index,pci_bar_t* out){
	u8 register_index=(bar_index<<2)+16;
	u32 bar=pci_device_read_data(device,register_index);
	if (!bar){
		return 0;
	}
	u32 bar_high=0;
	u32 size_high=0;
	u8 flags=0;
	u32 mask=0xfffffffc;
	if (!(bar&1)){
		flags|=PCI_BAR_FLAG_MEMORY;
		if (bar&6){
			bar_high=pci_device_read_data(device,register_index+4);
			pci_device_write_data(device,register_index+4,0xffffffff);
			size_high=pci_device_read_data(device,register_index+4);
			pci_device_write_data(device,register_index+4,bar_high);
		}
		if (bar&8){
			WARN("Prefeachable PCI BAR is unimplemented!");
			return 0;
		}
		mask=0xfffffff0;
	}
	out->address=(((u64)bar_high)<<32)|(bar&mask);
	pci_device_write_data(device,register_index,0xffffffff);
	out->size=-((((u64)size_high)<<32)|(pci_device_read_data(device,register_index)&mask));
	pci_device_write_data(device,register_index,bar);
	out->flags=flags;
	return 1;
}
