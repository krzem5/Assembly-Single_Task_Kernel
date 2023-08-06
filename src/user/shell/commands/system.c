#include <command.h>
#include <string.h>
#include <user/io.h>
#include <user/system.h>



static const char* _system_wakeup_type_name[]={
	[SYSTEM_WAKEUP_TYPE_UNKNOWN]="Unknown",
	[SYSTEM_WAKEUP_TYPE_POWER_SWITCH]="Power switch",
	[SYSTEM_WAKEUP_TYPE_AC_POWER]="AC power supply"
};



void system_main(int argc,const char*const* argv){
	if (argc>1){
		printf("system: unrecognized option '%s'\n",argv[1]);
		return;
	}
	printf("BIOS vendor: \x1b[1m%s\x1b[0m\n",system_data.bios_vendor);
	printf("BIOS version: \x1b[1m%s\x1b[0m\n",system_data.bios_version);
	printf("Manufacturer: \x1b[1m%s\x1b[0m\n",system_data.manufacturer);
	printf("Product: \x1b[1m%s\x1b[0m\n",system_data.product);
	printf("Version: \x1b[1m%s\x1b[0m\n",system_data.version);
	printf("Serial number: \x1b[1m%s\x1b[0m\n",system_data.serial_number);
	printf("UUID: \x1b[1m%X%X%X%X-%X%X-%X%X-%X%X-%X%X%X%X%X%X\x1b[0m\n",
		system_data.uuid[0],
		system_data.uuid[1],
		system_data.uuid[2],
		system_data.uuid[3],
		system_data.uuid[4],
		system_data.uuid[5],
		system_data.uuid[6],
		system_data.uuid[7],
		system_data.uuid[8],
		system_data.uuid[9],
		system_data.uuid[10],
		system_data.uuid[11],
		system_data.uuid[12],
		system_data.uuid[13],
		system_data.uuid[14],
		system_data.uuid[15]
	);
	printf("Last wakeup: \x1b[1m%s\x1b[0m\n",_system_wakeup_type_name[system_data.wakeup_type]);
}



DECLARE_COMMAND(system,"system");
