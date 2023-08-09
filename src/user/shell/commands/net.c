#include <command.h>
#include <string.h>
#include <user/clock.h>
#include <user/io.h>
#include <user/network.h>



#define FLAG_REFRESH 1
#define FLAG_INFO 2



static _Bool _parse_address(const char* str,u8* out){
	for (u8 i=0;i<12;i++){
		u8 c=*str;
		str++;
		if (i&&!(i&1)){
			if (c!=':'){
				return 0;
			}
			c=*str;
			str++;
		}
		if (i&1){
			out[i>>1]<<=4;
		}
		else{
			out[i>>1]=0;
		}
		if (c>47&&c<58){
			out[i>>1]|=c-48;
		}
		else if (c>64&&c<91){
			out[i>>1]|=c-55;
		}
		else if (c>96&&c<123){
			out[i>>1]|=c-87;
		}
		else{
			return 0;
		}
	}
	return !*str;
}



void net_main(int argc,const char*const* argv){
	u8 flags=0;
	const char* address_to_delete=NULL;
	for (u32 i=1;i<argc;i++){
		if (i<argc-1&&string_equal(argv[i],"-d")){
			i++;
			address_to_delete=argv[i];
		}
		else if (string_equal(argv[i],"-i")){
			flags|=FLAG_INFO;
		}
		else if (string_equal(argv[i],"-r")){
			flags|=FLAG_REFRESH;
		}
		else{
			printf("net: unrecognized option '%s'\n",argv[i]);
			return;
		}
	}
	if (address_to_delete){
		u8 address[6];
		if (!_parse_address(address_to_delete,address)){
			printf("net: invalid MAC address '%s'\n",address_to_delete);
			return;
		}
		if (!network_device_delete(address)){
			printf("net: MAC address '%s' not found\n",address_to_delete);
			return;
		}
	}
	if (flags&FLAG_REFRESH){
		network_refresh_device_list();
	}
	if (flags&FLAG_INFO){
		network_config_t config;
		if (!network_config(&config)){
			printf("net: unable to read network configuration\n");
			return;
		}
		printf("Driver: \x1b[1m%s\x1b[0m\nAddress: \x1b[1m%X:%X:%X:%X:%X:%X\x1b[0m\n",
			config.name,
			config.address[0],config.address[1],config.address[2],config.address[3],config.address[4],config.address[5]
		);
		return;
	}
	u32 count=network_device_count();
	for (u32 i=0;i<count;i++){
		network_device_t device;
		if (!network_device_get(i,&device)){
			continue;
		}
		printf("Device #%u:\n  Flags:\x1b[1m%s%s\x1b[0m\n  MAC: \x1b[1m%X:%X:%X:%X:%X:%X\x1b[0m\n  UUID: \x1b[1m%X%X%X%X-%X%X-%X%X-%X%X-%X%X%X%X%X%X\x1b[0m\n  SN: \x1b[1m%s\x1b[0m\n  Ping: \x1b[1m%lu\x1b[0mms (\x1b[1m%lu\x1b[0ms ago)\n",
			i,
			((device.flags&NETWORK_DEVICE_FLAG_ONLINE)?" Online":""),
			((device.flags&NETWORK_DEVICE_FLAG_LOG_TARGET)?" Log target":""),
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
}



DECLARE_COMMAND(net,"net [-d <address>] [-i] [-r]");
