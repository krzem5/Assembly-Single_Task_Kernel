#include <command.h>
#include <string.h>
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
		return;
	}
	network_config_t config;
	if (!network_config(&config)){
		printf("net: unable to read network configuration\n");
		return;
	}
	printf("Name: \x1b[1m%s\x1b[0m\nAddress: \x1b[1m%X:%X:%X:%X:%X:%X\x1b[0m\n",config.name,config.address[0],config.address[1],config.address[2],config.address[3],config.address[4],config.address[5]);
}



DECLARE_COMMAND(net,"net [-r] [-l]");
