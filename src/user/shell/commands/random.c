#include <command.h>
#include <string.h>
#include <sys/fd.h>
#include <sys/io.h>
#include <sys/types.h>



#define COLUMNS 16



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



void random_main(int argc,const char*const* argv){
	if (argc<2){
		printf("random: no size supplied\n");
		return;
	}
	if (argc>2){
		printf("random: unrecognized option '%s'\n",argv[2]);
		return;
	}
	u64 size;
	if (!_parse_number(argv[1],&size)){
		printf("random: '%s' is not a valid size\n",argv[2]);
		return;
	}
	s64 fd=fd_open(0,"/dev/random",FD_FLAG_READ);
	if (fd<0){
		printf("random: unable to open random file\n");
		return;
	}
	u8 buffer[512];
	u32 i=0;
	while (size){
		u64 count=(size>512?512:size);
		fd_read(fd,buffer,count,0);
		size-=count;
		for (u32 j=0;j<count;j++){
			if (i>=COLUMNS){
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
	fd_close(fd);
}



DECLARE_COMMAND(random,"random <size>");
