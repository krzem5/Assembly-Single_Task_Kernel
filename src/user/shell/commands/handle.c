#include <command.h>
#include <string.h>
// #include <sys/handle.h>
#include <sys/io.h>



void handle_main(int argc,const char*const* argv){
	if (argc>1){
		printf("handle: unrecognized option '%s'\n",argv[1]);
		return;
	}
	// handle_iterator_t iterator;
	// HANDLE_FOREACH(&iterator,"handle"){
	// 	handle_data_t data;
	// 	if (!handle_get_data(iterator.handle,&data)){
	// 		continue;
	// 	}
	// 	printf("%s:\t\x1b[1m%u\x1b[0m\t(\x1b[1m%u\x1b[0m)\n",data.name,data.active_count,data.count);
	// }
}



DECLARE_COMMAND(handle,"handle");
