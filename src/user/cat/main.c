#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/heap/heap.h>
#include <sys/io/io.h>
#include <sys/types.h>
#include <sys/util/options.h>



int main(int argc,const char** argv){
	const char** files=NULL;
	u32 file_count=0;
	if (!sys_options_parse_NEW(argc,argv,"{:f:file}!*s",&files,&file_count)){
		return 1;
	}
	int ret=0;
	for (u32 i=0;i<file_count;i++){
		sys_fd_t fd=(files[i][0]=='-'&&!files[i][1]?sys_fd_dup(SYS_FD_DUP_STDIN,SYS_FD_FLAG_READ):sys_fd_open(0,files[i],SYS_FD_FLAG_READ));
		if (SYS_IS_ERROR(fd)){
			sys_io_print("cat: unable to open file '%s': error %lld\n",files[i],fd);
			ret=1;
			goto _cleanup;
		}
		char buffer[512];
		while (1){
			u64 length=sys_fd_read(fd,buffer,512,0);
			if (SYS_IS_ERROR(length)){
				sys_io_print("cat: unable to read from file '%s': error %lld\n",files[i],length);
				sys_fd_close(fd);
				ret=1;
				goto _cleanup;
			}
			if (!length){
				break;
			}
			if (sys_fd_write(sys_io_output_fd,buffer,length,0)!=length){
				sys_io_print("cat: unable to write to output file\n");
				sys_fd_close(fd);
				ret=1;
				goto _cleanup;
			}
		}
		sys_fd_close(fd);
		sys_io_print("\n");
	}
_cleanup:
	sys_heap_dealloc(NULL,files);
	return ret;
}
