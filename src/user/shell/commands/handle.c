#include <command.h>
#include <string.h>
#include <user/handle.h>
#include <user/io.h>



void handle_main(int argc,const char*const* argv){
	if (argc>1){
		printf("handle: unrecognized option '%s'\n",argv[1]);
		return;
	}
	u32 type_count=handle_get_type_count();
	if (!type_count){
		goto _error;
	}
	for (u32 i=0;i<type_count;i++){
		handle_type_data_t type_data;
		if (!handle_get_type_data(i,&type_data)){
			goto _error;
		}
		u8 j=0;
		for (;type_data.name[j];j++);
		printf("%s:\t%s\x1b[1m%u\x1b[0m\n",type_data.name,(j>6?"":"\t"),type_data.count);
	}
	return;
_error:
	printf("handle: unable to read handle stats\n");
}



DECLARE_COMMAND(handle,"handle");
