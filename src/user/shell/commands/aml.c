#include <command.h>
#include <cwd.h>
#include <user/aml.h>
#include <user/io.h>



void aml_main(int argc,const char*const* argv){
	if (argc>1){
		printf("aml: unrecognized option '%s'\n",argv[1]);
		return;
	}
	if (aml_root_node){
		aml_print_node(aml_root_node);
	}
	else{
		printf("aml: unable to access AML data\n");
	}
}



DECLARE_COMMAND(aml,"aml");
