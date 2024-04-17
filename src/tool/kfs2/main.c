#include <stdio.h>



int main(int argc,const char** argv){
	if (argc<4){
		printf("Usage:\n%s <drive file> <offset> <command> [...arguments]\n",(argc?argv[0]:"kfs2"));
		return 1;
	}
	printf("kfs2 tool\n");
	return 0;
}
