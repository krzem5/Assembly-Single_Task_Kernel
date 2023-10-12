#include <command.h>
#include <cwd.h>
#include <user/aml.h>
#include <user/io.h>



void aml_main(int argc,const char*const* argv){
	if (argc>1){
		printf("aml: unrecognized option '%s'\n",argv[1]);
		return;
	}
	const aml_node_t* root=aml_get_root_node();
	if (root){
		aml_print_node(root);
	}
	else{
		printf("aml: unable to access AML data\n");
	}
}



DECLARE_COMMAND(aml,"aml");
