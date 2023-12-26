#include <kernel/apic/ioapic.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/pci/pci.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <virtio/registers.h>
#include <virtio/virtio.h>
#define KERNEL_LOG_NAME "virtio"



#define MAX_QUEUE_SIZE 256



static u16 _virtio_device_next_index=0;
static omm_allocator_t* _virtio_device_allocator=NULL;
static omm_allocator_t* _virtio_device_driver_node_allocator=NULL;
static rb_tree_t _virtio_device_driver_tree;
static spinlock_t _virtio_device_driver_tree_lock;
static omm_allocator_t* _virtio_queue_allocator=NULL;
static pmm_counter_descriptor_t* _virtio_queue_pmm_counter=NULL;

static handle_type_t _virtio_device_handle_type=0;



static void _virtio_init_device(pci_device_t* device){
	if (device->device_id<0x1000||device->device_id>0x107f||device->vendor_id!=0x1af4){
		return;
	}
	LOG("Attached root virtio driver to PCI device %x:%x:%x",device->address.bus,device->address.slot,device->address.func);
	pci_device_enable_bus_mastering(device);
	pci_device_enable_memory_access(device);
	virtio_device_t* virtio_device=omm_alloc(_virtio_device_allocator);
	u32 found_structures=0;
	for (u8 offset=pci_device_get_cap(device,PCI_CAP_ID_VENDOR,0);offset;offset=pci_device_get_cap(device,PCI_CAP_ID_VENDOR,offset)){
		u8 type=pci_device_read_data(device,offset)>>24;
		u8 bar=pci_device_read_data(device,offset+4)&0xff;
		if (type<VIRTIO_PCI_CAP_MIN||type>VIRTIO_PCI_CAP_MAX||bar>5){
			continue;
		}
		pci_bar_t pci_bar;
		if (!pci_device_get_bar(device,bar,&pci_bar)){
			continue;
		}
		found_structures|=1<<type;
		virtio_field_t field=((pci_bar.flags&PCI_BAR_FLAG_MEMORY)?vmm_identity_map(pci_bar.address,pci_bar.size):pci_bar.address)+pci_device_read_data(device,offset+8);
		switch (type){
			case VIRTIO_PCI_CAP_COMMON_CFG:
				virtio_device->common_field=field;
				break;
			case VIRTIO_PCI_CAP_NOTIFY_CFG:
				virtio_device->notify_field=field;
				virtio_device->notify_off_multiplier=pci_device_read_data(device,offset+16);
				break;
			case VIRTIO_PCI_CAP_ISR_CFG:
				virtio_device->isr_field=field;
				break;
			case VIRTIO_PCI_CAP_DEVICE_CFG:
				virtio_device->device_field=field;
				break;
		}
		INFO("VirtIO PCI structure: type=%u, address=%p, length=%u",type,field,pci_device_read_data(device,offset+12));
	}
	if (found_structures!=(((1<<(VIRTIO_PCI_CAP_MAX-VIRTIO_PCI_CAP_MIN+1))-1)<<VIRTIO_PCI_CAP_MIN)){
		ERROR("Legacy VirtIO device found; unimplemented");
		omm_dealloc(_virtio_device_allocator,virtio_device);
		return;
	}
	handle_new(virtio_device,_virtio_device_handle_type,&(virtio_device->handle));
	virtio_device->type=device->device_id-(device->device_id>=0x1040?0x1040:0x1000);
	virtio_device->index=_virtio_device_next_index;
	_virtio_device_next_index++;
	spinlock_init(&(virtio_device->lock));
	virtio_write(virtio_device->common_field+VIRTIO_REG_DEVICE_STATUS,1,0x00);
	handle_finish_setup(&(virtio_device->handle));
}



KERNEL_PUBLIC _Bool virtio_register_device_driver(const virtio_device_driver_t* driver){
	spinlock_acquire_exclusive(&_virtio_device_driver_tree_lock);
	LOG("Registering VirtIO device driver '%s/%X%X'...",driver->name,driver->type>>8,driver->type);
	virtio_device_driver_node_t* node=(virtio_device_driver_node_t*)rb_tree_lookup_node(&_virtio_device_driver_tree,driver->type);
	if (node){
		ERROR("VirtIO device type %X%X is already allocated by '%s'",driver->type>>8,driver->type,node->driver->name);
		spinlock_release_exclusive(&_virtio_device_driver_tree_lock);
		return 0;
	}
	node=omm_alloc(_virtio_device_driver_node_allocator);
	node->rb_node.key=driver->type;
	node->driver=driver;
	rb_tree_insert_node(&_virtio_device_driver_tree,&(node->rb_node));
	spinlock_release_exclusive(&_virtio_device_driver_tree_lock);
	HANDLE_FOREACH(_virtio_device_handle_type){
		virtio_device_t* device=handle->object;
		if (device->type!=driver->type){
			continue;
		}
		virtio_write(device->common_field+VIRTIO_REG_DEVICE_STATUS,1,VIRTIO_DEVICE_STATUS_FLAG_ACKNOWLEDGE);
		virtio_write(device->common_field+VIRTIO_REG_DEVICE_STATUS,1,VIRTIO_DEVICE_STATUS_FLAG_ACKNOWLEDGE|VIRTIO_DEVICE_STATUS_FLAG_DRIVER);
		virtio_write(device->common_field+VIRTIO_REG_DEVICE_FEATURE_SELECT,4,0);
		u64 features=virtio_read(device->common_field+VIRTIO_REG_DEVICE_FEATURE,4);
		virtio_write(device->common_field+VIRTIO_REG_DEVICE_FEATURE_SELECT,4,1);
		features|=virtio_read(device->common_field+VIRTIO_REG_DEVICE_FEATURE,4)<<32;
		features&=driver->features;
		virtio_write(device->common_field+VIRTIO_REG_DRIVER_FEATURE_SELECT,4,0);
		virtio_write(device->common_field+VIRTIO_REG_DRIVER_FEATURE,4,features);
		virtio_write(device->common_field+VIRTIO_REG_DRIVER_FEATURE_SELECT,4,1);
		virtio_write(device->common_field+VIRTIO_REG_DRIVER_FEATURE,4,features>>32);
		virtio_write(device->common_field+VIRTIO_REG_DEVICE_STATUS,1,VIRTIO_DEVICE_STATUS_FLAG_ACKNOWLEDGE|VIRTIO_DEVICE_STATUS_FLAG_DRIVER|VIRTIO_DEVICE_STATUS_FLAG_FEATURES_OK);
		if (!(virtio_read(device->common_field+VIRTIO_REG_DEVICE_STATUS,1)&VIRTIO_DEVICE_STATUS_FLAG_FEATURES_OK)){
			ERROR("Failed to negotiate VirtIO device features");
			virtio_write(device->common_field+VIRTIO_REG_DEVICE_STATUS,1,VIRTIO_DEVICE_STATUS_FLAG_FAILED);
			continue;
		}
		INFO("Negotiated device features: %p",features);
		if (!driver->init(device,features)){
			virtio_write(device->common_field+VIRTIO_REG_DEVICE_STATUS,1,VIRTIO_DEVICE_STATUS_FLAG_FAILED);
		}
	}
	return 1;
}



KERNEL_PUBLIC _Bool virtio_unregister_device_driver(const virtio_device_driver_t* driver){
	spinlock_acquire_exclusive(&_virtio_device_driver_tree_lock);
	LOG("Unregistering VirtIO device driver '%s/%X%X'...",driver->name,driver->type>>8,driver->type);
	rb_tree_node_t* node=rb_tree_lookup_node(&_virtio_device_driver_tree,driver->type);
	_Bool out=!!node;
	if (node){
		rb_tree_remove_node(&_virtio_device_driver_tree,node);
		omm_dealloc(_virtio_device_driver_node_allocator,node);
	}
	spinlock_release_exclusive(&_virtio_device_driver_tree_lock);
	return out;
}



KERNEL_PUBLIC u64 virtio_read(virtio_field_t field,u8 size){
	if (field>>63){
		switch (size){
			case 1:
				return *((const u8*)field);
			case 2:
				return *((const u16*)field);
			case 4:
				return *((const u32*)field);
			case 8:
				return *((const u64*)field);
		}
	}
	else{
		switch (size){
			case 1:
				return io_port_in8(field);
			case 2:
				return io_port_in16(field);
			case 4:
				return io_port_in32(field);
			case 8:
				return io_port_in32(field)|(((u64)io_port_in32(field+4))<<32);
		}
	}
	panic("virtio_read: invalid size");
}



KERNEL_PUBLIC void virtio_write(virtio_field_t field,u8 size,u32 value){
	if (field>>63){
		switch (size){
			case 1:
				*((u8*)field)=value;
				return;
			case 2:
				*((u16*)field)=value;
				return;
			case 4:
				*((u32*)field)=value;
				return;
		}
	}
	else{
		switch (size){
			case 1:
				io_port_out8(field,value);
				return;
			case 2:
				io_port_out16(field,value);
				return;
			case 4:
				io_port_out32(field,value);
				return;
		}
	}
	panic("virtio_write: invalid size");
}



KERNEL_PUBLIC virtio_queue_t* virtio_init_queue(const virtio_device_t* device,u16 index){
	virtio_write(device->common_field+VIRTIO_REG_QUEUE_SELECT,2,index);
	u16 size=virtio_read(device->common_field+VIRTIO_REG_QUEUE_SIZE,2);
	if (size>MAX_QUEUE_SIZE){
		virtio_write(device->common_field+VIRTIO_REG_QUEUE_SIZE,2,MAX_QUEUE_SIZE);
		size=virtio_read(device->common_field+VIRTIO_REG_QUEUE_SIZE,2);
	}
	if (!size){
		ERROR("virtio_init_queue: empty queue");
		return NULL;
	}
	if (size>MAX_QUEUE_SIZE){
		ERROR("virtio_init_queue: unable to set queue size");
		return NULL;
	}
	if (virtio_read(device->common_field+VIRTIO_REG_QUEUE_ENABLE,2)){
		ERROR("virtio_init_queue: queue already initialized");
		return NULL;
	}
	u64 queue_descriptors=pmm_alloc((pmm_align_up_address(size*sizeof(virtio_queue_descriptor_t)+sizeof(virtio_queue_available_t)+size*sizeof(u16))+pmm_align_up_address(sizeof(virtio_queue_used_t)+size*sizeof(virtio_queue_used_entry_t)))>>PAGE_SIZE_SHIFT,_virtio_queue_pmm_counter,0);
	u64 queue_available=queue_descriptors+size*sizeof(virtio_queue_descriptor_t);
	u64 queue_used=queue_available+sizeof(virtio_queue_available_t)+size*sizeof(u16);
	virtio_queue_t* out=omm_alloc(_virtio_queue_allocator);
	out->device=device;
	out->index=index;
	out->size=size;
	out->notify_offset=virtio_read(device->common_field+VIRTIO_REG_QUEUE_NOTIFY_OFF,2);
	out->first_free_index=0;
	out->last_used_index=0;
	out->descriptors=(void*)(queue_descriptors+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	out->available=(void*)(queue_available+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	out->used=(void*)(queue_used+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	for (u16 i=0;i<size-1;i++){ // last entry is zero-initialized by pmm_alloc
		(out->descriptors+i)->next=i+1;
	}
	out->available->flags=VIRTQ_AVAIL_F_NO_INTERRUPT;
	virtio_write(device->common_field+VIRTIO_REG_QUEUE_DESC_LO,4,queue_descriptors);
	virtio_write(device->common_field+VIRTIO_REG_QUEUE_DESC_HI,4,queue_descriptors>>32);
	virtio_write(device->common_field+VIRTIO_REG_QUEUE_DRIVER_LO,4,queue_available);
	virtio_write(device->common_field+VIRTIO_REG_QUEUE_DRIVER_HI,4,queue_available>>32);
	virtio_write(device->common_field+VIRTIO_REG_QUEUE_DEVICE_LO,4,queue_used);
	virtio_write(device->common_field+VIRTIO_REG_QUEUE_DEVICE_HI,4,queue_used>>32);
	virtio_write(device->common_field+VIRTIO_REG_QUEUE_ENABLE,2,1);
	return out;
}



KERNEL_PUBLIC void virtio_queue_transfer(virtio_queue_t* queue,const virtio_buffer_t* buffers,u16 tx_count,u16 rx_count){
	u16 i=queue->first_free_index;
	for (u16 count=tx_count+rx_count;count;count--){
		(queue->descriptors+i)->flags=(count>1?VIRTQ_DESC_F_NEXT:0)|(count>rx_count?0:VIRTQ_DESC_F_WRITE);
		(queue->descriptors+i)->address=buffers->address;
		(queue->descriptors+i)->length=buffers->length;
		i=(queue->descriptors+i)->next;
		buffers++;
	}
	u16 available_index=queue->available->index;
	queue->available->ring[available_index]=queue->first_free_index;
	queue->available->index=(available_index+1==queue->size?0:available_index+1);
	queue->first_free_index=i;
	virtio_write(queue->device->notify_field+queue->notify_offset*queue->device->notify_off_multiplier,2,queue->index);
}



KERNEL_PUBLIC void virtio_queue_wait(virtio_queue_t* queue){
	SPINLOOP(queue->last_used_index==queue->used->index);
}



KERNEL_PUBLIC u32 virtio_queue_pop(virtio_queue_t* queue,u32* length){
	queue->last_used_index=(queue->last_used_index+1==queue->size?0:queue->last_used_index+1);
	if (length){
		*length=(queue->used->ring+queue->last_used_index)->length;
	}
	u32 out=(queue->used->ring+queue->last_used_index)->index;
	virtio_read(queue->device->isr_field,1);
	return out;
}



void virtio_locate_devices(void){
	LOG("Initializing VirtIO driver...");
	_virtio_device_allocator=omm_init("virtio_device",sizeof(virtio_device_t),8,1,pmm_alloc_counter("omm_virtio_device"));
	spinlock_init(&(_virtio_device_allocator->lock));
	_virtio_device_driver_node_allocator=omm_init("virtio_device_driver_node",sizeof(virtio_device_driver_node_t),8,1,pmm_alloc_counter("omm_virtio_device_driver_node"));
	spinlock_init(&(_virtio_device_driver_node_allocator->lock));
	_virtio_device_handle_type=handle_alloc("virtio_device",NULL);
	rb_tree_init(&_virtio_device_driver_tree);
	spinlock_init(&_virtio_device_driver_tree_lock);
	_virtio_queue_allocator=omm_init("virtio_queue",sizeof(virtio_queue_t),8,1,pmm_alloc_counter("omm_virtio_queue"));
	spinlock_init(&(_virtio_queue_allocator->lock));
	_virtio_queue_pmm_counter=pmm_alloc_counter("virtio_queue");
	HANDLE_FOREACH(pci_device_handle_type){
		pci_device_t* device=handle->object;
		_virtio_init_device(device);
	}
}
