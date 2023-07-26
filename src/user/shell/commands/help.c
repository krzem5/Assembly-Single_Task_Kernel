#include <command.h>
#include <string.h>
#include <user/io.h>



extern const command_t* __start_commands;
extern const command_t* __stop_commands;



void help_main(int argc,const char*const* argv){
	if (argc>2){
		printf("help: too many arguments\n");
		return;
	}
	const char* help_command=(argc>1?argv[1]:NULL);
	if (!help_command){
		printf("Possible command:\n");
	}
	for (const command_t*const* ptr=&__start_commands;ptr<&__stop_commands;ptr++){
		if (!*ptr||(help_command&&!string_equal(help_command,(*ptr)->name))){
			continue;
		}
		printf("\x1b[1m%s\x1b[0m\n",(*ptr)->help);
		if (help_command){
			return;
		}
	}
	if (help_command){
		printf("help: unrecognized command '%s'\n",help_command);
	}
}



DECLARE_COMMAND(help,"help [command]");
