#include <kernel/io/io.h>
#include <kernel/log/log.h>
#include <kernel/pci/pci.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/omm.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "pci"



static omm_allocator_t* KERNEL_INIT_WRITE _pci_device_allocator=NULL;

KERNEL_PUBLIC handle_type_t KERNEL_INIT_WRITE pci_device_handle_type=0;



KERNEL_INIT(){
	LOG("Scanning PCI devices...");
	pci_device_handle_type=handle_alloc("kernel.pci.device",NULL);
	_pci_device_allocator=omm_init("kernel.pci.device",sizeof(pci_device_t),8,1);
	rwlock_init(&(_pci_device_allocator->lock));
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
				handle_new(pci_device_handle_type,&(device->handle));
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
				u8 msi_offset=pci_device_get_cap(device,PCI_CAP_ID_MSI,0);
				if (msi_offset){
					device->interrupt_state.state=PCI_INTERRUPT_STATE_MSI;
					device->interrupt_state.msi.offset=msi_offset;
					goto _skip_msix_discovery;
				}
				u8 msix_offset=pci_device_get_cap(device,PCI_CAP_ID_MSIX,0);
				if (msix_offset){
					device->interrupt_state.state=PCI_INTERRUPT_STATE_MSIX;
					device->interrupt_state.msi.offset=msix_offset;
					device->interrupt_state.msix.next_table_index=0;
					device->interrupt_state.msix.table_size=(pci_device_read_data(device,msix_offset)>>16)&0x1ff;
				}
_skip_msix_discovery:
				INFO("Found PCI device at [%X:%X:%X]: %X/%X/%X/%X/%X%X:%X%X",device->address.bus,device->address.slot,device->address.func,device->class,device->subclass,device->progif,device->revision_id,device->device_id>>8,device->device_id,device->vendor_id>>8,device->vendor_id);
			}
		}
	}
}



KERNEL_PUBLIC bool pci_device_get_bar(const pci_device_t* device,u8 bar_index,pci_bar_t* out){
	u8 register_index=(bar_index<<2)+16;
	u32 bar=pci_device_read_data(device,register_index);
	if (!bar){
		return 0;
	}
	u32 bar_high=0;
	u32 size_high=0xffffffff;
	u8 flags=0;
	u32 mask=0xfffffffc;
	if (!(bar&1)){
		flags|=PCI_BAR_FLAG_MEMORY;
		if ((bar&6)==4){
			bar_high=pci_device_read_data(device,register_index+4);
			pci_device_write_data(device,register_index+4,0xffffffff);
			size_high=pci_device_read_data(device,register_index+4);
			pci_device_write_data(device,register_index+4,bar_high);
		}
		if (bar&8){
			WARN("Prefeachable PCI BAR is unimplemented!");
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



KERNEL_PUBLIC u8 pci_device_get_cap(const pci_device_t* device,u8 cap,u8 offset){
	if (!(pci_device_read_data(device,4)&0x100000)){
		return 0;
	}
	offset=(offset?pci_device_read_data(device,offset)>>8:pci_device_read_data(device,0x34));
	while (offset){
		u32 header=pci_device_read_data(device,offset);
		if ((header&0xff)==cap){
			return offset;
		}
		offset=header>>8;
	}
	return 0;
}
