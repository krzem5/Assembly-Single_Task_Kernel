#include <user/io.h>
#include <user/types.h>



#define AT_NULL 0
#define AT_IGNORE 1
#define AT_PHDR 3
#define AT_PHENT 4
#define AT_PHNUM 5
#define AT_PAGESZ 6
#define AT_BASE 7
#define AT_FLAGS 8
#define AT_ENTRY 9
#define AT_PLATFORM 15
#define AT_HWCAP 16
#define AT_RANDOM 25
#define AT_HWCAP2 26
#define AT_EXECFN 31



extern void main(void);



void _entry(const u64* data){
	const char* string_table=(const char*)data;
	u32 argc=data[0];
	data++;
	printf("argc=%u\n",argc);
	for (u32 i=0;i<argc;i++){
		printf("argv[%u]=%s\n",i,string_table+data[0]);
		data++;
	}
	for (u32 i=0;data[0];i++){
		printf("environ[%u]=%s\n",i,string_table+data[0]);
		data++;
	}
	for (data++;data[0];data+=2){
		if (data[0]==AT_IGNORE){
			continue;
		}
		switch (data[0]){
			case AT_PHDR:
				printf("AT_PHDR=%p\n",data[1]);
				break;
			case AT_PHENT:
				printf("AT_PHENT=%v\n",data[1]);
				break;
			case AT_PHNUM:
				printf("AT_PHNUM=%lu\n",data[1]);
				break;
			case AT_PAGESZ:
				printf("AT_PAGESZ=%lu B\n",data[1]);
				break;
			case AT_BASE:
				printf("AT_BASE=%p\n",data[1]);
				break;
			case AT_FLAGS:
				printf("AT_FLAGS=%u\n",data[1]);
				break;
			case AT_ENTRY:
				printf("AT_ENTRY=%p\n",data[1]);
				break;
			case AT_PLATFORM:
				printf("AT_PLATFORM=%s\n",string_table+data[1]);
				break;
			case AT_HWCAP:
				printf("AT_HWCAP=%x\n",data[1]);
				break;
			case AT_RANDOM:
				printf("AT_RANDOM=%p\n",string_table+data[1]);
				break;
			case AT_HWCAP2:
				printf("AT_HWCAP2=%x\n",data[1]);
				break;
			case AT_EXECFN:
				printf("AT_EXECFN=%s\n",string_table+data[1]);
				break;
			default:
				printf("AT_%u=%lx\n",data[0],data[1]);
		}
	}
	main();
}
