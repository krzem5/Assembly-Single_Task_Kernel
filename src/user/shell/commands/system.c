#include <command.h>
#include <core/io.h>
#include <core/system.h>



#define BUFFER_SIZE 256



static void _get_string(u32 index,char* buffer){
	buffer[0]='<';
	buffer[1]='U';
	buffer[2]='n';
	buffer[3]='k';
	buffer[4]='n';
	buffer[5]='o';
	buffer[6]='w';
	buffer[7]='n';
	buffer[8]='>';
	buffer[9]=0;
}



void system_main(int argc,const char*const* argv){
	if (argc>1){
		printf("system: unrecognized option '%s'\n",argv[1]);
		return;
	}
	char buffer[BUFFER_SIZE];
	_get_string(0,buffer);
	printf("BIOS vendor: \x1b[1m%s\x1b[0m\n",buffer);
	_get_string(0,buffer);
	printf("BIOS version: \x1b[1m%s\x1b[0m\n",buffer);
	_get_string(0,buffer);
	printf("Manufacturer: \x1b[1m%s\x1b[0m\n",buffer);
	_get_string(0,buffer);
	printf("Product: \x1b[1m%s\x1b[0m\n",buffer);
	_get_string(0,buffer);
	printf("Version: \x1b[1m%s\x1b[0m\n",buffer);
	_get_string(0,buffer);
	printf("Serial number: \x1b[1m%s\x1b[0m\n",buffer);
	_get_string(0,buffer);
	printf("UUID: \x1b[1m%s\x1b[0m\n",buffer);
	_get_string(0,buffer);
	printf("Last wakeup: \x1b[1m%s\x1b[0m\n",buffer);
}



DECLARE_COMMAND(system,"system");
