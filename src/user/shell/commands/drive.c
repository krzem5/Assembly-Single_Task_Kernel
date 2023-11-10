#include <command.h>
#include <string.h>
// #include <user/drive.h>
// #include <user/handle.h>
#include <user/io.h>
#include <user/types.h>



void drive_main(int argc,const char*const* argv){
	const char* drive_name=NULL;
	for (u32 i=1;i<argc;i++){
		if (argv[i][0]!='-'&&!drive_name){
			drive_name=argv[i];
		}
		else{
			printf("drive: unrecognized option '%s'\n",argv[i]);
			return;
		}
	}
	if (!drive_name){
		printf("drive: no drive supplied\n");
		return;
	}
	// drive_data_t data;
	// handle_iterator_t iterator;
	// HANDLE_FOREACH(&iterator,"drive"){
	// 	if (!drive_get_data(iterator.handle,&data)){
	// 		continue;
	// 	}
	// 	if (string_equal(data.name,drive_name)){
	// 		goto _drive_found;
	// 	}
	// }
	printf("drive: drive '%s' not found\n",drive_name);
	return;
// _drive_found:
// 	printf("Name: \x1b[1m%s\x1b[0m (%s)\nModel name: \x1b[1m%s\x1b[0m\nSerial Number: \x1b[1m%s\x1b[0m\nSize: \x1b[1m%v\x1b[0m\nBlock size: \x1b[1m%v\x1b[0m\nBlock count: \x1b[1m%lu\x1b[0m\n",
// 		data.name,
// 		data.type,
// 		data.model_number,
// 		data.serial_number,
// 		data.block_count*data.block_size,
// 		data.block_size,
// 		data.block_count
// 	);
}



DECLARE_COMMAND(drive,"drive <drive>");
