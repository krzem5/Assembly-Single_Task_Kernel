#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/io/io.h>
#include <sys/types.h>
#include <sys/util/options.h>
#include <sys/signal/signal.h>



static void _signal_handler(sys_signal_handler_context_t* ctx){
	sys_io_print("Signal %u [rip=%p rflags=%p rax=%p]\n",ctx->signal,ctx->rip,ctx->rflags,ctx->return_code);
}



int main(int argc,const char** argv){
	sys_signal_set_handler(_signal_handler);
	u32 i=sys_options_parse(argc,argv,NULL);
	if (!i){
		return 1;
	}
	for (;i<argc;i++){
		sys_fd_t fd=(argv[i][0]=='-'&&!argv[i][1]?sys_fd_dup(SYS_FD_DUP_STDIN,SYS_FD_FLAG_READ):sys_fd_open(0,argv[i],SYS_FD_FLAG_READ));
		if (SYS_IS_ERROR(fd)){
			sys_io_print("cat: unable to open file '%s': error %lld\n",argv[i],fd);
			return 1;
		}
		char buffer[512];
		while (1){
			u64 length=sys_fd_read(fd,buffer,512,0);
			if (SYS_IS_ERROR(length)){
				sys_io_print("cat: unable to read from file '%s': error %lld\n",argv[i],length);
				sys_fd_close(fd);
				return 1;
			}
			if (!length){
				break;
			}
			if (sys_fd_write(sys_io_output_fd,buffer,length,0)!=length){
				sys_io_print("cat: unable to write to output file\n");
				sys_fd_close(fd);
				return 1;
			}
		}
		sys_fd_close(fd);
		sys_io_print("\n");
	}
	return 0;
}
