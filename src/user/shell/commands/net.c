#include <command.h>
#include <string.h>
#include <user/clock.h>
#include <user/io.h>
#include <user/network.h>



#define FLAG_REFRESH 1
#define FLAG_LIST 2



void net_main(int argc,const char*const* argv){
	u8 flags=0;
	for (u32 i=1;i<argc;i++){
		if (string_equal(argv[i],"-r")){
			flags|=FLAG_REFRESH;
		}
		else if (string_equal(argv[i],"-l")){
			flags|=FLAG_LIST;
		}
		else{
			printf("net: unrecognized option '%s'\n",argv[i]);
			return;
		}
	}
	if (flags&FLAG_REFRESH){
		network_refresh_device_list();
	}
	if (flags&FLAG_LIST){
		u32 count=network_device_count();
		for (u32 i=0;i<count;i++){
			network_device_t device;
			if (!network_device_get(i,&device)){
				continue;
			}
			printf("Device #%u:\n  MAC: \x1b[1m%X:%X:%X:%X:%X:%X\x1b[0m\n  UUID: \x1b[1m%X%X%X%X-%X%X-%X%X-%X%X-%X%X%X%X%X%X\x1b[0m\n  SN: \x1b[1m%s\x1b[0m\n  Ping duration: \x1b[1m%lu\x1b[0mms\n  Ping time: \x1b[1m%lu\x1b[0ms ago\n",
				i,
				device.address[0],device.address[1],device.address[2],device.address[3],device.address[4],device.address[5],
				device.uuid[0],device.uuid[1],device.uuid[2],device.uuid[3],
				device.uuid[4],device.uuid[5],
				device.uuid[6],device.uuid[7],
				device.uuid[8],device.uuid[9],
				device.uuid[10],device.uuid[11],device.uuid[12],device.uuid[13],device.uuid[14],device.uuid[15],
				device.serial_number,
				(device.ping+500000)/1000000,
				(clock_get_time()-device.last_ping_time+500000000)/1000000000
			);
		}
		return;
	}
	network_config_t config;
	if (!network_config(&config)){
		printf("net: unable to read network configuration\n");
		return;
	}
	printf("Name: \x1b[1m%s\x1b[0m\nAddress: \x1b[1m%X:%X:%X:%X:%X:%X\x1b[0m\n",
		config.name,
		config.address[0],config.address[1],config.address[2],config.address[3],config.address[4],config.address[5]
	);
}



DECLARE_COMMAND(net,"net [-r] [-l]");
