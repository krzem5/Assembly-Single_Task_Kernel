#include <sys/fd/fd.h>
#include <sys/heap/heap.h>
#include <sys/io/io.h>
#include <sys/types.h>
#include <sys/util/options.h>



int main(int argc,const char** argv){
	u64 bytes=0;
	u64 columns=16;
	u64 buffer_size=512;
	if (!sys_options_parse(argc,argv,"{:b:bytes}!+q{c:columns}+q{s:buffer-size}+q",&bytes,&columns,&buffer_size)){
		return 1;
	}
	sys_fd_t fd=sys_fd_open(0,"/dev/random",SYS_FD_FLAG_READ);
	if (SYS_IS_ERROR(fd)){
		sys_io_print("random: unable to open randomness source\n");
		return 1;
	}
	int ret=0;
	u8* buffer=sys_heap_alloc(NULL,buffer_size);
	u32 i=0;
	while (bytes){
		u64 count=(bytes>buffer_size?buffer_size:bytes);
		count=sys_fd_read(fd,buffer,count,0);
		if (SYS_IS_ERROR(count)){
			sys_fd_close(fd);
			sys_io_print("random: unable to read from file: error %d\n",count);
			ret=1;
			goto _cleanup;
		}
		bytes-=count;
		for (u64 j=0;j<count;j++){
			if (i>=columns){
				i=0;
				sys_io_print("\n");
			}
			else if (i){
				sys_io_print(" ");
			}
			i++;
			sys_io_print("%X",buffer[j]);
		}
	}
	sys_io_print("\n");
	sys_fd_close(fd);
_cleanup:
	sys_heap_dealloc(NULL,buffer);
	return ret;
}
