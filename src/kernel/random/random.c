#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/random/random.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#define KERNEL_LOG_NAME "random"



static const char* _random_password_characters="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";



static rwlock_t _random_chacha_lock;
static u32 _random_chacha_state[16];
static u32 _random_chacha_buffer[16];



static KERNEL_INLINE u32 _rotate_bits(u32 a,u8 b){
	asm("rol %1,%0":"+r"(a):"c"(b));
	return a;
}



static KERNEL_INLINE void _chacha_block_quarter(u32* out,u8 a,u8 b,u8 c,u8 d){
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



static KERNEL_INLINE void _chacha_block(u32* state,u32* out){
	mem_copy(out,state,16);
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



void KERNEL_EARLY_EXEC random_init(void){
	LOG("Initializing PRNG...");
	rwlock_init(&_random_chacha_lock);
	_random_init_entropy_pool();
	_random_get_entropy(_random_chacha_state);
}



KERNEL_PUBLIC void random_generate(void* buffer,u64 length){
	rwlock_acquire_write(&_random_chacha_lock);
	_random_get_entropy(_random_chacha_state);
	u8* buffer_ptr=buffer;
	while (length){
		_chacha_block(_random_chacha_state,_random_chacha_buffer);
		u32 chunk_size=(length>32?32:length);
		mem_copy(buffer_ptr,_random_chacha_buffer,chunk_size);
		length-=chunk_size;
		buffer_ptr+=chunk_size;
	}
	rwlock_release_write(&_random_chacha_lock);
}



KERNEL_PUBLIC void random_generate_password(char* buffer,u64 length){
	random_generate(buffer,length-1);
	for (u64 i=0;i<length;i++){
		buffer[i]=_random_password_characters[buffer[i]&0x3f];
	}
	buffer[length]=0;
}
