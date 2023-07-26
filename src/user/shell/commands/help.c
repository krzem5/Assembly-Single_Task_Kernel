#include <command.h>
#include <user/io.h>



extern const command_t* __start_commands;
extern const command_t* __stop_commands;



void help_main(int argc,const char*const* argv){
	printf("Possible command:\n");
	for (const command_t*const* ptr=&__start_commands;ptr<&__stop_commands;ptr++){
		if (!*ptr){
			continue;
		}
		printf("- \x1b[1m%s\x1b[0m\n",(*ptr)->name);
	}
}



DECLARE_COMMAND(help);
