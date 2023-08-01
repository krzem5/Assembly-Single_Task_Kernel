#include <command.h>
#include <string.h>
#include <user/clock.h>
#include <user/drive.h>
#include <user/fs.h>
#include <user/io.h>
#include <user/memory.h>
#include <user/types.h>



// Must be page-aligned
#define SPEED_TEST_BUFFER_SIZE (65536*4096)

#define SPEED_TEST_COUNT 2



static const char* drive_type_names[]={
	[DRIVE_TYPE_AHCI]="AHCI",
	[DRIVE_TYPE_ATA]="ATA",
	[DRIVE_TYPE_ATAPI]="ATAPI",
	[DRIVE_TYPE_NVME]="NVMe"
};



void drive_main(int argc,const char*const* argv){
	_Bool speed_test=0;
	const char* drive_name=NULL;
	for (u32 i=1;i<argc;i++){
		if (string_equal(argv[i],"-s")){
			speed_test=1;
		}
		else if (argv[i][0]!='-'&&!drive_name){
			drive_name=argv[i];
		}
		else{
			printf("drive: unrecognized option '%s'\n",argv[i]);
			return;
		}
	}
	if (!drive_name){
		printf("drive: no drive supplied\n");
		return;
	}
	u32 i=0;
	for (;i<drive_count;i++){
		if (string_equal((drives+i)->name,drive_name)){
			break;
		}
	}
	if (i==drive_count){
		printf("drive: drive '%s' not found\n",drive_name);
		return;
	}
	drive_stats_t stats;
	if (!drive_get_stats(i,&stats)){
		printf("drive: unable to get drive stats\n");
		return;
	}
	const drive_t* drive=drives+i;
	printf("Name: \x1b[1m%s\x1b[0m\nType: \x1b[1m%s\x1b[0m\nSize: \x1b[1m%v\x1b[0m\nBlock size: \x1b[1m%v\x1b[0m\nBlock count: \x1b[1m%lu\x1b[0m\n",
		drive_name,
		drive_type_names[drive->type],
		drive->block_count*drive->block_size,
		drive->block_size,
		drive->block_count
	);
	if (stats.root_block_count){
		printf("KFS blocks:\n  ROOT: \x1b[1m%v\x1b[0m\n  BATC: \x1b[1m%v\x1b[0m\n  NDA3: \x1b[1m%v\x1b[0m\n  NDA2: \x1b[1m%v\x1b[0m\n  NDA1: \x1b[1m%v\x1b[0m\n  NFDA: \x1b[1m%v\x1b[0m\n  DATA: \x1b[1m%v\x1b[0m\n",
			stats.root_block_count*drive->block_size,
			stats.batc_block_count*drive->block_size,
			stats.nda3_block_count*drive->block_size,
			stats.nda2_block_count*drive->block_size,
			stats.nda1_block_count*drive->block_size,
			stats.nfda_block_count*drive->block_size,
			stats.data_block_count*drive->block_size
		);
	}
	if (!speed_test){
		return;
	}
	char path[64];
	i=0;
	while (drive_name[i]){
		path[i]=drive_name[i];
		i++;
	}
	path[i]='p';
	path[i+1]='0';
	path[i+2]=':';
	path[i+3]='t';
	path[i+4]='m';
	path[i+5]='p';
	path[i+6]=0;
	int dst_fd=fs_open(0,path,0);
	if (dst_fd>=0){
		fs_close(dst_fd);
		printf("drive: file '%s' already exists\n",path);
		return;
	}
	dst_fd=fs_open(0,path,FS_FLAG_READ|FS_FLAG_WRITE|FS_FLAG_CREATE);
	void* buffer=memory_map(SPEED_TEST_BUFFER_SIZE,MEMORY_FLAG_LARGE);
	u64 read_speed=0;
	u64 write_speed=0;
	for (u8 i=0;i<SPEED_TEST_COUNT;i++){
		fs_seek(dst_fd,0,FS_SEEK_SET);
		u64 start_write=clock_get_ticks();
		s64 write=fs_write(dst_fd,buffer,SPEED_TEST_BUFFER_SIZE);
		u64 end_write=clock_get_ticks();
		fs_seek(dst_fd,0,FS_SEEK_SET);
		u64 start_read=clock_get_ticks();
		s64 read=fs_read(dst_fd,buffer,SPEED_TEST_BUFFER_SIZE);
		u64 end_read=clock_get_ticks();
		read_speed+=read*1000000000ull/clock_ticks_to_time(end_read-start_read);
		write_speed+=write*1000000000ull/clock_ticks_to_time(end_write-start_write);
		if (write!=SPEED_TEST_BUFFER_SIZE||read!=SPEED_TEST_BUFFER_SIZE){
			return;
		}
	}
	memory_unmap(buffer,SPEED_TEST_BUFFER_SIZE);
	fs_delete(dst_fd);
	printf("Speed:\n  Read: \x1b[1m%v/s\x1b[0m\n  Write: \x1b[1m%v/s\x1b[0m\n",read_speed/SPEED_TEST_COUNT,write_speed/SPEED_TEST_COUNT);
}



DECLARE_COMMAND(drive,"drive [-s] <drive>");
