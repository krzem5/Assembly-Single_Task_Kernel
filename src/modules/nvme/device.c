#include <kernel/drive/drive.h>
#include <kernel/format/format.h>
#include <kernel/handle/handle.h>
#include <kernel/lock/lock.h>
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



static pmm_counter_descriptor_t _nvme_driver_pmm_counter=PMM_COUNTER_INIT_STRUCT("nvme");
static pmm_counter_descriptor_t _nvme_device_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_nvme_device");
static omm_allocator_t _nvme_device_allocator=OMM_ALLOCATOR_INIT_STRUCT("nvme_deivce",sizeof(nvme_device_t),8,1,&_nvme_device_omm_pmm_counter);



static KERNEL_INLINE void _init_queue(nvme_device_t* device,u16 queue_index,u16 queue_length,nvme_queue_t* out){
	out->doorbell=(volatile u32*)(((u64)(device->registers))+0x1000+queue_index*device->doorbell_stride);
	out->mask=queue_length-1;
}



static void _completion_queue_init(nvme_device_t* device,u16 queue_index,u16 queue_length,nvme_completion_queue_t* out){
	_init_queue(device,queue_index,queue_length,&(out->queue));
	out->entries=(void*)(pmm_alloc_zero(pmm_align_up_address(queue_length*sizeof(nvme_completion_queue_entry_t))>>PAGE_SIZE_SHIFT,&_nvme_driver_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	out->head=0;
	out->phase=1;
}



static nvme_completion_queue_entry_t* _completion_queue_wait(nvme_submission_queue_t* queue){
	lock_acquire_exclusive(&(queue->lock));
	nvme_completion_queue_t* completion_queue=queue->completion_queue;
	nvme_completion_queue_entry_t* out=completion_queue->entries+completion_queue->head;
	SPINLOOP((out->status&1)!=completion_queue->phase);
	completion_queue->head=(completion_queue->head+1)&completion_queue->queue.mask;
	completion_queue->phase^=!completion_queue->head;
	queue->head=out->sq_head;
	*(completion_queue->queue.doorbell)=completion_queue->head;
	lock_release_exclusive(&(queue->lock));
	return out;
}



static void _submission_queue_init(nvme_device_t* device,nvme_completion_queue_t* completion_queue,u16 queue_index,u16 queue_length,nvme_submission_queue_t* out){
	_init_queue(device,queue_index,queue_length,&(out->queue));
	out->entries=(void*)(pmm_alloc_zero(pmm_align_up_address(queue_length*sizeof(nvme_submission_queue_entry_t))>>PAGE_SIZE_SHIFT,&_nvme_driver_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	out->completion_queue=completion_queue;
	lock_init(&(out->lock));
	out->head=0;
	out->tail=0;
}



static nvme_submission_queue_entry_t* _submission_queue_init_entry(nvme_submission_queue_t* queue,u8 opc){
	lock_acquire_exclusive(&(queue->lock));
	SPINLOOP(((queue->head+1)&queue->queue.mask)==queue->tail);
	nvme_submission_queue_entry_t* out=queue->entries+queue->tail;
	memset((void*)out,0,sizeof(nvme_submission_queue_entry_t));
	out->cdw0=(queue->tail<<16)|opc;
	return out;
}



static void _submission_queue_send_entry(nvme_submission_queue_t* queue){
	queue->tail=(queue->tail+1)&queue->queue.mask;
	*(queue->queue.doorbell)=queue->tail;
	lock_release_exclusive(&(queue->lock));
}



static _Bool _request_identify_data(nvme_device_t* device,u64 buffer,u8 cns,u32 namespace_id){
	nvme_submission_queue_entry_t* entry=_submission_queue_init_entry(&(device->admin_submission_queue),SQE_OPC_ADMIN_IDENTIFY);
	entry->dptr_prp1=buffer;
	entry->nsid=namespace_id;
	entry->extra_data[0]=cns;
	_submission_queue_send_entry(&(device->admin_submission_queue));
	return !(_completion_queue_wait(&(device->admin_submission_queue))->status&0x1fe);
}



static void _create_io_completion_queue(nvme_device_t* device,u16 queue_index,nvme_completion_queue_t* out){
	_completion_queue_init(device,queue_index,(device->registers->cap&0xffff)+1,out);
	nvme_submission_queue_entry_t* entry=_submission_queue_init_entry(&(device->admin_submission_queue),SQE_OPC_ADMIN_CREATE_IO_CQ);
	entry->dptr_prp1=((u64)(out->entries))-VMM_HIGHER_HALF_ADDRESS_OFFSET;
	entry->extra_data[0]=(out->queue.mask<<16)|(queue_index>>1);
	entry->extra_data[1]=1;
	_submission_queue_send_entry(&(device->admin_submission_queue));
	_completion_queue_wait(&(device->admin_submission_queue));
}



static void _create_io_submission_queue(nvme_device_t* device,nvme_completion_queue_t* completion_queue,u16 queue_index,nvme_submission_queue_t* out){
	_submission_queue_init(device,completion_queue,queue_index,(device->registers->cap&0xffff)+1,out);
	nvme_submission_queue_entry_t* entry=_submission_queue_init_entry(&(device->admin_submission_queue),SQE_OPC_ADMIN_CREATE_IO_SQ);
	entry->dptr_prp1=((u64)(out->entries))-VMM_HIGHER_HALF_ADDRESS_OFFSET;
	entry->extra_data[0]=(out->queue.mask<<16)|(queue_index>>1);
	entry->extra_data[1]=((queue_index>>1)<<16)|1;
	_submission_queue_send_entry(&(device->admin_submission_queue));
	_completion_queue_wait(&(device->admin_submission_queue));
}



static u64 _nvme_read_write(void* extra_data,u64 offset,void* buffer,u64 count){
	panic("_nvme_read_write");
}



static drive_type_t _nvme_drive_type={
	"NVMe",
	_nvme_read_write
};



static void _load_namespace(nvme_device_t* device,u32 namespace_id,const nvme_identify_data_t* controller_identify_data,nvme_identify_data_t* identify_data){
	if (!_request_identify_data(device,((u64)identify_data)-VMM_HIGHER_HALF_ADDRESS_OFFSET,ADMIN_IDENTIFY_CNS_ID_NS,namespace_id)||!identify_data->namespace.nsze||(identify_data->namespace.flbas&0xf)>=identify_data->namespace.nlbaf){
		return;
	}
	INFO("Found valid namespace: %u",namespace_id);
	drive_config_t config={
		.type=&_nvme_drive_type,
		.block_count=identify_data->namespace.nsze,
		.block_size=1<<(identify_data->namespace.lbaf+(identify_data->namespace.flbas&0xf))->lbads,
		.extra_data=device
	};
	format_string(config.name,DRIVE_NAME_LENGTH,"nvme%u",namespace_id);
	memcpy_trunc_spaces(config.serial_number,(const char*)(controller_identify_data->controller.sn),20);
	memcpy_trunc_spaces(config.model_number,(const char*)(controller_identify_data->controller.mn),40);
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
	nvme_device_t* nvme_device=omm_alloc(&_nvme_device_allocator);
	nvme_device->registers=registers;
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
	nvme_identify_data_t* identify_data=(void*)(pmm_alloc(2,&_nvme_driver_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	_request_identify_data(nvme_device,((u64)identify_data)-VMM_HIGHER_HALF_ADDRESS_OFFSET,ADMIN_IDENTIFY_CNS_ID_CTRL,0);
	INFO("Namespace count: %u, Maximum data transfer size: %u",identify_data->controller.nn,identify_data->controller.mdts);
	_create_io_completion_queue(nvme_device,3,&(nvme_device->io_completion_queue));
	_create_io_submission_queue(nvme_device,&(nvme_device->io_completion_queue),2,&(nvme_device->io_submission_queue));
	for (u32 i=0;i<identify_data->controller.nn;i++){
		_load_namespace(nvme_device,i,identify_data,(void*)(((u64)identify_data)+PAGE_SIZE));
	}
	pmm_dealloc(((u64)identify_data)-VMM_HIGHER_HALF_ADDRESS_OFFSET,2,&_nvme_driver_pmm_counter);

}



void nvme_locate_devices(void){
	HANDLE_FOREACH(HANDLE_TYPE_PCI_DEVICE){
		pci_device_t* device=handle->object;
		_nvme_init_device(device);
	}
}
