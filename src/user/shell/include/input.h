#ifndef _INPUT_H_
#define _INPUT_H_ 1
#include <user/types.h>



#define INPUT_BUFFER_SIZE 256



extern char input[];
extern u32 input_length;



void input_get(void);



#endif
