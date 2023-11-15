#include <command.h>
#include <cwd.h>
#include <sys/fd.h>
#include <sys/io.h>
#include <sys/types.h>



static _Bool _parse_number(const char* str,u64* out){
	u64 value=0;
	if (!str[0]){
		return 0;
	}
	if (str[0]=='0'&&str[1]=='x'){
		str+=2;
		if (!str[0]){
			return 0;
		}
		while (str[0]){
			u8 c=str[0];
			str++;
			value<<=4;
			if (c>47&&c<58){
				value|=c-48;
			}
			else if (c>64&&c<91){
				value|=c-55;
			}
			else if (c>96&&c<123){
				value|=c-87;
			}
			else{
				return 0;
			}
		}
	}
	else{
		while (str[0]){
			u8 c=str[0];
			str++;
			value*=10;
			if (c>47&&c<58){
				value+=c-48;
			}
			else{
				return 0;
			}
		}
	}
	*out=value;
	return 1;
}



void sz_main(int argc,const char*const* argv){
	if (argc<2){
		printf("sz: no input file supplied\n");
		return;
	}
	if (argc>3){
		printf("sz: unrecognized option '%s'\n",argv[3]);
		return;
	}
	s64 fd=fd_open(cwd_fd,argv[1],0);
	if (fd<0){
		printf("sz: unable to open file '%s': error %d\n",argv[1],fd);
		return;
	}
	if (argc==3){
		u64 size;
		if (!_parse_number(argv[2],&size)){
			printf("sz: '%s' is not a valid size\n",argv[2]);
			goto _cleanup;
		}
		int error=fd_resize(fd,size,0);
		if (error<0){
			printf("sz: unable to resize '%s' to '%s': error %d\n",argv[1],argv[2],error);
			goto _cleanup;
		}
	}
	printf("%lu\n",fd_seek(fd,0,FD_SEEK_END));
_cleanup:
	fd_close(fd);
}



DECLARE_COMMAND(sz,"sz <file> [<size>]");
