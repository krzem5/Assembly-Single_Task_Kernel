#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/rsa/rsa.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "rsa"
#pragma GCC optimize("no-tree-loop-distribute-patterns") // disable memmove



#define LOOKUP_TABLE_SIZE_SHIFT 5



static void _clear(rsa_number_t* a){
	for (u32 i=0;i<a->capacity;i++){
		a->data[i]=0;
	}
}



static void _normalize(rsa_number_t* a){
	for (u32 i=a->length;i<a->capacity;i++){
		a->data[i]=0;
	}
	for (;a->length>1&&!a->data[a->length-1];a->length--);
}



static _Bool _is_less_than(const rsa_number_t* a,const rsa_number_t* b){
	if (a->length<b->length){
		return 1;
	}
	if (a->length>b->length){
		return 0;
	}
	for (u32 i=a->length;i;){
		i--;
		if (a->data[i]<b->data[i]){
			return 1;
		}
		if (a->data[i]>b->data[i]){
			return 0;
		}
	}
	return 0;
}



static void _add(const rsa_number_t* a,const rsa_number_t* b,u32 b_offset,rsa_number_t* out){
	for (u32 i=0;i<b_offset;i++){
		out->data[i]=a->data[i];
	}
	u64 carry=0;
	for (u32 i=b_offset;i<a->length;i++){
		carry+=a->data[i]+b->data[i-b_offset];
		out->data[i]=carry;
		carry>>=32;
	}
	out->length=a->length+1;
	_normalize(out);
}



static void _subtract(const rsa_number_t* a,const rsa_number_t* b,rsa_number_t* out){
	if (_is_less_than(a,b)){
		panic("_subtract: invalid argument");
	}
	u32 carry=0;
	for (u32 i=0;i<a->length;i++){
		s64 value=((s64)(a->data[i]))-b->data[i]-carry;
		out->data[i]=value;
		carry=(value<0);
	}
	out->length=a->length;
	_normalize(out);
}



static void _subtract_fixed_length(const rsa_number_t* a,const rsa_number_t* b,u32 length,rsa_number_t* out){
	u32 carry=0;
	for (u32 i=0;i<length;i++){
		s64 value=((s64)(a->data[i]))-b->data[i]-carry;
		out->data[i]=value;
		carry=(value<0);
	}
	out->length=length;
	_normalize(out);
}



static void _mult(const rsa_number_t* a,u32 a_right_shift,const rsa_number_t* b,rsa_number_t* out){
	_clear(out);
	if (a_right_shift>=a->length){
		out->length=1;
		return;
	}
	for (u32 i=0;i<a->length-a_right_shift;i++){
		u64 x=a->data[i+a_right_shift];
		u64 carry=0;
		for (u32 j=0;j<b->length;j++){
			carry+=out->data[i+j]+x*b->data[j];
			out->data[i+j]=carry;
			carry>>=32;
		}
		out->data[i+b->length]=carry;
	}
	out->length=a->length-a_right_shift+b->length;
	_normalize(out);
}



static void _mult_int(const rsa_number_t* a,u32 b,rsa_number_t* out,u32 out_offset){
	for (u32 i=0;i<out_offset;i++){
		out->data[i]=0;
	}
	u64 carry=0;
	for (u32 i=0;i<a->length;i++){
		carry+=((u64)(a->data[i]))*b;
		out->data[i+out_offset]=carry;
		carry>>=32;
	}
	out->data[a->length+out_offset]=carry;
	out->length=a->length+out_offset+1;
	_normalize(out);
}



static void _square(const rsa_number_t* a,rsa_number_t* out){
	_clear(out);
	for (u32 i=0;i<a->length;i++){
		u64 x=a->data[i];
		u64 carry=0;
		for (u32 j=0;j<a->length;j++){
			carry+=out->data[i+j]+x*a->data[j];
			out->data[i+j]=carry;
			carry>>=32;
		}
		out->data[i+a->length]=carry;
	}
	out->length=a->length<<1;
	_normalize(out);
}



static void _modulo(rsa_number_t* a,const rsa_state_t* state){
	if (_is_less_than(a,state->modulus)){
		return;
	}
	u32 k=state->modulus->length;
	if (a->length>(k<<1)){
		panic("_modulo: invalid argument");
	}
	rsa_number_t* tmp1=rsa_number_create(state);
	_mult(a,k-1,state->_mu,tmp1);
	rsa_number_t* tmp2=rsa_number_create(state);
	_mult(tmp1,k+1,state->modulus,tmp2);
	rsa_number_delete(tmp1);
	_subtract_fixed_length(a,tmp2,k+1,a);
	rsa_number_delete(tmp2);
	while (_is_less_than(state->modulus,a)){
		_subtract(a,state->modulus,a);
	}
}



static void _shift_left_nooverflow(const rsa_number_t* a,u32 b,rsa_number_t* out){
	u64 carry=0;
	for (u32 i=0;i<a->length;i++){
		carry|=((u64)(a->data[i]))<<b;
		out->data[i]=carry;
		carry>>=32;
	}
	out->length=a->length;
}



static void _calculate_mu(rsa_state_t* state){
	u32 shift=__builtin_clz(state->modulus->data[state->modulus->length-1]);
	rsa_number_t* a=rsa_number_create(state);
	a->length=(state->modulus->length<<1)+1;
	a->data[state->modulus->length<<1]=1<<shift;
	rsa_number_t* b=rsa_number_create(state);
	_shift_left_nooverflow(state->modulus,shift,b);
	state->_mu=rsa_number_create(state);
	state->_mu->length=b->length+2;
	rsa_number_t* tmp=rsa_number_create(state);
	u32 b32=b->data[b->length-1];
	u64 b64=(((u64)b32)<<32)|b->data[b->length-2];
	for (u32 i=(b->length<<1);i>=b->length;i--){
		u32 j=i-b->length;
		u64 a64=(((u64)(a->data[i]))<<32)|a->data[i-1];
		u128 a128=(((u128)a64)<<32)|a->data[i-2];
		state->_mu->data[j]=(a->data[i]==b32?0xffffffff:a64/b32);
		for (u128 k=((u128)b64)*state->_mu->data[j];a128<k;k-=b64){
			state->_mu->data[j]--;
		}
		_mult_int(b,state->_mu->data[j],tmp,j);
		if (_is_less_than(a,tmp)){
			_add(a,b,j,a);
			state->_mu->data[j]--;
		}
		_subtract(a,tmp,a);
	}
	rsa_number_delete(a);
	rsa_number_delete(b);
	rsa_number_delete(tmp);
	_normalize(state->_mu);
}



KERNEL_PUBLIC rsa_number_t* rsa_number_create(const rsa_state_t* state){
	rsa_number_t* out=amm_alloc(sizeof(rsa_number_t)+state->max_number_length*sizeof(u32));
	out->length=0;
	out->capacity=state->max_number_length;
	_clear(out);
	return out;
}



KERNEL_PUBLIC rsa_number_t* rsa_number_create_from_bytes(const rsa_state_t* state,const u32* data,u32 length){
	rsa_number_t* out=amm_alloc(sizeof(rsa_number_t)+state->max_number_length*sizeof(u32));
	if (length>state->max_number_length){
		length=state->max_number_length;
	}
	out->length=length;
	out->capacity=state->max_number_length;
	memcpy(out->data,data,length*sizeof(u32));
	_normalize(out);
	return out;
}



KERNEL_PUBLIC void rsa_number_delete(rsa_number_t* number){
	_clear(number);
	number->length=0;
	number->capacity=0;
	amm_dealloc(number);
}



KERNEL_PUBLIC void rsa_state_init(const u32* modulus,u32 modulus_bit_length,rsa_state_t* out){
	u32 modulus_length=(modulus_bit_length+31)>>5;
	out->max_number_length=(modulus_length<<1)+3;
	out->modulus_bit_length=modulus_bit_length;
	out->public_key=NULL;
	out->private_key=NULL;
	out->modulus=rsa_number_create(out);
	out->modulus->length=modulus_length;
	memcpy(out->modulus->data,modulus,modulus_length*sizeof(u32));
	out->modulus->data[modulus_bit_length>>5]&=(1ull<<(modulus_bit_length&31))-1;
	_normalize(out->modulus);
	_calculate_mu(out);
}



KERNEL_PUBLIC void rsa_state_deinit(rsa_state_t* state){
	if (state->public_key){
		rsa_number_delete(state->public_key);
	}
	if (state->private_key){
		rsa_number_delete(state->private_key);
	}
	if (state->modulus){
		rsa_number_delete(state->modulus);
	}
	if (state->_mu){
		rsa_number_delete(state->_mu);
	}
}



KERNEL_PUBLIC void rsa_state_process(const rsa_state_t* state,rsa_number_t* value,u32 key,rsa_number_t* out){
	if (value->capacity!=state->max_number_length){
		panic("rsa_state_process: invalid argument");
	}
	_normalize(value);
	if (!_is_less_than(value,state->modulus)){
		panic("rsa_state_process: invalid argument");
	}
	const rsa_number_t* exponent=NULL;
	if (key==RSA_PUBLIC_KEY){
		exponent=state->public_key;
	}
	else if (key==RSA_PRIVATE_KEY){
		exponent=state->private_key;
	}
	if (!exponent){
		panic("rsa_state_process: invalid key");
	}
	if (!_is_less_than(exponent,state->modulus)){
		panic("rsa_state_process: invalid exponent");
	}
	if (out->capacity!=state->max_number_length){
		panic("rsa_state_process: invalid argument");
	}
	rsa_number_t* table[1<<LOOKUP_TABLE_SIZE_SHIFT];
	table[0]=value;
	rsa_number_t* value_sq=rsa_number_create(state);
	_square(value,value_sq);
	_modulo(value_sq,state);
	for (u32 i=1;i<(1<<LOOKUP_TABLE_SIZE_SHIFT);i++){
		table[i]=rsa_number_create(state);
		_mult(table[i-1],0,value_sq,table[i]);
		_modulo(table[i],state);
	}
	rsa_number_delete(value_sq);
	u32 mask=0;
	u32 mask_length=0;
	rsa_number_t* tmp[2]={rsa_number_create(state),rsa_number_create(state)};
	_Bool j=0;
	tmp[j]->data[0]=1;
	tmp[j]->length=1;
	for (u32 i=(exponent->length<<5);i;){
		i--;
		mask=(mask<<1)|((exponent->data[i>>5]>>(i&31))&1);
		mask_length+=!!mask;
		if (i){
			if (!mask){
				_square(tmp[j],tmp[j^1]);
				_modulo(tmp[j^1],state);
				j^=1;
				continue;
			}
			if (mask_length<LOOKUP_TABLE_SIZE_SHIFT+1){
				continue;
			}
		}
		else if (!mask_length){
			_square(tmp[j],tmp[j^1]);
			_modulo(tmp[j^1],state);
			j^=1;
			break;
		}
		u32 padding=__builtin_ffs(mask)-1;
		mask>>=padding;
		for (mask_length-=padding;mask_length;mask_length--){
			_square(tmp[j],tmp[j^1]);
			_modulo(tmp[j^1],state);
			j^=1;
		}
		_mult(tmp[j],0,table[mask>>1],tmp[j^1]);
		_modulo(tmp[j^1],state);
		j^=1;
		for (;padding;padding--){
			_square(tmp[j],tmp[j^1]);
			_modulo(tmp[j^1],state);
			j^=1;
		}
		mask=0;
	}
	for (u32 i=1;i<(1<<LOOKUP_TABLE_SIZE_SHIFT);i++){
		rsa_number_delete(table[i]);
	}
	out->length=tmp[j]->length;
	for (u32 i=0;i<tmp[j]->capacity;i++){
		out->data[i]=tmp[j]->data[i];
	}
	rsa_number_delete(tmp[0]);
	rsa_number_delete(tmp[1]);
}
