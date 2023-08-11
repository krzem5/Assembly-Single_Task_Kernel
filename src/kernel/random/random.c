#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/random/random.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "random"



static inline u32 _rotate_bits(u32 a,u8 b){
	asm("rol %1,%0":"+r"(a):"c"(b));
	return a;
}



static inline void _chacha_block_quarter(u32* out,u8 a,u8 b,u8 c,u8 d){
	out[a]+=out[b];
	out[d]^=out[a];
	out[d]=_rotate_bits(out[d],16);
	out[c]+=out[d];
	out[b]^=out[c];
	out[b]=_rotate_bits(out[b],12);
	out[a]+=out[b];
	out[d]^=out[a];
	out[d]=_rotate_bits(out[d],8);
	out[c]+=out[d];
	out[b]^=out[c];
	out[b]=_rotate_bits(out[b],7);
}



static inline void _chacha_block(u32* state,u32* out){
	for (u8 i=0;i<16;i++){
		out[i]=state[i];
	}
	for (u8 i=0;i<10;i++){
		_chacha_block_quarter(out,0,4,8,12);
		_chacha_block_quarter(out,1,5,9,13);
		_chacha_block_quarter(out,2,6,10,14);
		_chacha_block_quarter(out,3,7,11,15);
		_chacha_block_quarter(out,0,5,10,15);
		_chacha_block_quarter(out,1,6,11,12);
		_chacha_block_quarter(out,2,7,8,13);
		_chacha_block_quarter(out,3,4,9,14);
	}
	(*((u64*)(state+4)))++;
}



static lock_t _random_chacha_lock;
static u32 _random_chacha_state[16];
static u32 _random_chacha_buffer[16];



void random_init(void){
	LOG("Initializing PRNG...");
	lock_init(&_random_chacha_lock);
	_random_init_entropy_pool();
	_random_get_entropy(_random_chacha_state);
}



void random_generate(void* buffer,u64 length){
	lock_acquire(&_random_chacha_lock);
	if (_random_has_entropy()){
		_random_get_entropy(_random_chacha_state);
	}
	u32* buffer_ptr=buffer;
	while (length){
		_chacha_block(_random_chacha_state,_random_chacha_buffer);
		if (length<32){
			u8* buffer_ptr8=(u8*)buffer_ptr;
			const u8* chacha_buffer8=(const u8*)_random_chacha_buffer;
			for (u8 i=0;i<length;i++){
				buffer_ptr8[i]=chacha_buffer8[i];
			}
			break;
		}
		for (u8 i=0;i<8;i++){
			buffer_ptr[i]=_random_chacha_buffer[i];
		}
		length-=32;
		buffer_ptr+=8;
	}
	lock_release(&_random_chacha_lock);
}
