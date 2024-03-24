#include <kernel/hash/sha256.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



#define SHA256_STEP(a,b,c,d,e,f,g,h,v) \
	t=(_rotate_bits_right(e,6)^_rotate_bits_right(e,11)^_rotate_bits_right(e,25))+(g^(e&(f^g)))+h+v; \
	d+=t; \
	h=t+(_rotate_bits_right(a,2)^_rotate_bits_right(a,13)^_rotate_bits_right(a,22))+((a&b)|((a|b)&c));



static KERNEL_INLINE u32 _rotate_bits_right(u32 a,u8 b){
	asm("ror %1,%0":"+r"(a):"c"(b));
	return a;
}



static void _process_chunk(hash_sha256_state_t* state,const u32* chunk){
	u32 w[64];
	for (u32 j=0;j<16;j++){
		w[j]=__builtin_bswap32(chunk[j]);
	}
	for (u32 j=16;j<64;j++){
		w[j]=(_rotate_bits_right(w[j-2],17)^_rotate_bits_right(w[j-2],19)^(w[j-2]>>10))+w[j-7]+(_rotate_bits_right(w[j-15],7)^_rotate_bits_right(w[j-15],18)^(w[j-15]>>3))+w[j-16];
	}
	u32 a=state->a;
	u32 b=state->b;
	u32 c=state->c;
	u32 d=state->d;
	u32 e=state->e;
	u32 f=state->f;
	u32 g=state->g;
	u32 h=state->h;
	u32 t;
	SHA256_STEP(a,b,c,d,e,f,g,h,w[0]+0x428a2f98);
	SHA256_STEP(h,a,b,c,d,e,f,g,w[1]+0x71374491);
	SHA256_STEP(g,h,a,b,c,d,e,f,w[2]+0xb5c0fbcf);
	SHA256_STEP(f,g,h,a,b,c,d,e,w[3]+0xe9b5dba5);
	SHA256_STEP(e,f,g,h,a,b,c,d,w[4]+0x3956c25b);
	SHA256_STEP(d,e,f,g,h,a,b,c,w[5]+0x59f111f1);
	SHA256_STEP(c,d,e,f,g,h,a,b,w[6]+0x923f82a4);
	SHA256_STEP(b,c,d,e,f,g,h,a,w[7]+0xab1c5ed5);
	SHA256_STEP(a,b,c,d,e,f,g,h,w[8]+0xd807aa98);
	SHA256_STEP(h,a,b,c,d,e,f,g,w[9]+0x12835b01);
	SHA256_STEP(g,h,a,b,c,d,e,f,w[10]+0x243185be);
	SHA256_STEP(f,g,h,a,b,c,d,e,w[11]+0x550c7dc3);
	SHA256_STEP(e,f,g,h,a,b,c,d,w[12]+0x72be5d74);
	SHA256_STEP(d,e,f,g,h,a,b,c,w[13]+0x80deb1fe);
	SHA256_STEP(c,d,e,f,g,h,a,b,w[14]+0x9bdc06a7);
	SHA256_STEP(b,c,d,e,f,g,h,a,w[15]+0xc19bf174);
	SHA256_STEP(a,b,c,d,e,f,g,h,w[16]+0xe49b69c1);
	SHA256_STEP(h,a,b,c,d,e,f,g,w[17]+0xefbe4786);
	SHA256_STEP(g,h,a,b,c,d,e,f,w[18]+0x0fc19dc6);
	SHA256_STEP(f,g,h,a,b,c,d,e,w[19]+0x240ca1cc);
	SHA256_STEP(e,f,g,h,a,b,c,d,w[20]+0x2de92c6f);
	SHA256_STEP(d,e,f,g,h,a,b,c,w[21]+0x4a7484aa);
	SHA256_STEP(c,d,e,f,g,h,a,b,w[22]+0x5cb0a9dc);
	SHA256_STEP(b,c,d,e,f,g,h,a,w[23]+0x76f988da);
	SHA256_STEP(a,b,c,d,e,f,g,h,w[24]+0x983e5152);
	SHA256_STEP(h,a,b,c,d,e,f,g,w[25]+0xa831c66d);
	SHA256_STEP(g,h,a,b,c,d,e,f,w[26]+0xb00327c8);
	SHA256_STEP(f,g,h,a,b,c,d,e,w[27]+0xbf597fc7);
	SHA256_STEP(e,f,g,h,a,b,c,d,w[28]+0xc6e00bf3);
	SHA256_STEP(d,e,f,g,h,a,b,c,w[29]+0xd5a79147);
	SHA256_STEP(c,d,e,f,g,h,a,b,w[30]+0x06ca6351);
	SHA256_STEP(b,c,d,e,f,g,h,a,w[31]+0x14292967);
	SHA256_STEP(a,b,c,d,e,f,g,h,w[32]+0x27b70a85);
	SHA256_STEP(h,a,b,c,d,e,f,g,w[33]+0x2e1b2138);
	SHA256_STEP(g,h,a,b,c,d,e,f,w[34]+0x4d2c6dfc);
	SHA256_STEP(f,g,h,a,b,c,d,e,w[35]+0x53380d13);
	SHA256_STEP(e,f,g,h,a,b,c,d,w[36]+0x650a7354);
	SHA256_STEP(d,e,f,g,h,a,b,c,w[37]+0x766a0abb);
	SHA256_STEP(c,d,e,f,g,h,a,b,w[38]+0x81c2c92e);
	SHA256_STEP(b,c,d,e,f,g,h,a,w[39]+0x92722c85);
	SHA256_STEP(a,b,c,d,e,f,g,h,w[40]+0xa2bfe8a1);
	SHA256_STEP(h,a,b,c,d,e,f,g,w[41]+0xa81a664b);
	SHA256_STEP(g,h,a,b,c,d,e,f,w[42]+0xc24b8b70);
	SHA256_STEP(f,g,h,a,b,c,d,e,w[43]+0xc76c51a3);
	SHA256_STEP(e,f,g,h,a,b,c,d,w[44]+0xd192e819);
	SHA256_STEP(d,e,f,g,h,a,b,c,w[45]+0xd6990624);
	SHA256_STEP(c,d,e,f,g,h,a,b,w[46]+0xf40e3585);
	SHA256_STEP(b,c,d,e,f,g,h,a,w[47]+0x106aa070);
	SHA256_STEP(a,b,c,d,e,f,g,h,w[48]+0x19a4c116);
	SHA256_STEP(h,a,b,c,d,e,f,g,w[49]+0x1e376c08);
	SHA256_STEP(g,h,a,b,c,d,e,f,w[50]+0x2748774c);
	SHA256_STEP(f,g,h,a,b,c,d,e,w[51]+0x34b0bcb5);
	SHA256_STEP(e,f,g,h,a,b,c,d,w[52]+0x391c0cb3);
	SHA256_STEP(d,e,f,g,h,a,b,c,w[53]+0x4ed8aa4a);
	SHA256_STEP(c,d,e,f,g,h,a,b,w[54]+0x5b9cca4f);
	SHA256_STEP(b,c,d,e,f,g,h,a,w[55]+0x682e6ff3);
	SHA256_STEP(a,b,c,d,e,f,g,h,w[56]+0x748f82ee);
	SHA256_STEP(h,a,b,c,d,e,f,g,w[57]+0x78a5636f);
	SHA256_STEP(g,h,a,b,c,d,e,f,w[58]+0x84c87814);
	SHA256_STEP(f,g,h,a,b,c,d,e,w[59]+0x8cc70208);
	SHA256_STEP(e,f,g,h,a,b,c,d,w[60]+0x90befffa);
	SHA256_STEP(d,e,f,g,h,a,b,c,w[61]+0xa4506ceb);
	SHA256_STEP(c,d,e,f,g,h,a,b,w[62]+0xbef9a3f7);
	SHA256_STEP(b,c,d,e,f,g,h,a,w[63]+0xc67178f2);
	state->a+=a;
	state->b+=b;
	state->c+=c;
	state->d+=d;
	state->e+=e;
	state->f+=f;
	state->g+=g;
	state->h+=h;
}



KERNEL_PUBLIC void hash_sha256_init(hash_sha256_state_t* out){
	out->a=0x6a09e667;
	out->b=0xbb67ae85;
	out->c=0x3c6ef372;
	out->d=0xa54ff53a;
	out->e=0x510e527f;
	out->f=0x9b05688c;
	out->g=0x1f83d9ab;
	out->h=0x5be0cd19;
	out->length=0;
}



KERNEL_PUBLIC void hash_sha256_process_chunk(hash_sha256_state_t* state,const void* chunk,u64 length){
	if (!length){
		return;
	}
	u64 padding=state->length&63;
	if (padding){
		u64 fragment=(length>64-padding?64-padding:length);
		memcpy(state->buffer+padding,chunk,fragment);
		chunk+=fragment;
		length-=fragment;
		state->length+=fragment;
		if (padding+fragment!=64){
			return;
		}
		_process_chunk(state,(const u32*)(state->buffer));
	}
	state->length+=length;
	for (;length>=64;length-=64){
		_process_chunk(state,(const u32*)chunk);
		chunk+=64;
	}
	if (length){
		memcpy(state->buffer,chunk,length);
	}
}



KERNEL_PUBLIC void hash_sha256_finalize(hash_sha256_state_t* state){
	u8 buffer[128];
	buffer[0]=0x80;
	u64 padding=(-state->length-9)&63;
	memset(buffer+1,0,padding);
	*((u64*)(buffer+1+padding))=__builtin_bswap64(state->length<<3);
	hash_sha256_process_chunk(state,buffer,padding+9);
	for (u32 i=0;i<8;i++){
		state->result32[i]=__builtin_bswap32(state->result32[i]);
	}
}
