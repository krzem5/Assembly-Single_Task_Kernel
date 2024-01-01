#include <sys2/error/error.h>
#include <sys2/fd/fd.h>
#include <sys2/io/io.h>
#include <sys2/types.h>
#include <sys2/util/options.h>



#define COLUMNS 16



int main(int argc,const char** argv){
	u32 i=sys2_options_parse(argc,argv,NULL);
	if (!i){
		return 1;
	}
	for (;i<argc;i++){
		sys2_fd_t fd=sys2_fd_open(0,(argv[i][0]=='-'&&!argv[i][1]?"/proc/self/stdin":argv[i]),SYS2_FD_FLAG_READ);
		if (SYS2_IS_ERROR(fd)){
			sys2_io_print("hexdump: unable to open file '%s': error %d\n",argv[i],fd);
			return 1;
		}
		char buffer[512];
		while (1){
			u64 length=sys2_fd_read(fd,buffer,512,0);
			if (SYS2_IS_ERROR(length)){
				sys2_io_print("hexdump: unable to read from file '%s': error %d\n",argv[i],length);
				sys2_fd_close(fd);
				return 1;
			}
			if (!length){
				break;
			}
			u32 k=0;
			for (u32 j=0;j<length;j++){
				if (k>=COLUMNS){
					k=0;
					sys2_io_print("\n");
				}
				else if (k){
					sys2_io_print(" ");
				}
				k++;
				sys2_io_print("%X",buffer[j]);
			}
		}
		sys2_fd_close(fd);
		sys2_io_print("\n");
	}
	return 0;
}
