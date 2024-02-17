#include <kernel/drive/drive.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/memory/vmm.h>
#include <kernel/random/random.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <test/test.h>
#define KERNEL_LOG_NAME "test"



static drive_t* _drive_io_callback_drive=NULL;
static u64 _drive_io_callback_offset=0;
static u64 _drive_io_callback_buffer=0;
static u64 _drive_io_callback_size=0;
static u64 _drive_io_callback_return=0;
static u8* _drive_io_callback_return_buffer=NULL;



static u64 _drive_io_callback(drive_t* drive,u64 offset,u64 buffer,u64 size){
	_drive_io_callback_drive=drive;
	_drive_io_callback_offset=offset;
	_drive_io_callback_buffer=buffer;
	_drive_io_callback_size=size;
	if (_drive_io_callback_return_buffer){
		if (offset&DRIVE_OFFSET_FLAG_WRITE){
			memcpy(_drive_io_callback_return_buffer,(const void*)(buffer+VMM_HIGHER_HALF_ADDRESS_OFFSET),size<<drive->block_size_shift);
		}
		else{
			memcpy((void*)(buffer+VMM_HIGHER_HALF_ADDRESS_OFFSET),_drive_io_callback_return_buffer,size<<drive->block_size_shift);
		}
	}
	return _drive_io_callback_return;
}



static const drive_type_t _test_drive_type={
	"test-drive-type",
	0,
	_drive_io_callback
};



static const drive_type_t _test_drive_type_readonly={
	"test-drive-type-readonly",
	DRIVE_TYPE_FLAG_READ_ONLY,
	_drive_io_callback
};



void test_drive(void){
	TEST_MODULE("drive");
	TEST_FUNC("drive_create");
	TEST_GROUP("correct args");
	drive_config_t config={
		&_test_drive_type,
		1,
		2,
		smm_alloc("serial_number",0),
		smm_alloc("model_number",0),
		4,
		512,
		(void*)6
	};
	drive_t* drive=drive_create(&config);
	TEST_ASSERT(drive);
	TEST_ASSERT(drive->type==&_test_drive_type);
	TEST_ASSERT(drive->block_size_shift==__builtin_ctzll(config.block_size));
	TEST_ASSERT(drive->controller_index==1);
	TEST_ASSERT(drive->device_index==2);
	TEST_ASSERT(drive->serial_number==config.serial_number);
	TEST_ASSERT(drive->model_number==config.model_number);
	TEST_ASSERT(drive->block_count==config.block_count);
	TEST_ASSERT(drive->block_size==config.block_size);
	TEST_ASSERT(drive->extra_data==config.extra_data);
	TEST_ASSERT(!drive->partition_table_descriptor);
	TEST_FUNC("drive_read");
	TEST_GROUP("empty buffer");
	TEST_ASSERT(!drive_read(drive,0,NULL,0));
	TEST_GROUP("correct args");
	u8 buffer[512];
	u8 buffer2[512];
	random_generate(buffer,sizeof(buffer));
	memset(buffer2,0,sizeof(buffer2));
	_drive_io_callback_return=1;
	_drive_io_callback_return_buffer=buffer;
	TEST_ASSERT(drive_read(drive,0,buffer2,1)==1);
	TEST_ASSERT(_drive_io_callback_drive==drive);
	TEST_ASSERT(!_drive_io_callback_offset);
	TEST_ASSERT(!(_drive_io_callback_buffer&(PAGE_SIZE-1)));
	TEST_ASSERT(_drive_io_callback_size==1);
	for (u32 i=0;i<512;i++){
		TEST_ASSERT(buffer[i]==buffer2[i]);
	}
	TEST_FUNC("drive_write");
	TEST_GROUP("empty buffer");
	TEST_ASSERT(!drive_write(drive,0,NULL,0));
	TEST_GROUP("read-only drive");
	drive->type=&_test_drive_type_readonly;
	TEST_ASSERT(!drive_write(drive,0,buffer2,1));
	TEST_GROUP("correct args");
	memset(buffer,0,sizeof(buffer));
	random_generate(buffer2,sizeof(buffer2));
	drive->type=&_test_drive_type;
	TEST_ASSERT(drive_write(drive,0,buffer2,1)==1);
	TEST_ASSERT(_drive_io_callback_drive==drive);
	TEST_ASSERT(_drive_io_callback_offset==DRIVE_OFFSET_FLAG_WRITE);
	TEST_ASSERT(!(_drive_io_callback_buffer&(PAGE_SIZE-1)));
	TEST_ASSERT(_drive_io_callback_size==1);
	for (u32 i=0;i<512;i++){
		TEST_ASSERT(buffer[i]==buffer2[i]);
	}
	_drive_io_callback_return=0;
	_drive_io_callback_return_buffer=NULL;
	handle_release(&(drive->handle));
}
