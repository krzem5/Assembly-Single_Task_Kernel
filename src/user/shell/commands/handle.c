#include <command.h>
#include <string.h>
#include <user/handle.h>
#include <user/io.h>



void handle_main(int argc,const char*const* argv){
	if (argc>1){
		printf("handle: unrecognized option '%s'\n",argv[1]);
		return;
	}
	handle_iterator_t iterator;
	HANDLE_FOREACH(&iterator,"handle"){
		printf("%p\n",iterator.handle);
	}
// 		u8 i=0;
// 		for (;type_data.name[i];i++);
// 		printf("%s:\t%s\x1b[1m%u\x1b[0m\t(\x1b[1m%u\x1b[0m)\n",type_data.name,(i>6?"":"\t"),type_data.active_count,type_data.count);
}



DECLARE_COMMAND(handle,"handle");
