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
		system_bios_data->bios_vendor,
		system_bios_data->bios_version,
		system_bios_data->manufacturer,
		system_bios_data->product,
		system_bios_data->version,
		system_bios_data->serial_number,
		system_bios_data->uuid[0],system_bios_data->uuid[1],system_bios_data->uuid[2],system_bios_data->uuid[3],
		system_bios_data->uuid[4],system_bios_data->uuid[5],
		system_bios_data->uuid[6],system_bios_data->uuid[7],
		system_bios_data->uuid[8],system_bios_data->uuid[9],
		system_bios_data->uuid[10],system_bios_data->uuid[11],system_bios_data->uuid[12],system_bios_data->uuid[13],system_bios_data->uuid[14],system_bios_data->uuid[15],
		_system_wakeup_type_name[system_bios_data->wakeup_type]
	);
}



DECLARE_COMMAND(system,"system");
