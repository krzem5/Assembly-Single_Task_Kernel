#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/heap/heap.h>
#include <sys/io/io.h>
#include <sys/types.h>
#include <sys/util/options.h>



int main(int argc,const char** argv){
	u64 buffer_size=4096;
	u64 columns=16;
	const char** files=NULL;
	u32 file_count=0;
	if (!sys_options_parse_NEW(argc,argv,"{b:buffer-size}+q{c:columns}+q{:f:file}!*s",&buffer_size,&columns,&files,&file_count)){
		return 1;
	}
	int ret=0;
	u8* buffer=sys_heap_alloc(NULL,buffer_size);
	for (u32 i=0;i<file_count;i++){
		sys_fd_t fd=(files[i][0]=='-'&&!files[i][1]?sys_fd_dup(SYS_FD_DUP_STDIN,SYS_FD_FLAG_READ):sys_fd_open(0,files[i],SYS_FD_FLAG_READ));
		if (SYS_IS_ERROR(fd)){
			sys_io_print("hexdump: unable to open file '%s': error %d\n",files[i],fd);
			ret=1;
			goto _cleanup;
		}
		u64 k=0;
		while (1){
			u64 length=sys_fd_read(fd,buffer,buffer_size,0);
			if (SYS_IS_ERROR(length)){
				sys_io_print("hexdump: unable to read from file '%s': error %d\n",files[i],length);
				sys_fd_close(fd);
				ret=1;
				goto _cleanup;
			}
			if (!length){
				break;
			}
			for (u64 j=0;j<length;j++){
				if (k>=columns){
					k=0;
					sys_io_print("\n");
				}
				else if (k){
					sys_io_print(" ");
				}
				k++;
				sys_io_print("%X",buffer[j]);
			}
		}
		sys_fd_close(fd);
		sys_io_print("\n");
	}
_cleanup:
	sys_heap_dealloc(NULL,files);
	sys_heap_dealloc(NULL,buffer);
	return ret;
}
