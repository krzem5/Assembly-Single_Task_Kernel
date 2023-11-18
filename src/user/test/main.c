#include <sys/io.h>



void main(int argc,const char*const* argv,const char*const* environ){
	printf("argc=%u\n",argc);
	for (int i=0;i<argc;i++){
		printf("argv[%u]=\"%s\"\n",i,argv[i]);
	}
	for (int i=0;environ[i];i++){
		printf("environ[%u]=\"%s\"\n",i,environ[i]);
	}
}
