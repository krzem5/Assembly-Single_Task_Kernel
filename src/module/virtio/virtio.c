#include <kernel/handle/handle.h>
#include <kernel/isr/isr.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/module/module.h>
#include <kernel/mp/event.h>
#include <kernel/pci/msix.h>
#include <kernel/pci/pci.h>
#include <kernel/scheduler/load_balancer.h>
#include <kernel/timer/timer.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <virtio/registers.h>
#include <virtio/virtio.h>
#define KERNEL_LOG_NAME "virtio"



#define MAX_QUEUE_SIZE 256



static u16 _virtio_device_next_index=0;
static omm_allocator_t* KERNEL_INIT_WRITE _virtio_device_allocator=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _virtio_device_driver_node_allocator=NULL;
static rb_tree_t _virtio_device_driver_tree;
static rwlock_t _virtio_device_driver_tree_lock;
static omm_allocator_t* KERNEL_INIT_WRITE _virtio_queue_allocator=NULL;
static pmm_counter_descriptor_t* KERNEL_INIT_WRITE _virtio_queue_pmm_counter=NULL;
static handle_type_t KERNEL_INIT_WRITE _virtio_device_handle_type=0;



static void _virtio_irq_handler(void* ctx){
	virtio_device_t* device=ctx;
	for (virtio_queue_t* queue=device->queues;queue;queue=queue->next){
		event_dispatch(queue->event,EVENT_DISPATCH_FLAG_SET_ACTIVE|EVENT_DISPATCH_FLAG_BYPASS_ACL);
	}
}



static void _virtio_init_device(pci_device_t* device){
	if (device->device_id<0x1000||device->device_id>0x107f||device->vendor_id!=0x1af4){
		return;
	}
	LOG("Attached root virtio driver to PCI device %x:%x:%x",device->bus,device->slot,device->func);
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
	handle_new(_virtio_device_handle_type,&(virtio_device->handle));
	virtio_device->type=device->device_id-(device->device_id>=0x1040?0x1040:0x1000);
	virtio_device->index=_virtio_device_next_index;
	virtio_device->queues=NULL;
	_virtio_device_next_index++;
	rwlock_init(&(virtio_device->lock));
	if (found_structures!=(((1<<(VIRTIO_PCI_CAP_MAX-VIRTIO_PCI_CAP_MIN+1))-1)<<VIRTIO_PCI_CAP_MIN)){
		virtio_device->is_legacy=1;
		pci_bar_t pci_bar;
		if (!pci_device_get_bar(device,0,&pci_bar)){
			handle_destroy(&(virtio_device->handle));
			omm_dealloc(_virtio_device_allocator,virtio_device);
			return;
		}
		virtio_device->common_field=((pci_bar.flags&PCI_BAR_FLAG_MEMORY)?vmm_identity_map(pci_bar.address,pci_bar.size):pci_bar.address);
	}
	else{
		virtio_device->is_legacy=0;
	}
	virtio_write(virtio_device->common_field+VIRTIO_REG_DEVICE_STATUS,1,0x00);
	msix_table_t msix_table;
	if (pci_msix_load(device,&msix_table)){
		virtio_device->irq=isr_allocate();
		virtio_device->queue_msix_vector=0;
		if (!pci_msix_redirect_entry(&msix_table,virtio_device->queue_msix_vector,virtio_device->irq)){
			panic("Unable to initialize VirtIO device MSI-x vector");
		}
		IRQ_HANDLER_CTX(virtio_device->irq)=virtio_device;
		IRQ_HANDLER(virtio_device->irq)=_virtio_irq_handler;
	}
}



KERNEL_PUBLIC KERNEL_AWAITS bool virtio_register_device_driver(const virtio_device_driver_t* driver){
	rwlock_acquire_write(&_virtio_device_driver_tree_lock);
	LOG("Registering VirtIO device driver '%s/%X%X'...",driver->name,driver->type>>8,driver->type);
	virtio_device_driver_node_t* node=(virtio_device_driver_node_t*)rb_tree_lookup_node(&_virtio_device_driver_tree,driver->type);
	if (node){
		ERROR("VirtIO device type %X%X is already allocated by '%s'",driver->type>>8,driver->type,node->driver->name);
		rwlock_release_write(&_virtio_device_driver_tree_lock);
		return 0;
	}
	node=omm_alloc(_virtio_device_driver_node_allocator);
	node->rb_node.key=driver->type;
	node->driver=driver;
	rb_tree_insert_node(&_virtio_device_driver_tree,&(node->rb_node));
	rwlock_release_write(&_virtio_device_driver_tree_lock);
	HANDLE_FOREACH(_virtio_device_handle_type){
		virtio_device_t* device=KERNEL_CONTAINEROF(handle,virtio_device_t,handle);
		if (device->type!=driver->type||device->is_legacy!=driver->is_legacy){
			continue;
		}
		virtio_write(device->common_field+VIRTIO_REG_DEVICE_STATUS,1,VIRTIO_DEVICE_STATUS_FLAG_ACKNOWLEDGE);
		virtio_write(device->common_field+VIRTIO_REG_DEVICE_STATUS,1,VIRTIO_DEVICE_STATUS_FLAG_ACKNOWLEDGE|VIRTIO_DEVICE_STATUS_FLAG_DRIVER);
		virtio_write(device->common_field+VIRTIO_REG_DEVICE_FEATURE_SELECT,4,0);
		u64 features=virtio_read(device->common_field+VIRTIO_REG_DEVICE_FEATURE,4);
		virtio_write(device->common_field+VIRTIO_REG_DEVICE_FEATURE_SELECT,4,1);
		features|=virtio_read(device->common_field+VIRTIO_REG_DEVICE_FEATURE,4)<<32;
		INFO("Device features: %p",features);
		INFO("Driver features: %p",driver->features|(1<<VIRTIO_F_EVENT_IDX));
		features&=driver->features|(1<<VIRTIO_F_EVENT_IDX);
		virtio_write(device->common_field+VIRTIO_REG_DRIVER_FEATURE_SELECT,4,0);
		virtio_write(device->common_field+VIRTIO_REG_DRIVER_FEATURE,4,features);
		virtio_write(device->common_field+VIRTIO_REG_DRIVER_FEATURE_SELECT,4,1);
		virtio_write(device->common_field+VIRTIO_REG_DRIVER_FEATURE,4,features>>32);
		virtio_write(device->common_field+VIRTIO_REG_MSIX_CONFIG,2,0xffff);
		virtio_write(device->common_field+VIRTIO_REG_DEVICE_STATUS,1,VIRTIO_DEVICE_STATUS_FLAG_ACKNOWLEDGE|VIRTIO_DEVICE_STATUS_FLAG_DRIVER|VIRTIO_DEVICE_STATUS_FLAG_FEATURES_OK);
		if (!(virtio_read(device->common_field+VIRTIO_REG_DEVICE_STATUS,1)&VIRTIO_DEVICE_STATUS_FLAG_FEATURES_OK)){
			ERROR("Failed to negotiate VirtIO device features");
			virtio_write(device->common_field+VIRTIO_REG_DEVICE_STATUS,1,VIRTIO_DEVICE_STATUS_FLAG_FAILED);
			continue;
		}
		LOG("Negotiated device features: %p",features);
		device->events_negotiated=!!(features&(1<<VIRTIO_F_EVENT_IDX));
		if (!driver->init(device,features&(~(1<<VIRTIO_F_EVENT_IDX)))){
			virtio_write(device->common_field+VIRTIO_REG_DEVICE_STATUS,1,VIRTIO_DEVICE_STATUS_FLAG_FAILED);
		}
	}
	return 1;
}



KERNEL_PUBLIC bool virtio_unregister_device_driver(const virtio_device_driver_t* driver){
	rwlock_acquire_write(&_virtio_device_driver_tree_lock);
	LOG("Unregistering VirtIO device driver '%s/%X%X'...",driver->name,driver->type>>8,driver->type);
	rb_tree_node_t* node=rb_tree_lookup_node(&_virtio_device_driver_tree,driver->type);
	bool out=!!node;
	if (node){
		rb_tree_remove_node(&_virtio_device_driver_tree,node);
		omm_dealloc(_virtio_device_driver_node_allocator,node);
	}
	rwlock_release_write(&_virtio_device_driver_tree_lock);
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



KERNEL_PUBLIC virtio_queue_t* virtio_init_queue(virtio_device_t* device,u16 index){
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
	u64 queue_descriptors=pmm_alloc((pmm_align_up_address(size*sizeof(virtio_queue_descriptor_t)+sizeof(virtio_queue_available_t)+size*sizeof(u16)+sizeof(virtio_queue_event_t))+pmm_align_up_address(sizeof(virtio_queue_used_t)+size*sizeof(virtio_queue_used_entry_t)+sizeof(virtio_queue_event_t)))>>PAGE_SIZE_SHIFT,_virtio_queue_pmm_counter,0);
	u64 queue_available=queue_descriptors+size*sizeof(virtio_queue_descriptor_t);
	u64 queue_available_used_event=queue_available+sizeof(virtio_queue_available_t)+size*sizeof(u16);
	u64 queue_used=queue_descriptors+pmm_align_up_address(size*sizeof(virtio_queue_descriptor_t)+sizeof(virtio_queue_available_t)+size*sizeof(u16)+sizeof(virtio_queue_event_t));
	u64 queue_used_available_event=queue_used+sizeof(virtio_queue_used_t)+size*sizeof(virtio_queue_used_entry_t);
	virtio_queue_t* out=omm_alloc(_virtio_queue_allocator);
	out->next=device->queues;
	device->queues=out;
	out->device=device;
	out->index=index;
	out->size=size;
	out->notify_offset=virtio_read(device->common_field+VIRTIO_REG_QUEUE_NOTIFY_OFF,2);
	out->first_free_index=0;
	out->last_used_index=0;
	out->descriptors=(void*)(queue_descriptors+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	out->available=(void*)(queue_available+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	out->available_used_event=(void*)(queue_available_used_event+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	out->used=(void*)(queue_used+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	out->used_available_event=(void*)(queue_used_available_event+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	out->event=event_create("virtio.queue",NULL);
	for (u16 i=0;i<size-1;i++){ // last entry is zero-initialized by pmm_alloc
		(out->descriptors+i)->next=i+1;
	}
	if (!device->events_negotiated){
		out->available->flags=VIRTQ_AVAIL_F_NO_INTERRUPT;
	}
	virtio_write(device->common_field+VIRTIO_REG_QUEUE_MSIX_VECTOR,2,device->queue_msix_vector);
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
		i++;
		if (i>=queue->size){
			i=0;
		}
		buffers++;
	}
	u16 available_index=queue->available->index;
	queue->available->ring[available_index%queue->size]=queue->first_free_index;
	queue->available->index++;
	queue->first_free_index=i;
	queue->available_used_event->index=queue->last_used_index;
	virtio_write(queue->device->notify_field+queue->notify_offset*queue->device->notify_off_multiplier,2,queue->index);
}



KERNEL_PUBLIC KERNEL_AWAITS void virtio_queue_wait(virtio_queue_t* queue){
	// timer_t* timer=timer_create("virtio.queue.timeout",1000000000,TIMER_COUNT_INFINITE);
	while (queue->last_used_index==queue->used->index){
		event_t* events[2]={
			queue->event,
			// timer->event
		};
		event_await(events,1/*2*/,0);
		event_set_active(queue->event,0,1);
	}
	// timer_delete(timer);
}



KERNEL_PUBLIC u32 virtio_queue_pop(virtio_queue_t* queue,u32* length){
	u16 i=queue->last_used_index%queue->size;
	queue->last_used_index++;
	if (length){
		*length=(queue->used->ring+i)->length;
	}
	u32 out=(queue->used->ring+i)->index;
	(void)((queue->used->ring+i)->length);
	return out;
}



MODULE_INIT(){
	LOG("Initializing VirtIO driver...");
	_virtio_device_allocator=omm_init("virtio.device",sizeof(virtio_device_t),8,1);
	rwlock_init(&(_virtio_device_allocator->lock));
	_virtio_device_driver_node_allocator=omm_init("virtio.device_driver_node",sizeof(virtio_device_driver_node_t),8,1);
	rwlock_init(&(_virtio_device_driver_node_allocator->lock));
	_virtio_device_handle_type=handle_alloc("virtio.device",0,NULL);
	rb_tree_init(&_virtio_device_driver_tree);
	rwlock_init(&_virtio_device_driver_tree_lock);
	_virtio_queue_allocator=omm_init("virtio.queue",sizeof(virtio_queue_t),8,1);
	rwlock_init(&(_virtio_queue_allocator->lock));
	_virtio_queue_pmm_counter=pmm_alloc_counter("virtio.queue");
}



MODULE_POSTINIT(){
	HANDLE_FOREACH(pci_device_handle_type){
		_virtio_init_device(KERNEL_CONTAINEROF(handle,pci_device_t,handle));
	}
}
