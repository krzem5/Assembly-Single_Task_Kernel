#include <sys/fd.h>
#include <sys/io.h>
#include <sys/options.h>



int main(int argc,const char** argv,const char** environ){
	u32 i=sys_options_parse(argc,argv,NULL);
	if (!i){
		return 1;
	}
	for (;i<argc;i++){
		s64 fd=sys_fd_open(0,(argv[i][0]=='-'&&!argv[i][1]?"/proc/self/stdin":argv[i]),SYS_FD_FLAG_READ);
		if (fd<0){
			printf("cat: unable to open file '%s': error %d\n",argv[i],fd);
			return 1;
		}
		char buffer[512];
		while (1){
			s64 length=sys_fd_read(fd,buffer,512,0);
			if (length<0){
				printf("cat: unable to read from file '%s': error %d\n",argv[i],length);
				sys_fd_close(fd);
				return 1;
			}
			if (!length){
				break;
			}
			print_buffer(buffer,length);
		}
		sys_fd_close(fd);
		putchar('\n');
	}
	return 0;
}
