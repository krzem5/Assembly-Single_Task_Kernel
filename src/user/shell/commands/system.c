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
	printf("BIOS vendor: \x1b[1m%s\x1b[0m\nBIOS version: \x1b[1m%s\x1b[0m\nManufacturer: \x1b[1m%s\x1b[0m\nProduct: \x1b[1m%s\x1b[0m\nVersion: \x1b[1m%s\x1b[0m\nSerial number: \x1b[1m%s\x1b[0m\nUUID: \x1b[1m%X%X%X%X-%X%X-%X%X-%X%X-%X%X%X%X%X%X\x1b[0m\nLast wakeup: \x1b[1m%s\x1b[0m\n",
		bios_data->bios_vendor,
		bios_data->bios_version,
		bios_data->manufacturer,
		bios_data->product,
		bios_data->version,
		bios_data->serial_number,
		bios_data->uuid[0],bios_data->uuid[1],bios_data->uuid[2],bios_data->uuid[3],
		bios_data->uuid[4],bios_data->uuid[5],
		bios_data->uuid[6],bios_data->uuid[7],
		bios_data->uuid[8],bios_data->uuid[9],
		bios_data->uuid[10],bios_data->uuid[11],bios_data->uuid[12],bios_data->uuid[13],bios_data->uuid[14],bios_data->uuid[15],
		_system_wakeup_type_name[bios_data->wakeup_type]
	);
}



DECLARE_COMMAND(system,"system");
