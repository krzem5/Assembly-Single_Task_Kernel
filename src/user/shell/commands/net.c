#include <command.h>
#include <sys/io.h>
// #include <sys/network.h>



void net_main(int argc,const char*const* argv){
	if (argc>1){
		printf("net: unrecognized option '%s'\n",argv[1]);
		return;
	}
	// u8 mac_address[6];
	// network_get_mac_address(mac_address);
	// printf("Address: \x1b[1m%X:%X:%X:%X:%X:%X\x1b[0m\n",
	// 	mac_address[0],
	// 	mac_address[1],
	// 	mac_address[2],
	// 	mac_address[3],
	// 	mac_address[4],
	// 	mac_address[5]
	// );
}



DECLARE_COMMAND(net,"net");
