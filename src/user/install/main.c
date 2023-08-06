#include <user/drive.h>
#include <user/elf.h>
#include <user/fs.h>
#include <user/io.h>
#include <user/memory.h>
#include <user/partition.h>
#include <user/system.h>
#include <user/types.h>



#define BUFFER_SIZE 4096



#define BOOT_CODE_FILE_NAME "/core.bin"



static void _copy_data(int src_fd,int dst_fd,u32 offset,u32 length){
	fs_seek(src_fd,offset,FS_SEEK_SET);
	fs_seek(dst_fd,offset,FS_SEEK_SET);
	u8 buffer[BUFFER_SIZE];
	while (length){
		s64 read=fs_read(src_fd,buffer,(length>BUFFER_SIZE?BUFFER_SIZE:length));
		if (read<=0){
			break;
		}
		s64 write=fs_write(dst_fd,buffer,read);
		if (write!=read){
			break;
		}
		length-=read;
	}
}



static _Bool _copy_file(const char* name,char* path,u32 offset){
	u32 i=0;
	for (;name[i];i++){
		path[i+offset]=name[i];
	}
	path[i+offset]=0;
	printf("Copying '%s' to '%s'...\n",name,path);
	int src_fd=fs_open(0,name,FS_FLAG_READ);
	if (src_fd<0){
		printf("Unable to open file '%s': error %d\n",name,src_fd);
		return 0;
	}
	int dst_fd=fs_open(0,path,FS_FLAG_WRITE|FS_FLAG_CREATE);
	if (dst_fd<0){
		fs_close(src_fd);
		printf("Unable to open file '%s': error %d\n",path,dst_fd);
		return 0;
	}
	_copy_data(src_fd,dst_fd,0,0xffffffff);
	fs_close(src_fd);
	fs_close(dst_fd);
	return 1;
}



static u32 _partition_name_to_path(char* path,const char* name){
	u32 out=0;
	for (;name[out];out++){
		path[out]=name[out];
	}
	path[out]=':';
	return out+1;
}



void main(void){
	char path[64];
	const partition_t* partition=partitions;
	for (u32 i=0;i<partition_count;i++){
		if ((partition->flags&PARTITION_FLAG_HALF_INSTALLED)&&partition->type==PARTITION_TYPE_KFS){
			goto _copy_kernel_files;
		}
		if (partition->flags&PARTITION_FLAG_PREVIOUS_BOOT){
			goto _update_boot_version;
		}
		partition++;
	}
_start_shell:
	elf_load("/shell.elf");
	return;
_update_boot_version:
	path[_partition_name_to_path(path,(drives+partition->drive_index)->name)]=0;
	printf("Copying kernel core from '%s' to '%s'...\n",BOOT_CODE_FILE_NAME,path);
	int src_fd=fs_open(0,BOOT_CODE_FILE_NAME,FS_FLAG_READ);
	if (src_fd<0){
		printf("Unable to open file '%s': error %d\n",BOOT_CODE_FILE_NAME,src_fd);
		goto _start_shell;
	}
	int dst_fd=fs_open(0,path,FS_FLAG_WRITE);
	if (dst_fd<0){
		fs_close(src_fd);
		printf("Unable to open file '%s': error %d\n",path,dst_fd);
		goto _start_shell;
	}
	_copy_data(src_fd,dst_fd,0,512);
	_copy_data(src_fd,dst_fd,4608,0xffffffff);
	fs_close(src_fd);
	fs_close(dst_fd);
_copy_kernel_files:
	u32 offset=_partition_name_to_path(path,partition->name);
	_Bool out=_copy_file("/kernel.bin",path,offset);
	out&=_copy_file("/loader.elf",path,offset);
	out&=_copy_file("/shell.elf",path,offset);
	if (!out){
		goto _start_shell;
	}
	shutdown(SHUTDOWN_FLAG_RESTART);
}
