#include <kernel/acpi/structures.h>
#include <kernel/acpi/tpm2.h>
#include <kernel/aml/bus.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/tpm/commands.h>
#include <kernel/tpm/tpm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "tpm"



#define TPM_COMMAND_BUFFER_SIZE PAGE_SIZE



#define	TPM_REG_ACCESS(locality) (0x0000|((locality)<<10))
#define	TPM_REG_INT_ENABLE(locality) (0x0002|((locality)<<10))
#define	TPM_REG_INTF_CAPS(locality) (0x0005|((locality)<<10))
#define	TPM_REG_STS(locality) (0x0006|((locality)<<10))
#define	TPM_REG_DATA_FIFO(locality) (0x009|((locality)<<10))
#define	TPM_REG_DID_VID(locality) (0x03c0|((locality)<<10))
#define	TPM_REG_RID(locality) (0x03c1|((locality)<<10))

#define TPM_ACCESS_REQUEST_USE 0x02
#define TPM_ACCESS_REQUEST_PENDING 0x04
#define TPM_ACCESS_ACTIVE_LOCALITY 0x20
#define TPM_ACCESS_VALID 0x80

#define TPM_INTF_DATA_AVAIL_INT 0x001
#define TPM_INTF_STS_VALID_INT 0x002
#define TPM_INTF_LOCALITY_CHANGE_INT 0x004
#define TPM_INTF_INT_LEVEL_HIGH 0x008
#define TPM_INTF_INT_LEVEL_LOW 0x010
#define TPM_INTF_INT_EDGE_RISING 0x020
#define TPM_INTF_INT_EDGE_FALLING 0x040
#define TPM_INTF_CMD_READY_INT 0x080
#define TPM_INTF_BURST_COUNT_STATIC 0x100
#define TPM_GLOBAL_INT_ENABLE 0x80000000

#define TPM_STS_RESPONSE_RETRY 0x02
#define TPM_STS_DATA_EXPECT 0x08
#define TPM_STS_DATA_AVAIL 0x10
#define TPM_STS_GO 0x20
#define TPM_STS_COMMAND_READY 0x40
#define TPM_STS_VALID 0x80

#define TPM_STS_READ_ZERO 0x23



static omm_allocator_t* _tpm_allocator=NULL;
static pmm_counter_descriptor_t* _tpm_command_buffer_pmm_counter=NULL;
static u8 _tpm_signatures[TPM_SIGNATURE_MAX_TYPE+1][32];



static void _chip_start(tpm_t* tpm){
	if ((tpm->regs[TPM_REG_ACCESS(0)]&(TPM_ACCESS_VALID|TPM_ACCESS_ACTIVE_LOCALITY|TPM_ACCESS_REQUEST_USE))!=(TPM_ACCESS_VALID|TPM_ACCESS_ACTIVE_LOCALITY)){
		tpm->regs[TPM_REG_ACCESS(0)]=TPM_ACCESS_REQUEST_USE;
		SPINLOOP((tpm->regs[TPM_REG_ACCESS(0)]&(TPM_ACCESS_VALID|TPM_ACCESS_ACTIVE_LOCALITY|TPM_ACCESS_REQUEST_USE))!=(TPM_ACCESS_VALID|TPM_ACCESS_ACTIVE_LOCALITY));
	}
	tpm->regs[TPM_REG_STS(0)]=TPM_STS_COMMAND_READY;
}



static void _chip_stop(tpm_t* tpm){
	tpm->regs[TPM_REG_ACCESS(0)]=TPM_ACCESS_ACTIVE_LOCALITY;
}



static void _send_data(tpm_t* tpm,const void* buffer,u32 buffer_size){
	if (!(tpm->regs[TPM_REG_STS(0)]&TPM_STS_COMMAND_READY)){
		tpm->regs[TPM_REG_STS(0)]=TPM_STS_COMMAND_READY;
		SPINLOOP((tpm->regs[TPM_REG_STS(0)]&TPM_STS_COMMAND_READY)!=TPM_STS_COMMAND_READY);
	}
	u32 count=0;
	while (count<buffer_size-1){
		u32 burst_count;
		do{
			burst_count=(tpm->regs[TPM_REG_STS(0)]>>8)&0xffff;
		} while (!burst_count);
		if (buffer_size-count-1<burst_count){
			burst_count=buffer_size-count-1;
		}
		burst_count+=count;
		for (volatile u8* fifo=(volatile u8*)(tpm->regs+TPM_REG_DATA_FIFO(0));count<burst_count;count++){
			*fifo=*((const u8*)(buffer+count));
		}
		SPINLOOP((tpm->regs[TPM_REG_STS(0)]&TPM_STS_VALID)!=TPM_STS_VALID);
		if (!(tpm->regs[TPM_REG_STS(0)]&TPM_STS_DATA_EXPECT)){
			panic("End of data");
		}
	}
	*((u8*)(tpm->regs+TPM_REG_DATA_FIFO(0)))=*((const u8*)(buffer+count));
	SPINLOOP((tpm->regs[TPM_REG_STS(0)]&TPM_STS_VALID)!=TPM_STS_VALID);
	if (tpm->regs[TPM_REG_STS(0)]&TPM_STS_DATA_EXPECT){
		panic("Not enough data");
	}
}



static void _recv_data(tpm_t* tpm,void* buffer,u32 buffer_size){
	u32 count=0;
	while (count<buffer_size){
		SPINLOOP((tpm->regs[TPM_REG_STS(0)]&(TPM_STS_VALID|TPM_STS_DATA_AVAIL))!=(TPM_STS_VALID|TPM_STS_DATA_AVAIL));
		u32 burst_count;
		do{
			burst_count=(tpm->regs[TPM_REG_STS(0)]>>8)&0xffff;
		} while (!burst_count);
		if (buffer_size-count<burst_count){
			burst_count=buffer_size-count;
		}
		burst_count+=count;
		for (volatile u8* fifo=(volatile u8*)(tpm->regs+TPM_REG_DATA_FIFO(0));count<burst_count;count++){
			*((u8*)(buffer+count))=*fifo;
		}
	}
}



static void _execute_command(tpm_t* tpm){
	_send_data(tpm,tpm->command,__builtin_bswap32(tpm->command->header.length));
	tpm->regs[TPM_REG_STS(0)]=TPM_STS_GO;
	SPINLOOP((tpm->regs[TPM_REG_STS(0)]&(TPM_STS_VALID|TPM_STS_DATA_AVAIL))!=(TPM_STS_VALID|TPM_STS_DATA_AVAIL));
	_recv_data(tpm,tpm->command,sizeof(tpm_command_header_t));
	if (__builtin_bswap32(tpm->command->header.length)>TPM_COMMAND_BUFFER_SIZE){
		panic("Response too large");
	}
	_recv_data(tpm,tpm->command->_raw_data,__builtin_bswap32(tpm->command->header.length)-sizeof(tpm_command_header_t));
	tpm->regs[TPM_REG_STS(0)]=TPM_STS_COMMAND_READY;
}



static void _probe_tpm2(tpm_t* tpm){
	tpm->command->header.tag=__builtin_bswap16(TPM2_ST_NO_SESSIONS);
	tpm->command->header.length=__builtin_bswap32(sizeof(tpm_command_header_t)+sizeof(tpm->command->get_capability));
	tpm->command->header.command_code=__builtin_bswap32(TPM2_CC_GET_CAPABILITY);
	tpm->command->get_capability.capability=__builtin_bswap32(TPM2_CAP_TPM_PROPERTIES);
	tpm->command->get_capability.property=__builtin_bswap32(TPM_PT_TOTAL_COMMANDS);
	tpm->command->get_capability.property_count=__builtin_bswap32(1);
	_execute_command(tpm);
	if (__builtin_bswap16(tpm->command->header.tag)==TPM2_ST_NO_SESSIONS){
		tpm->flags|=TPM_FLAG_VERSION_2;
	}
	INFO("Detected chip version: %s",((tpm->flags&TPM_FLAG_VERSION_2)?"2.0":"1.2"));
}



static _Bool _self_test_tpm2(tpm_t* tpm){
	_Bool full_self_test=0;
_retry_self_test:
	tpm->command->header.tag=__builtin_bswap16(TPM2_ST_NO_SESSIONS);
	tpm->command->header.length=__builtin_bswap32(sizeof(tpm_command_header_t)+sizeof(tpm->command->self_test));
	tpm->command->header.command_code=__builtin_bswap32(TPM2_CC_SELF_TEST);
	tpm->command->self_test.full_test=full_self_test;
	_execute_command(tpm);
	if (__builtin_bswap32(tpm->command->header.return_code)==TPM2_RC_SUCCESS||__builtin_bswap32(tpm->command->header.return_code)==TPM2_RC_INITIALIZE||__builtin_bswap32(tpm->command->header.return_code)==TPM2_RC_TESTING){
		return __builtin_bswap32(tpm->command->header.return_code)==TPM2_RC_INITIALIZE;
	}
	if (!full_self_test){
		full_self_test=1;
		goto _retry_self_test;
	}
	panic("TPM2 self-test failed");
}



static u32 _read_pcr_value(tpm_t* tpm,u32 pcr_index,u16 hash_alg,void* buffer,u32 buffer_size){
	u32 digest_size=0;
	for (u32 i=0;i<tpm->bank_count;i++){
		if ((tpm->banks+i)->hash_alg==hash_alg){
			digest_size=(tpm->banks+i)->digest_size;
			goto _digest_size_found;
		}
	}
	return 0;
_digest_size_found:
	tpm->command->header.tag=__builtin_bswap16(TPM2_ST_NO_SESSIONS);
	tpm->command->header.length=__builtin_bswap32(sizeof(tpm_command_header_t)+sizeof(tpm->command->pcr_read));
	tpm->command->header.command_code=__builtin_bswap32(TPM2_CC_PCR_READ);
	tpm->command->pcr_read.selection_count=__builtin_bswap32(1);
	tpm->command->pcr_read.selection_hash_alg=__builtin_bswap16(hash_alg);
	tpm->command->pcr_read.selection_size=sizeof(tpm->command->pcr_read.selection_data);
	for (u32 i=0;i<tpm->command->pcr_read.selection_size;i++){
		tpm->command->pcr_read.selection_data[i]=0;
	}
	tpm->command->pcr_read.selection_data[pcr_index>>3]|=1<<(pcr_index&7);
	_execute_command(tpm);
	if (__builtin_bswap32(tpm->command->header.return_code)!=TPM2_RC_SUCCESS||!__builtin_bswap32(tpm->command->pcr_read_resp.digest_count)||__builtin_bswap16(tpm->command->pcr_read_resp.digest_size)!=digest_size){
		return 0;
	}
	if (digest_size<buffer_size){
		buffer_size=digest_size;
	}
	memcpy(buffer,tpm->command->pcr_read_resp.data,buffer_size);
	return buffer_size;
}



static _Bool _init_aml_device(aml_bus_device_t* device){
	if (!acpi_tpm2||acpi_tpm2->start_method!=ACPI_TPM2_START_METHOD_MEMORY_MAPPED){
		return 0;
	}
	const aml_bus_device_resource_t* res=aml_bus_device_get_resource(device,AML_BUS_RESOURCE_TYPE_MEMORY_REGION,0);
	if (!res){
		return 0;
	}
	LOG("Found TMP device: %s (SystemBus)",device->device->name);
	tpm_t* tpm=omm_alloc(_tpm_allocator);
	tpm->flags=0;
	tpm->regs=(void*)vmm_identity_map(res->memory_region.base,res->memory_region.size);
	tpm->command=(void*)(pmm_alloc(pmm_align_up_address(TPM_COMMAND_BUFFER_SIZE)>>PAGE_SIZE_SHIFT,_tpm_command_buffer_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	INFO("Memory range: %p - %p",res->memory_region.base,res->memory_region.base+res->memory_region.size);
	INFO("Vendor ID: %x, Device ID: %x, Revision ID: %x",tpm->regs[TPM_REG_DID_VID(0)]&0xffff,tpm->regs[TPM_REG_DID_VID(0)]>>16,tpm->regs[TPM_REG_RID(0)]);
	SPINLOOP(!(tpm->regs[TPM_REG_ACCESS(0)]&TPM_ACCESS_VALID));
	u32 interrupt_mask=tpm->regs[TPM_REG_INT_ENABLE(0)];
	u32 capabilites=tpm->regs[TPM_REG_INTF_CAPS(0)];
	INFO("Interrupt mask: %x, Capabilites: %x",interrupt_mask,capabilites);
	interrupt_mask=(interrupt_mask|TPM_INTF_CMD_READY_INT|TPM_INTF_LOCALITY_CHANGE_INT|TPM_INTF_STS_VALID_INT|TPM_INTF_DATA_AVAIL_INT)&(~TPM_GLOBAL_INT_ENABLE);
	_chip_start(tpm);
	tpm->regs[TPM_REG_INT_ENABLE(0)]=interrupt_mask;
	_probe_tpm2(tpm);
	if (!(tpm->flags&TPM_FLAG_VERSION_2)){
		ERROR("Found legacy TPM device");
		panic("Legacy TPM device");
	}
	_Bool require_initialization=_self_test_tpm2(tpm);
	if (require_initialization){
		panic("TPM2 requires initialization");
	}
	tpm->command->header.tag=__builtin_bswap16(TPM2_ST_NO_SESSIONS);
	tpm->command->header.length=__builtin_bswap32(sizeof(tpm_command_header_t)+sizeof(tpm->command->get_capability));
	tpm->command->header.command_code=__builtin_bswap32(TPM2_CC_GET_CAPABILITY);
	tpm->command->get_capability.capability=__builtin_bswap32(TPM2_CAP_TPM_PROPERTIES);
	tpm->command->get_capability.property=__builtin_bswap32(TPM_PT_TOTAL_COMMANDS);
	tpm->command->get_capability.property_count=__builtin_bswap32(1);
	_execute_command(tpm);
	if (!__builtin_bswap32(tpm->command->get_capability_resp_tpm_properties.property_count)){
		panic("No properties returned");
	}
	tpm->device_command_count=__builtin_bswap32(tpm->command->get_capability_resp_tpm_properties.value);
	tpm->command->header.tag=__builtin_bswap16(TPM2_ST_NO_SESSIONS);
	tpm->command->header.length=__builtin_bswap32(sizeof(tpm_command_header_t)+sizeof(tpm->command->get_capability));
	tpm->command->header.command_code=__builtin_bswap32(TPM2_CC_GET_CAPABILITY);
	tpm->command->get_capability.capability=__builtin_bswap32(TPM2_CAP_COMMANDS);
	tpm->command->get_capability.property=__builtin_bswap32(TPM2_CC_FIRST);
	tpm->command->get_capability.property_count=__builtin_bswap32(tpm->device_command_count);
	_execute_command(tpm);
	if (__builtin_bswap32(tpm->command->get_capability_resp_commands.command_count)!=tpm->device_command_count){
		panic("Inconsistent command count");
	}
	tpm->device_commands=amm_alloc(tpm->device_command_count*sizeof(tpm_device_command_t));
	for (u32 i=0;i<tpm->device_command_count;i++){
		u32 entry=__builtin_bswap32(tpm->command->get_capability_resp_commands.commands[i]);
		(tpm->device_commands+i)->cc=entry;
		(tpm->device_commands+i)->attributes=entry>>16;
	}
	tpm->command->header.tag=__builtin_bswap16(TPM2_ST_NO_SESSIONS);
	tpm->command->header.length=__builtin_bswap32(sizeof(tpm_command_header_t)+sizeof(tpm->command->get_capability));
	tpm->command->header.command_code=__builtin_bswap32(TPM2_CC_GET_CAPABILITY);
	tpm->command->get_capability.capability=__builtin_bswap32(TPM2_CAP_PCRS);
	tpm->command->get_capability.property=__builtin_bswap32(0);
	tpm->command->get_capability.property_count=__builtin_bswap32(1);
	_execute_command(tpm);
	tpm->bank_count=__builtin_bswap32(tpm->command->get_capability_resp_pcrs.bank_count);
	tpm->banks=amm_alloc(tpm->bank_count*sizeof(tpm_bank_t));
	u32 bank_idx=0;
	const u8* ptr=tpm->command->get_capability_resp_pcrs.data;
	for (u32 i=0;i<tpm->bank_count;i++){
		u16 hash_alg=(ptr[0]<<8)|ptr[1];
		u8 size_of_selection=ptr[2];
		ptr+=3;
		for (u32 j=0;j<size_of_selection;j++){
			if (!ptr[j]){
				continue;
			}
			u32 digest_size=0;
			if (hash_alg==TPM_ALG_SHA1){
				digest_size=20;
			}
			else if (hash_alg==TPM_ALG_SHA256){
				digest_size=32;
			}
			else if (hash_alg==TPM_ALG_SHA384){
				digest_size=48;
			}
			else if (hash_alg==TPM_ALG_SHA512){
				digest_size=64;
			}
			else{
				panic("Unknown hash algorithm digest size");
			}
			(tpm->banks+bank_idx)->hash_alg=hash_alg;
			(tpm->banks+bank_idx)->digest_size=digest_size;
			bank_idx++;
			break;
		}
		ptr+=size_of_selection;
	}
	tpm->bank_count=bank_idx;
	tpm->banks=amm_realloc(tpm->banks,tpm->bank_count*sizeof(tpm_bank_t));
	for (u32 i=0;i<TPM2_PLATFORM_PCR_COUNT;i++){
		u8 buffer[32];
		if (_read_pcr_value(tpm,i,TPM_ALG_SHA256,buffer,sizeof(buffer))==sizeof(buffer)){
			WARN("PCR[%u]: %X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X",i,buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],buffer[8],buffer[9],buffer[10],buffer[11],buffer[12],buffer[13],buffer[14],buffer[15],buffer[16],buffer[17],buffer[18],buffer[19],buffer[20],buffer[21],buffer[22],buffer[23],buffer[24],buffer[25],buffer[26],buffer[27],buffer[28],buffer[29],buffer[30],buffer[31]);
		}
	}
	_chip_stop(tpm);
	return 1;
}



static void _deinit_aml_device(aml_bus_device_t* device){
	return;
}



static const aml_bus_device_driver_t _tmp_aml_bus_device_driver={
	"TPM2",
	AML_BUS_ADDRESS_TYPE_HID_STR,
	{.hid_str="MSFT0101"},
	_init_aml_device,
	_deinit_aml_device
};



KERNEL_EARLY_EARLY_INIT(){
	LOG("Clearing TPM signatures...");
	for (u32 i=0;i<=TPM_SIGNATURE_MAX_TYPE;i++){
		memset(_tpm_signatures[i],0,32);
	}
}



KERNEL_INIT(){
	LOG("Initializing TPM driver...");
	_tpm_allocator=omm_init("tpm",sizeof(tpm_t),8,1,pmm_alloc_counter("omm_tpm"));
	spinlock_init(&(_tpm_allocator->lock));
	_tpm_command_buffer_pmm_counter=pmm_alloc_counter("tpm_command_buffer");
	aml_bus_register_device_driver(&_tmp_aml_bus_device_driver);
}



void KERNEL_EARLY_EXEC tpm_register_signature(u32 type,const void* data){
	if (type>TPM_SIGNATURE_MAX_TYPE){
		panic("tpm_register_signature: invalid signature type");
	}
	memcpy(_tpm_signatures[type],data,32);
}
