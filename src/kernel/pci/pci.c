#include <kernel/driver/ahci.h>
#include <kernel/driver/ata.h>
#include <kernel/driver/i82540.h>
#include <kernel/driver/nvme.h>
#include <kernel/io/io.h>
#include <kernel/log/log.h>
#include <kernel/pci/pci.h>
#include <kernel/types.h>



void pci_init(void){
	LOG("Scanning PCI devices...");
	pci_device_t device={
		0,
		0,
		0
	};
	u8 max_bus_index=((pci_device_read_data(&device,0xc)&0x800000)?8:1);
	for (u8 bus=0;bus<max_bus_index;bus++){
		device.bus=0;
		device.slot=0;
		device.func=bus;
		if (pci_device_read_data(&device,0)==0xffffffff){
			continue;
		}
		device.bus=bus;
		for (u16 slot=0;slot<256;slot++){
			device.slot=slot;
			for (u8 func=0;func<8;func++){
				device.func=func;
				u32 data[4]={pci_device_read_data(&device,0)};
				if (data[0]==0xffffffff){
					continue;
				}
				data[1]=pci_device_read_data(&device,4);
				data[2]=pci_device_read_data(&device,8);
				data[3]=pci_device_read_data(&device,12);
				if ((data[3]>>16)&0xff){ // PCI-to-??? bridge
					continue;
				}
				device.device_id=data[0]>>16;
				device.vendor_id=data[0];
				device.class=data[2]>>24;
				device.subclass=data[2]>>16;
				device.progif=data[2]>>8;
				device.revision_id=data[2];
				device.header_type=data[3]>>16;
				INFO("Found PCI device at [%x:%x:%x]: %u/%u/%u/%u/%x:%x",device.bus,device.slot,device.func,device.class,device.subclass,device.progif,device.revision_id,device.device_id,device.vendor_id);
				driver_ahci_init_device(&device);
				driver_ata_init_device(&device);
				driver_i82540_init_device(&device);
				driver_nvme_init_device(&device);
			}
		}
	}
}



_Bool pci_device_get_bar(const pci_device_t* device,u8 bar_index,pci_bar_t* out){
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
