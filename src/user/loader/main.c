#include <user/drive.h>
#include <user/elf.h>
#include <user/fs.h>
#include <user/partition.h>
#include <user/print.h>
#include <user/types.h>



static const char* partition_type_names[]={
	[PARTITION_TYPE_EMPTY_DRIVE]="empty drive",
	[PARTITION_TYPE_EMPTY]="empty",
	[PARTITION_TYPE_ISO9660]="iso9660",
	[PARTITION_TYPE_GPT]="gpt"
};



static void _format_drive_as_gpt(const partition_t* partition){
	const drive_t* drive=drives+partition->drive_index;
	if (drive->block_size!=512){
		printf("Unable to format drive as GPT, drive->block_size!=512\n");
		return;
	}
	char path[32];
	u8 i=0;
	while (partition->name[i]){
		path[i]=partition->name[i];
		i++;
	}
	path[i]=':';
	path[i+1]='/';
	path[i+2]=0;
	int fd=fs_open(path,FS_FLAG_READ|FS_FLAG_WRITE);
	u8 buffer[512];
	for (u16 i=0;i<512;i++){
		buffer[i]=0;
	}
	buffer[0x1be]=0x00;
	buffer[0x1bf]=0x00;
	buffer[0x1c0]=0x02;
	buffer[0x1c1]=0x00;
	buffer[0x1c2]=0xee;
	buffer[0x1c3]=0xff;
	buffer[0x1c4]=0xff;
	buffer[0x1c5]=0xff;
	buffer[0x1c6]=0x01;
	buffer[0x1c7]=0x00;
	buffer[0x1c8]=0x00;
	buffer[0x1c9]=0x00;
	buffer[0x1ca]=0xff;
	buffer[0x1cb]=0xff;
	buffer[0x1cc]=0xff;
	buffer[0x1cd]=0xff;
	if (fs_write(fd,buffer,512)!=512){
		printf("Error writing MBR\n");
		goto _cleanup;
	}
_cleanup:
	fs_close(fd);
}



void main(void){
	drive_init();
	partition_init();
	if (drives[drive_boot_index].type==DRIVE_TYPE_ATA||drives[drive_boot_index].type==DRIVE_TYPE_ATAPI){
		printf("Booting from an installable medium!\n");
	}
	printf("Drives:\n");
	for (u32 i=0;i<drive_count;i++){
		printf("[%u]: %s (%s)%s\n",i,drives[i].name,drives[i].model_number,((drives[i].flags&DRIVE_FLAG_BOOT)?" [boot]":""));
	}
	printf("Partitions:\n");
	for (u32 i=0;i<partition_count;i++){
		printf("[%u]: %s -> %s%s\n",i,partitions[i].name,partition_type_names[partitions[i].type],((partitions[i].flags&PARTITION_FLAG_BOOT)?" [boot]":""));
	}
	for (u32 i=0;i<partition_count;i++){
		if (partitions[i].type==PARTITION_TYPE_EMPTY_DRIVE){
			_format_drive_as_gpt(partitions+i);
		}
	}
	printf("\x1b[38;2;169;42;187mHello world!\x1b[0m\n");
	elf_load("/abc.elf");
}
