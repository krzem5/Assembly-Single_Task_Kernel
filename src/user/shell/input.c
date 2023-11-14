#include <cwd.h>
#include <input.h>
#include <core/io.h>



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

#define KEY_MASK 0x1fff
#define KEY_FLAG_SHIFT 0x2000
#define KEY_FLAG_ALT 0x4000
#define KEY_FLAG_CTRL 0x8000



#define IS_IDENTIFIER(c) (((c)>64&&(c)<91)||((c)>96&&(c)<123)||(c)=='_'||((c)>47&&(c)<58))

#define IS_WHITESPACE(c) ((c)==' '||(c)=='\t')



typedef struct _HISTORY_ENTRY{
	char data[INPUT_BUFFER_SIZE+1];
	u32 length;
} history_entry_t;



static u32 _input_cursor;
static history_entry_t _input_history[INPUT_REWIND_BUFFER_SIZE];
static u32 _input_history_index=0;
static u32 _input_history_length=0;

char input[INPUT_BUFFER_SIZE+1];
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
		if (i>=3&&j>=2){
			keycode|=buffer[1]<<8;
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
		if (buffer[j]>64&&buffer[j]<91){
			keycode=buffer[j];
		}
		else{
			modifiers=buffer[j];
			keycode=buffer[j+1];
		}
	}
	u16 out=0;
	switch (keycode){
		case 'A':
			out=KEY_UP;
			break;
		case 'B':
			out=KEY_DOWN;
			break;
		case 'C':
			out=KEY_RIGHT;
			break;
		case 'D':
			out=KEY_LEFT;
			break;
		case 'F':
		case '4':
		case '8':
			out=KEY_END;
			break;
		case 'H':
		case '1':
		case '7':
			out=KEY_HOME;
			break;
		case '2':
			out=KEY_INSERT;
			break;
		case '3':
			out=KEY_DELETE;
			break;
		case '5':
			out=KEY_PAGE_UP;
			break;
		case '6':
			out=KEY_PAGE_DOWN;
			break;
		default:
			goto _retry;
	}
	modifiers-=49;
	if (modifiers&1){
		out|=KEY_FLAG_SHIFT;
	}
	if (modifiers&2){
		out|=KEY_FLAG_ALT;
	}
	if (modifiers&4){
		out|=KEY_FLAG_CTRL;
	}
	return out;
}



static void _insert_char(char c){
	history_entry_t* entry=_input_history+_input_history_index;
	if (_input_cursor==INPUT_BUFFER_SIZE){
		entry->data[_input_cursor-1]=c;
	}
	else{
		for (u32 i=entry->length;i>_input_cursor;i--){
			entry->data[i]=entry->data[i-1];
		}
		entry->data[_input_cursor]=c;
		_input_cursor++;
		if (entry->length<INPUT_BUFFER_SIZE){
			entry->length++;
		}
	}
	entry->data[entry->length]=0;
}



static void _delete_char(void){
	history_entry_t* entry=_input_history+_input_history_index;
	if (!entry->length){
		return;
	}
	entry->length--;
	for (u32 i=_input_cursor;i<entry->length;i++){
		entry->data[i]=entry->data[i+1];
	}
	entry->data[entry->length]=0;
}



static void _move_cursor(_Bool is_right,_Bool whole_word){
	history_entry_t* entry=_input_history+_input_history_index;
	if (is_right){
		if (_input_cursor+1>=entry->length){
			_input_cursor=entry->length;
			return;
		}
		_input_cursor++;
		if (!whole_word){
			return;
		}
		_Bool inside_identifier=IS_IDENTIFIER(entry->data[_input_cursor]);
		while (_input_cursor<entry->length){
			_input_cursor++;
			if (inside_identifier){
				if (!IS_IDENTIFIER(entry->data[_input_cursor])){
					break;
				}
			}
			else{
				if (IS_WHITESPACE(entry->data[_input_cursor])){
					break;
				}
			}
		}
	}
	else{
		if (_input_cursor<2){
			_input_cursor=0;
			return;
		}
		_input_cursor--;
		if (!whole_word){
			return;
		}
		_Bool inside_identifier=IS_IDENTIFIER(entry->data[_input_cursor]);
		while (_input_cursor){
			_input_cursor--;
			if (inside_identifier){
				if (!IS_IDENTIFIER(entry->data[_input_cursor])){
					_input_cursor++;
					break;
				}
			}
			else{
				if (IS_WHITESPACE(entry->data[_input_cursor])){
					_input_cursor++;
					break;
				}
			}
		}
	}
}



static void _scroll_history(_Bool is_up){
	if (is_up){
		if (!_input_history_index){
			return;
		}
		_input_history_index--;
	}
	else{
		if (_input_history_index==_input_history_length-1){
			return;
		}
		_input_history_index++;
	}
	_input_cursor=_input_history[_input_history_index].length;
}



static _Bool _shift_history(void){
	if (_input_history_length<INPUT_REWIND_BUFFER_SIZE){
		_input_history_length++;
		return 0;
	}
	for (u32 i=1;i<INPUT_REWIND_BUFFER_SIZE;i++){
		_input_history[i-1]=_input_history[i];
	}
	return 1;
}



static void _ensure_top_of_history(_Bool duplicate_entry){
	if (_input_history_index==_input_history_length-1){
		return;
	}
	history_entry_t entry=_input_history[_input_history_index];
	if (_input_history[_input_history_length-1].length){
		if (_shift_history()){
			if (!_input_history_index){
				duplicate_entry=1;
			}
			else{
				_input_history_index--;
			}
		}
	}
	if (!duplicate_entry){
		_input_history_length--;
		for (u32 i=_input_history_index;i<_input_history_length-1;i++){
			_input_history[i]=_input_history[i+1];
		}
	}
	_input_history[_input_history_length-1]=entry;
	_input_history_index=_input_history_length-1;
}



void input_get(void){
	_shift_history();
	_input_history_index=_input_history_length-1;
	_input_history[_input_history_index].data[0]=0;
	_input_history[_input_history_index].length=0;
	_input_cursor=0;
	while (1){
		printf("\x1b[G\x1b[2K\x1b[\x1b[1m\x1b[32mroot\x1b[0m:\x1b[1m\x1b[34m%s\x1b[0m$ %s\x1b[%uG",cwd,_input_history[_input_history_index].data,_input_cursor+cwd_length+8);
		int key=_get_key();
		if ((key&KEY_MASK)>31&&(key&KEY_MASK)<127){
			_ensure_top_of_history(1);
			_insert_char(key&KEY_MASK);
		}
		else{
			switch (key&KEY_MASK){
				case 9:
					_ensure_top_of_history(1);
					_insert_char(' ');
					_insert_char(' ');
					_insert_char(' ');
					_insert_char(' ');
					break;
				case 10:
				case 13:
					_ensure_top_of_history(0);
					printf("\x1b[%uG\n",input_length+cwd_length+8);
					history_entry_t* entry=_input_history+_input_history_index;
					input_length=entry->length;
					for (u32 i=0;i<=input_length;i++){
						input[i]=entry->data[i];
					}
					return;
				case 127:
					if (_input_cursor){
						_ensure_top_of_history(1);
						_input_cursor--;
						_delete_char();
					}
					break;
				case KEY_UP:
				case KEY_DOWN:
					_scroll_history((key&KEY_MASK)==KEY_UP);
					break;
				case KEY_LEFT:
				case KEY_RIGHT:
					_move_cursor((key&KEY_MASK)==KEY_RIGHT,!!(key&KEY_FLAG_CTRL));
					break;
				case KEY_DELETE:
					_ensure_top_of_history(1);
					_delete_char();
					break;
				case KEY_HOME:
					_input_cursor=0;
					break;
				case KEY_END:
					_input_cursor=_input_history[_input_history_index].length;
					break;
			}
		}
	}
}
