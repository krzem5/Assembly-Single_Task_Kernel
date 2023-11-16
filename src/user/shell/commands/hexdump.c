#include <command.h>
#include <cwd.h>
#include <string.h>
#include <sys/fd.h>
#include <sys/io.h>
#include <sys/types.h>



void hexdump_main(int argc,const char*const* argv){
	u32 columns=16;
	const char* file=NULL;
	for (u32 i=1;i<argc;i++){
		if (i<argc-1&&string_equal(argv[i],"-c")){
			i++;
			columns=0;
			for (const char* column_count=argv[i];*column_count;column_count++){
				if (*column_count<48||*column_count>57){
					printf("hexdump: invalid column count '%s'\n",argv[i]);
					return;
				}
				columns=columns*10+(*column_count)-48;
			}
		}
		else if (argv[i][0]!='-'&&!file){
			file=argv[i];
		}
		else{
			printf("hexdump: unrecognized option '%s'\n",argv[i]);
			return;
		}
	}
	if (!file){
		printf("hexdump: no file supplied\n");
		return;
	}
	s64 fd=sys_fd_open(cwd_fd,file,FD_FLAG_READ);
	if (fd<0){
		printf("hexdump: unable to open file '%s': error %d\n",file,fd);
		return;
	}
	u8 buffer[512];
	u32 i=0;
	while (1){
		s64 length=sys_fd_read(fd,buffer,512,0);
		if (length<0){
			printf("hexdump: unable to read from file '%s': error %d\n",file,length);
			goto _cleanup;
		}
		if (!length){
			break;
		}
		for (u32 j=0;j<length;j++){
			if (i>=columns){
				i=0;
				putchar('\n');
			}
			else if (i){
				putchar(' ');
			}
			i++;
			u8 byte=buffer[j];
			printf("%c%c",(byte>>4)+((byte>>4)>9?87:48),(byte&0xf)+((byte&0xf)>9?87:48));
		}
	}
	putchar('\n');
_cleanup:
	sys_fd_close(fd);
}



DECLARE_COMMAND(hexdump,"hexdump [-c <columns>] <file>");
