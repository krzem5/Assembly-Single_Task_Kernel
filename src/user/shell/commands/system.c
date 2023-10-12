#include <command.h>
#include <user/io.h>
#include <user/system.h>



#define BUFFER_SIZE 256



static void _get_string(u32 index,char* buffer){
	if (system_get_string(index,buffer,BUFFER_SIZE)){
		return;
	}
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
	_get_string(SYSTEM_STRING_BIOS_VENDOR,buffer);
	printf("BIOS vendor: \x1b[1m%s\x1b[0m\n",buffer);
	_get_string(SYSTEM_STRING_BIOS_VERSION,buffer);
	printf("BIOS version: \x1b[1m%s\x1b[0m\n",buffer);
	_get_string(SYSTEM_STRING_MANUFACTURER,buffer);
	printf("Manufacturer: \x1b[1m%s\x1b[0m\n",buffer);
	_get_string(SYSTEM_STRING_PRODUCT,buffer);
	printf("Product: \x1b[1m%s\x1b[0m\n",buffer);
	_get_string(SYSTEM_STRING_VERSION,buffer);
	printf("Version: \x1b[1m%s\x1b[0m\n",buffer);
	_get_string(SYSTEM_STRING_SERIAL_NUMBER,buffer);
	printf("Serial number: \x1b[1m%s\x1b[0m\n",buffer);
	_get_string(SYSTEM_STRING_UUID,buffer);
	printf("UUID: \x1b[1m%s\x1b[0m\n",buffer);
	_get_string(SYSTEM_STRING_LAST_WAKEUP,buffer);
	printf("Last wakeup: \x1b[1m%s\x1b[0m\n",buffer);
}



DECLARE_COMMAND(system,"system");
