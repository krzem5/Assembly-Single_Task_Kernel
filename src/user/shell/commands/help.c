#include <command.h>
#include <string.h>
#include <user/io.h>



extern const command_t* __start_commands;
extern const command_t* __stop_commands;



static void _quicksort(const command_t** data,u32 length){
	const command_t* pivot=data[length];
	u32 i=0;
	for (u32 j=0;j<length;j++){
		if (string_compare(data[j]->name,pivot->name)<0){
			const command_t* tmp=data[i];
			data[i]=data[j];
			data[j]=tmp;
			i++;
		}
	}
	const command_t* tmp=data[i];
	data[i]=data[length];
	data[length]=tmp;
	if (i>1){
		_quicksort(data,i-1);
	}
	i++;
	if (i<length){
		_quicksort(data+i,length-i);
	}
}



void help_main(int argc,const char*const* argv){
	if (argc>2){
		printf("help: too many arguments\n");
		return;
	}
	const char* help_command=(argc>1?argv[1]:NULL);
	if (!help_command){
		printf("Possible commands:\n");
	}
	const command_t* data[&__stop_commands-&__start_commands];
	u32 data_length=0;
	for (const command_t*const* ptr=&__start_commands;ptr<&__stop_commands;ptr++){
		if (!*ptr||(help_command&&!string_equal(help_command,(*ptr)->name))){
			continue;
		}
		if (help_command){
			printf("\x1b[1m%s\x1b[0m\n",(*ptr)->help);
			return;
		}
		data[data_length]=*ptr;
		data_length++;
	}
	if (help_command){
		printf("help: unrecognized command '%s'\n",help_command);
		return;
	}
	_quicksort(data,data_length-1);
	for (u32 i=0;i<data_length;i++){
		printf("\x1b[1m%s\x1b[0m\n",data[i]->help);
	}
}



DECLARE_COMMAND(help,"help [command]");
