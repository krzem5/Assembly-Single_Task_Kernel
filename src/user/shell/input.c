#include <cwd.h>
#include <input.h>
#include <user/io.h>



#define MAKE_KEYCODE(a,b) ((a)|((b)<<8))



#define KEY_UP 256
#define KEY_DOWN 257
#define KEY_RIGHT 258
#define KEY_LEFT 259
#define KEY_END 260
#define KEY_HOME 261
#define KEY_INSERT 262
#define KEY_DELETE 263
#define KEY_PAGE_UP 264
#define KEY_PAGE_DOWN 265

#define KEY_MASK 0x3fff
#define KEY_FLAG_SHIFT 0x4000
#define KEY_FLAG_CTRL 0x8000



char input[INPUT_BUFFER_SIZE];
u32 input_length;



static int _get_key(void){
_retry:
	int c=getchar();
	if (c!=27){
		return c;
	}
	c=getchar();
	if (c!='['){
		switch (c){
			case 'A':
				return KEY_UP;
			case 'B':
				return KEY_DOWN;
			case 'C':
				return KEY_RIGHT;
			case 'D':
				return KEY_LEFT;
			case 'F':
				return KEY_END;
			case 'H':
				return KEY_HOME;
		}
		return c;
	}
	char buffer[16];
	u8 i=0;
	u8 j=0xff;
	for (;i<16;i++){
		c=getchar_timeout(0xfff);
		if (c==-1){
			break;
		}
		buffer[i]=c;
		if (c==';'){
			j=i;
		}
	}
	u8 modifiers=49;
	u16 keycode=0;
	if (i&&buffer[i-1]=='~'){
		keycode=buffer[0];
		if (i>=2&&j>=2){
			keycode=buffer[1]<<8;
		}
		if (j!=0xff){
			modifiers=buffer[j+1];
		}
	}
	else{
		if (j==0xff){
			j=0;
		}
		else{
			j++;
		}
		if (buffer[j]>65&&buffer[j]<90){
			keycode=buffer[j];
		}
		else{
			modifiers=buffer[j];
			keycode=buffer[j+1];
		}
	}
	u16 out=0;
	switch (keycode){
		case MAKE_KEYCODE('A',0):
			out=KEY_UP;
			break;
		case MAKE_KEYCODE('B',0):
			out=KEY_DOWN;
			break;
		case MAKE_KEYCODE('C',0):
			out=KEY_RIGHT;
			break;
		case MAKE_KEYCODE('D',0):
			out=KEY_LEFT;
			break;
		case MAKE_KEYCODE('F',0):
		case MAKE_KEYCODE(4,0):
		case MAKE_KEYCODE(8,0):
			out=KEY_END;
			break;
		case MAKE_KEYCODE('H',0):
		case MAKE_KEYCODE(1,0):
		case MAKE_KEYCODE(7,0):
			out=KEY_HOME;
			break;
		case MAKE_KEYCODE(2,0):
			out=KEY_INSERT;
			break;
		case MAKE_KEYCODE(3,0):
			out=KEY_DELETE;
			break;
		case MAKE_KEYCODE(5,0):
			out=KEY_PAGE_UP;
			break;
		case MAKE_KEYCODE(6,0):
			out=KEY_PAGE_DOWN;
			break;
		default:
			goto _retry;
	}
	modifiers-=49;
	if (modifiers&1){
		out|=KEY_FLAG_SHIFT;
	}
	if (modifiers&4){
		out|=KEY_FLAG_CTRL;
	}
	return out;
}



void input_get(void){
	input_length=0;
	printf("\x1b[1m\x1b[38;2;67;154;6mshell\x1b[0m:\x1b[1m\x1b[38;2;52;101;164m%s\x1b[0m$ \n",cwd);
	while (1){
		int key=_get_key();
		printf("%u [%c]",key,key);
	}
}
