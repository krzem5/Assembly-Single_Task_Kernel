#include <command.h>
#include <input.h>
#include <string.h>
#include <core/io.h>
#include <core/types.h>



extern const command_t* __start_commands;
extern const command_t* __stop_commands;



void command_execute(const char* command){
	char buffer[INPUT_BUFFER_SIZE+1];
	char* buffer_ptr=buffer;
	const char* argv[MAX_ARG_COUNT]={buffer_ptr};
	u32 argc=1;
	while (*command){
		if (*command==' '){
			argv[argc]=buffer_ptr+1;
			if (argc<MAX_ARG_COUNT){
				argc++;
			}
			*buffer_ptr=0;
		}
		else{
			*buffer_ptr=*command;
		}
		command++;
		buffer_ptr++;
	}
	*buffer_ptr=0;
	for (const command_t*const* ptr=&__start_commands;ptr<&__stop_commands;ptr++){
		if (*ptr&&string_equal(buffer,(*ptr)->name)){
			(*ptr)->func(argc,argv);
			return;
		}
	}
	printf("%s: command not found\n",buffer);
}
