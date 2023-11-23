#include <kernel/drive/drive.h>
#include <kernel/format/format.h>
#include <kernel/handle/handle.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/pci/pci.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <nvme/device.h>
#include <nvme/registers.h>
#define KERNEL_LOG_NAME "nvme"



static pmm_counter_descriptor_t* _nvme_driver_pmm_counter=NULL;
static omm_allocator_t* _nvme_device_allocator=NULL;



static u16 _nvme_device_index=0;



static KERNEL_INLINE void _init_queue(nvme_device_t* device,u16 queue_index,u16 queue_length,nvme_queue_t* out){
	out->doorbell=(volatile u32*)(((u64)(device->registers))+0x1000+queue_index*device->doorbell_stride);
	out->mask=queue_length-1;
}



static void _completion_queue_init(nvme_device_t* device,u16 queue_index,u16 queue_length,nvme_completion_queue_t* out){
	_init_queue(device,queue_index,queue_length,&(out->queue));
	out->entries=(void*)(pmm_alloc(pmm_align_up_address(queue_length*sizeof(nvme_completion_queue_entry_t))>>PAGE_SIZE_SHIFT,_nvme_driver_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	out->head=0;
	out->phase=1;
}



static nvme_completion_queue_entry_t* _completion_queue_wait(nvme_submission_queue_t* queue){
	spinlock_acquire_exclusive(&(queue->lock));
	nvme_completion_queue_t* completion_queue=queue->completion_queue;
	nvme_completion_queue_entry_t* out=completion_queue->entries+completion_queue->head;
	SPINLOOP((out->status&1)!=completion_queue->phase);
	completion_queue->head=(completion_queue->head+1)&completion_queue->queue.mask;
	completion_queue->phase^=!completion_queue->head;
	queue->head=out->sq_head;
	*(completion_queue->queue.doorbell)=completion_queue->head;
	spinlock_release_exclusive(&(queue->lock));
	return out;
}



static void _submission_queue_init(nvme_device_t* device,nvme_completion_queue_t* completion_queue,u16 queue_index,u16 queue_length,nvme_submission_queue_t* out){
	_init_queue(device,queue_index,queue_length,&(out->queue));
	out->entries=(void*)(pmm_alloc(pmm_align_up_address(queue_length*sizeof(nvme_submission_queue_entry_t))>>PAGE_SIZE_SHIFT,_nvme_driver_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	out->completion_queue=completion_queue;
	spinlock_init(&(out->lock));
	out->head=0;
	out->tail=0;
}



static nvme_submission_queue_entry_t* _submission_queue_init_entry(nvme_submission_queue_t* queue,u8 opc){
	spinlock_acquire_exclusive(&(queue->lock));
	SPINLOOP(((queue->head+1)&queue->queue.mask)==queue->tail);
	nvme_submission_queue_entry_t* out=queue->entries+queue->tail;
	memset((void*)out,0,sizeof(nvme_submission_queue_entry_t));
	out->cdw0=(queue->tail<<16)|opc;
	return out;
}



static void _submission_queue_send_entry(nvme_submission_queue_t* queue){
	queue->tail=(queue->tail+1)&queue->queue.mask;
	*(queue->queue.doorbell)=queue->tail;
	spinlock_release_exclusive(&(queue->lock));
}



static _Bool _request_identify_data(nvme_device_t* device,u64 buffer,u8 cns,u32 namespace_id){
	nvme_submission_queue_entry_t* entry=_submission_queue_init_entry(&(device->admin_submission_queue),OPC_ADMIN_IDENTIFY);
	entry->dptr_prp1=buffer;
	entry->nsid=namespace_id;
	entry->extra_data[0]=cns;
	_submission_queue_send_entry(&(device->admin_submission_queue));
	return !(_completion_queue_wait(&(device->admin_submission_queue))->status&0x1fe);
}



static void _create_io_completion_queue(nvme_device_t* device,u16 queue_index,nvme_completion_queue_t* out){
	_completion_queue_init(device,queue_index,(device->registers->cap&0xffff)+1,out);
	nvme_submission_queue_entry_t* entry=_submission_queue_init_entry(&(device->admin_submission_queue),OPC_ADMIN_CREATE_IO_CQ);
	entry->dptr_prp1=((u64)(out->entries))-VMM_HIGHER_HALF_ADDRESS_OFFSET;
	entry->extra_data[0]=(out->queue.mask<<16)|(queue_index>>1);
	entry->extra_data[1]=1;
	_submission_queue_send_entry(&(device->admin_submission_queue));
	_completion_queue_wait(&(device->admin_submission_queue));
}



static void _create_io_submission_queue(nvme_device_t* device,nvme_completion_queue_t* completion_queue,u16 queue_index,nvme_submission_queue_t* out){
	_submission_queue_init(device,completion_queue,queue_index,(device->registers->cap&0xffff)+1,out);
	nvme_submission_queue_entry_t* entry=_submission_queue_init_entry(&(device->admin_submission_queue),OPC_ADMIN_CREATE_IO_SQ);
	entry->dptr_prp1=((u64)(out->entries))-VMM_HIGHER_HALF_ADDRESS_OFFSET;
	entry->extra_data[0]=(out->queue.mask<<16)|(queue_index>>1);
	entry->extra_data[1]=((queue_index>>1)<<16)|1;
	_submission_queue_send_entry(&(device->admin_submission_queue));
	_completion_queue_wait(&(device->admin_submission_queue));
}



static u64 _nvme_read_write(drive_t* drive,u64 offset,void* buffer,u64 count){
	nvme_device_t* device=drive->extra_data;
	if (count>(device->max_request_size>>drive->block_size_shift)){
		count=device->max_request_size>>drive->block_size_shift;
	}
	u64 aligned_buffer=pmm_alloc(pmm_align_up_address(count<<drive->block_size_shift)>>PAGE_SIZE_SHIFT,_nvme_driver_pmm_counter,0);
	if (offset&DRIVE_OFFSET_FLAG_WRITE){
		memcpy((void*)(aligned_buffer+VMM_HIGHER_HALF_ADDRESS_OFFSET),buffer,count<<drive->block_size_shift);
	}
	nvme_submission_queue_entry_t* entry=_submission_queue_init_entry(&(device->io_submission_queue),((offset&DRIVE_OFFSET_FLAG_WRITE)?OPC_IO_WRITE:OPC_IO_READ));
	entry->dptr_prp1=aligned_buffer;
	entry->extra_data[0]=offset;
	entry->extra_data[1]=offset>>32;
	entry->extra_data[2]=0x80000000|(count-1);
	_submission_queue_send_entry(&(device->io_submission_queue));
	_completion_queue_wait(&(device->io_submission_queue));
	if (!(offset&DRIVE_OFFSET_FLAG_WRITE)){
		memcpy(buffer,(void*)(aligned_buffer+VMM_HIGHER_HALF_ADDRESS_OFFSET),count<<drive->block_size_shift);
	}
	pmm_dealloc(aligned_buffer,pmm_align_up_address(count<<drive->block_size_shift)>>PAGE_SIZE_SHIFT,_nvme_driver_pmm_counter);
	return 0;
}



static const drive_type_t _nvme_drive_type_config={
	"nvme",
	_nvme_read_write
};



static void _load_namespace(nvme_device_t* device,u32 namespace_id,const nvme_identify_data_t* controller_identify_data,nvme_identify_data_t* identify_data){
	if (!_request_identify_data(device,((u64)identify_data)-VMM_HIGHER_HALF_ADDRESS_OFFSET,CNS_ID_NS,namespace_id)||!identify_data->namespace.nsze||(identify_data->namespace.flbas&0xf)>=identify_data->namespace.nlbaf){
		return;
	}
	INFO("Found valid namespace: %u",namespace_id);
	char serial_number_buffer[21];
	char model_number_buffer[41];
	memcpy_trunc_spaces(serial_number_buffer,(const char*)(controller_identify_data->controller.sn),20);
	memcpy_trunc_spaces(model_number_buffer,(const char*)(controller_identify_data->controller.mn),40);
	drive_config_t config={
		&_nvme_drive_type_config,
		device->index,
		namespace_id,
		smm_alloc(serial_number_buffer,0),
		smm_alloc(model_number_buffer,0),
		identify_data->namespace.nsze,
		1<<(identify_data->namespace.lbaf+(identify_data->namespace.flbas&0xf))->lbads,
		device
	};
	drive_create(&config);
}



static void _nvme_init_device(pci_device_t* device){
	if (device->class!=0x01||device->subclass!=0x08||device->progif!=0x02){
		return;
	}
	pci_device_enable_memory_access(device);
	pci_device_enable_bus_mastering(device);
	pci_bar_t pci_bar;
	if (!pci_device_get_bar(device,0,&pci_bar)){
		return;
	}
	LOG("Attached NVMe driver to PCI device %x:%x:%x",device->address.bus,device->address.slot,device->address.func);
	nvme_registers_t* registers=(void*)vmm_identity_map(pci_bar.address,pci_bar.size);
	if (!(registers->cap&CAP_CSS_NVME)){
		WARN("NVMe instruction set not supported");
		return;
	}
	nvme_device_t* nvme_device=omm_alloc(_nvme_device_allocator);
	nvme_device->registers=registers;
	nvme_device->index=_nvme_device_index;
	_nvme_device_index++;
	INFO("NVMe version %x.%x.%x",registers->vs>>16,(registers->vs>>8)&0xff,registers->vs&0xff);
	registers->cc=0;
	SPINLOOP(registers->csts&CSTS_RDY);
	u32 queue_entries=(registers->cap&0xffff)+1;
	nvme_device->doorbell_stride=4<<((registers->cap>>32)&0xf);
	INFO("Queue entry count: %u, Doorbell stride: %v",queue_entries,nvme_device->doorbell_stride);
	_completion_queue_init(nvme_device,1,PAGE_SIZE/sizeof(nvme_completion_queue_entry_t),&(nvme_device->admin_completion_queue));
	_submission_queue_init(nvme_device,&(nvme_device->admin_completion_queue),0,PAGE_SIZE/sizeof(nvme_submission_queue_entry_t),&(nvme_device->admin_submission_queue));
	registers->aqa=(nvme_device->admin_completion_queue.queue.mask<<16)|nvme_device->admin_submission_queue.queue.mask;
	registers->acq=((u64)(nvme_device->admin_completion_queue.entries))-VMM_HIGHER_HALF_ADDRESS_OFFSET;
	registers->asq=((u64)(nvme_device->admin_submission_queue.entries))-VMM_HIGHER_HALF_ADDRESS_OFFSET;
	registers->cc=CC_EN|0x460000;
	SPINLOOP(!(registers->csts&CSTS_RDY));
	nvme_identify_data_t* identify_data=(void*)(pmm_alloc(2,_nvme_driver_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	_request_identify_data(nvme_device,((u64)identify_data)-VMM_HIGHER_HALF_ADDRESS_OFFSET,CNS_ID_CTRL,0);
	nvme_device->max_request_size=PAGE_SIZE<<identify_data->controller.mdts;
	INFO("Namespace count: %u, Maximum data transfer size: %v",identify_data->controller.nn,nvme_device->max_request_size);
	_create_io_completion_queue(nvme_device,3,&(nvme_device->io_completion_queue));
	_create_io_submission_queue(nvme_device,&(nvme_device->io_completion_queue),2,&(nvme_device->io_submission_queue));
	for (u32 i=0;i<identify_data->controller.nn;i++){
		_load_namespace(nvme_device,i,identify_data,(void*)(((u64)identify_data)+PAGE_SIZE));
	}
	pmm_dealloc(((u64)identify_data)-VMM_HIGHER_HALF_ADDRESS_OFFSET,2,_nvme_driver_pmm_counter);

}



void nvme_locate_devices(void){
	_nvme_driver_pmm_counter=pmm_alloc_counter("nvme");
	_nvme_device_allocator=omm_init("nvme_device",sizeof(nvme_device_t),8,1,pmm_alloc_counter("omm_nvme_device"));
	spinlock_init(&(_nvme_device_allocator->lock));
	HANDLE_FOREACH(pci_device_handle_type){
		pci_device_t* device=handle->object;
		_nvme_init_device(device);
	}
}
