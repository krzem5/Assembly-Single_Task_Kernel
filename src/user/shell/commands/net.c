#include <command.h>
#include <string.h>
#include <user/io.h>
#include <user/network.h>



void net_main(int argc,const char*const* argv){
	if (argc>1){
		printf("net: unrecognized option '%s'\n",argv[1]);
		return;
	}
	network_config_t config;
	if (!network_config(&config)){
		printf("net: unable to read network configuration\n");
		return;
	}
	printf("Name: \x1b[1m%s\x1b[0m\nAddress: \x1b[1m%X:%X:%X:%X:%X:%X\x1b[0m\n",config.name,config.address[0],config.address[1],config.address[2],config.address[3],config.address[4],config.address[5]);
}



DECLARE_COMMAND(net,"net");
