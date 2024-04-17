#include <common/kfs2/api.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>



static void* _drive_data=NULL;
static uint64_t _drive_block_size=0;



static uint64_t _read_callback(void* ctx,uint64_t offset,void* buffer,uint64_t size){
	memcpy(buffer,_drive_data+offset*_drive_block_size,size*_drive_block_size);
	return size;
}



static uint64_t _write_callback(void* ctx,uint64_t offset,const void* buffer,uint64_t size){
	memcpy(_drive_data+offset*_drive_block_size,buffer,size*_drive_block_size);
	return size;
}



static void* _alloc_callback(uint64_t count){
	return malloc(count<<12);
}



static void _dealloc_callback(void* ptr,uint64_t count){
	free(ptr);
}



int main(int argc,const char** argv){
	if (argc<4){
		printf("Usage:\n\n%s <drive file> <block_size>:<start_lba>:<end_lba> <command> [...arguments]\n",(argc?argv[0]:"kfs2"));
		return 1;
	}
	int fd=open(argv[1],O_RDWR,0);
	if (fd<0){
		printf("Unable to open drive file '%s'\n",argv[1]);
		return 1;
	}
	uint64_t drive_size=lseek(fd,0,SEEK_END);
	lseek(fd,0,SEEK_SET);
	uint64_t start_lba;
	uint64_t end_lba;
	if (sscanf(argv[2],"%lu:%lu:%lu",&_drive_block_size,&start_lba,&end_lba)<0||(_drive_block_size&(_drive_block_size-1))||end_lba<=start_lba||end_lba*_drive_block_size>drive_size){
		printf("Invalid partition description '%s'\n",argv[2]);
		return 1;
	}
	_drive_data=mmap(0,drive_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	close(fd);
	int ret=1;
	const kfs2_filesystem_config_t fs_config={
		NULL,
		(kfs2_filesystem_block_read_callback_t)_read_callback,
		(kfs2_filesystem_block_write_callback_t)_write_callback,
		(kfs2_filesystem_page_alloc_callback_t)_alloc_callback,
		(kfs2_filesystem_page_dealloc_callback_t)_dealloc_callback,
		_drive_block_size,
		start_lba,
		end_lba
	};
	if (!strcmp(argv[3],"format")){
		if (!kfs2_filesystem_format(&fs_config)){
			printf("Unable to format partition as KFS2\n");
		}
		goto _cleanup_without_fs;
	}
	kfs2_filesystem_t fs;
	if (!kfs2_filesystem_init(&fs_config,&fs)){
		printf("KFS2 corrupted\n");
		goto _cleanup_without_fs;
	}
	if (!strcmp(argv[3],"test-command")){
		//
	}
	else{
		printf("Unknown command '%s'\n",argv[3]);
		goto _cleanup;
	}
	ret=0;
_cleanup:
	kfs2_filesystem_deinit(&fs);
_cleanup_without_fs:
	munmap(_drive_data,drive_size);
	return ret;
}
