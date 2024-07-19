#include <kernel/io/io.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/pci/pci.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "pci"



static const acpi_mcfg_t* KERNEL_EARLY_WRITE _pci_mcfg=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _pci_device_allocator=NULL;
static rwlock_t _pci_device_io_lock;

KERNEL_PUBLIC handle_type_t KERNEL_INIT_WRITE pci_device_handle_type=0;



static u64 KERNEL_INLINE KERNEL_EARLY_EXEC _get_io_offset(u64 base_offset,u8 bus,u8 slot,u8 func){
	if (base_offset){
		return vmm_identity_map(base_offset+((bus<<20)|(slot<<15)|(func<<12)),PAGE_SIZE);
	}
	else{
		return 0x80000000|(bus<<16)|(slot<<11)|(func<<8);
	}
}



static pci_device_t* KERNEL_EARLY_EXEC _load_device_from_offset(u64 base_offset,u16 segment_group,u8 bus,u8 slot,u8 func){
	pci_device_t tmp_device={
		._offset=_get_io_offset(base_offset,bus,slot,func)
	};
	u32 data[4]={pci_device_read_data(&tmp_device,0)};
	if (data[0]==0xffffffff){
		return NULL;
	}
	data[1]=pci_device_read_data(&tmp_device,4);
	data[2]=pci_device_read_data(&tmp_device,8);
	data[3]=pci_device_read_data(&tmp_device,12);
	if ((data[3]>>16)&0xff){ // PCI-to-XXX bridge
		return NULL;
	}
	pci_device_t* device=omm_alloc(_pci_device_allocator);
	handle_new(pci_device_handle_type,&(device->handle));
	device->_offset=tmp_device._offset;
	device->segment_group=segment_group;
	device->bus=bus;
	device->slot=slot;
	device->func=func;
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
	INFO("Found PCI device at [%X:%X:%X]: %X/%X/%X/%X/%X%X:%X%X",device->bus,device->slot,device->func,device->class,device->subclass,device->progif,device->revision_id,device->device_id>>8,device->device_id,device->vendor_id>>8,device->vendor_id);
	return device;
}



static void KERNEL_EARLY_EXEC _load_devices(u32 start_bus,u32 end_bus,u16 segment_group,u64 offset){
	for (u16 bus=start_bus;bus<=end_bus;bus++){
		pci_device_t tmp_device={
			._offset=_get_io_offset(offset,0,0,bus)
		};
		if (pci_device_read_data(&tmp_device,0)==0xffffffff){
			continue;
		}
		for (u16 slot=0;slot<256;slot++){
			for (u8 func=0;func<8;func++){
				pci_device_t* device=_load_device_from_offset(offset,segment_group,bus,slot,func);
				if (device&&!(device->header_type&0x80)){
					break;
				}
			}
		}
	}
}



static void KERNEL_EARLY_EXEC _load_devices_from_io(void){
	INFO("Loading PCI devices...");
	pci_device_t tmp_device={
		._offset=_get_io_offset(0,0,0,0)
	};
	_load_devices(0,((pci_device_read_data(&tmp_device,0x0c)&0x800000)?8:1),0,0);
}



static void KERNEL_EARLY_EXEC _load_devices_from_memory(void){
	INFO("Loading PCIe devices...");
	u32 entry_count=(_pci_mcfg->header.length-__builtin_offsetof(acpi_mcfg_t,entries))/sizeof(((acpi_mcfg_t*)NULL)->entries[0]);
	for (u32 i=0;i<entry_count;i++){
		_load_devices(_pci_mcfg->entries[i].start_pci_bus,_pci_mcfg->entries[i].end_pci_bus,_pci_mcfg->entries[i].pci_segment_group_number,_pci_mcfg->entries[i].base_address);
	}
}



KERNEL_ASYNC_INIT(){
	LOG("Scanning PCI devices...");
	pci_device_handle_type=handle_alloc("kernel.pci.device",0,NULL);
	_pci_device_allocator=omm_init("kernel.pci.device",sizeof(pci_device_t),8,1);
	rwlock_init(&(_pci_device_allocator->lock));
	rwlock_init(&_pci_device_io_lock);
	if (_pci_mcfg){
		_load_devices_from_memory();
	}
	else{
		_load_devices_from_io();
	}
}



void pci_set_pcie_table(const acpi_mcfg_t* mcfg){
	_pci_mcfg=mcfg;
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
		mask=0xfffffff0;
	}
	out->address=(((u64)bar_high)<<32)|(bar&mask);
	pci_device_write_data(device,register_index,0xffffffff);
	out->size=-((((u64)size_high)<<32)|(pci_device_read_data(device,register_index)&mask));
	pci_device_write_data(device,register_index,bar);
	out->flags=flags;
	return 1;
}



KERNEL_PUBLIC u32 pci_device_get_cap(const pci_device_t* device,u8 cap,u32 offset){
	if (!(pci_device_read_data(device,4)&0x100000)){
		return 0;
	}
	offset=(offset?pci_device_read_data(device,offset)>>8:pci_device_read_data(device,0x34));
	while (offset){
		u32 header=pci_device_read_data(device,offset);
		if ((header&0xff)==cap){
			return offset;
		}
		offset=(header>>8)&0xff;
	}
	return 0;
}



KERNEL_PUBLIC u8 KERNEL_NOCOVERAGE pci_device_read_config8(u64 offset){
	if (offset>>48){
		return *((const volatile u8*)(void*)offset);
	}
	rwlock_acquire_write(&_pci_device_io_lock);
	io_port_out32(0xcf8,offset);
	u8 out=io_port_in8(0xcfc);
	rwlock_release_write(&_pci_device_io_lock);
	return out;
}



KERNEL_PUBLIC u16 KERNEL_NOCOVERAGE pci_device_read_config16(u64 offset){
	if (offset>>48){
		return *((const volatile u16*)(void*)offset);
	}
	rwlock_acquire_write(&_pci_device_io_lock);
	io_port_out32(0xcf8,offset);
	u16 out=io_port_in16(0xcfc);
	rwlock_release_write(&_pci_device_io_lock);
	return out;
}



KERNEL_PUBLIC u32 KERNEL_NOCOVERAGE pci_device_read_config32(u64 offset){
	if (offset>>48){
		return *((const volatile u32*)(void*)offset);
	}
	rwlock_acquire_write(&_pci_device_io_lock);
	io_port_out32(0xcf8,offset);
	u32 out=io_port_in32(0xcfc);
	rwlock_release_write(&_pci_device_io_lock);
	return out;
}



KERNEL_PUBLIC void KERNEL_NOCOVERAGE pci_device_write_config8(u64 offset,u8 value){
	if (offset>>48){
		*((volatile u8*)(void*)offset)=value;
		return;
	}
	rwlock_acquire_write(&_pci_device_io_lock);
	io_port_out32(0xcf8,offset);
	io_port_out8(0xcfc,value);
	rwlock_release_write(&_pci_device_io_lock);
}



KERNEL_PUBLIC void KERNEL_NOCOVERAGE pci_device_write_config16(u64 offset,u16 value){
	if (offset>>48){
		*((volatile u16*)(void*)offset)=value;
		return;
	}
	rwlock_acquire_write(&_pci_device_io_lock);
	io_port_out32(0xcf8,offset);
	io_port_out16(0xcfc,value);
	rwlock_release_write(&_pci_device_io_lock);
}



KERNEL_PUBLIC void KERNEL_NOCOVERAGE pci_device_write_config32(u64 offset,u32 value){
	if (offset>>48){
		*((volatile u32*)(void*)offset)=value;
		return;
	}
	rwlock_acquire_write(&_pci_device_io_lock);
	io_port_out32(0xcf8,offset);
	io_port_out32(0xcfc,value);
	rwlock_release_write(&_pci_device_io_lock);
}



KERNEL_PUBLIC void KERNEL_NOCOVERAGE pci_device_enable_io_access(const pci_device_t* device){
	pci_device_write_data(device,4,pci_device_read_data(device,4)|1);
}



KERNEL_PUBLIC void KERNEL_NOCOVERAGE pci_device_enable_memory_access(const pci_device_t* device){
	pci_device_write_data(device,4,pci_device_read_data(device,4)|2);
}



KERNEL_PUBLIC void KERNEL_NOCOVERAGE pci_device_enable_bus_mastering(const pci_device_t* device){
	pci_device_write_data(device,4,pci_device_read_data(device,4)|4);
}
