#include <compressor/compressor.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#define MIN_MATCH_LENGTH 4
#define MAX_NON_MATCH_LENGTH ((1<<14)-1)
#define MAX_MATCH_LENGTH ((1<<13)-1)
#define MATCH_ALIGNMENT 8

#define WINDOW_SIZE ((1<<10)-1)
#define MAX_PREPROCESSED_MATCH_LENGTH ((1<<10)-1)

#define WINDOW_SIZE_FAST ((1<<5)-1)
#define MAX_PREPROCESSED_MATCH_LENGTH_FAST ((1<<5)-1)



static uint32_t _encode_non_match(uint32_t non_match_length,uint8_t* out){
	out[0]=((non_match_length&0x3f)<<2)|((non_match_length>63)<<1);
	if (non_match_length>63){
		out[1]=non_match_length>>6;
		return 2;
	}
	return 1;
}



uint32_t compressor_get_max_compressed_size(uint32_t data_length){
	return (data_length+MAX_NON_MATCH_LENGTH-1)/MAX_NON_MATCH_LENGTH*(MAX_NON_MATCH_LENGTH+2);
}



uint32_t compressor_compress(const uint8_t* data,uint32_t data_length,uint32_t compression_level,uint8_t* out){
	uint32_t out_length=0;
	if (compression_level==COMPRESSOR_COMPRESSION_LEVEL_NONE){
		for (uint32_t offset=0;offset<data_length;){
			uint32_t chunk=data_length-offset;
			if (chunk>MAX_NON_MATCH_LENGTH){
				chunk=MAX_NON_MATCH_LENGTH;
			}
			out_length+=_encode_non_match(chunk,out+out_length);
			memcpy(out+out_length,data+offset,chunk);
			out_length+=chunk;
			offset+=chunk;
		}
		return out_length;
	}
	uint32_t window_size=WINDOW_SIZE;
	uint32_t max_preprocessed_match_length=MAX_PREPROCESSED_MATCH_LENGTH;
	if (compression_level==COMPRESSOR_COMPRESSION_LEVEL_FAST){
		window_size=WINDOW_SIZE_FAST;
		max_preprocessed_match_length=MAX_PREPROCESSED_MATCH_LENGTH_FAST;
	}
	uint16_t* kmp_search_table=calloc(max_preprocessed_match_length,sizeof(uint16_t));
	uint32_t non_match_length=0;
	uint32_t offset=0;
	while (offset<data_length){
		uint32_t capped_data_length=data_length-offset;
		if (capped_data_length>MAX_MATCH_LENGTH){
			capped_data_length=MAX_MATCH_LENGTH;
		}
		uint32_t preprocessed_data_length=capped_data_length;
		if (preprocessed_data_length>max_preprocessed_match_length){
			preprocessed_data_length=max_preprocessed_match_length;
		}
		uint32_t window_offset=(offset<window_size?0:offset-window_size);
		kmp_search_table[0]=0xffff;
		uint32_t j=0;
		for (uint32_t i=1;i<preprocessed_data_length;i++){
			if (data[offset+i]==data[offset+j]){
				kmp_search_table[i]=kmp_search_table[j];
			}
			else{
				kmp_search_table[i]=j;
				for (;j!=0xffff&&data[offset+i]!=data[offset+j];j=kmp_search_table[j]);
			}
			j=(j+1)&0xffff;
		}
		uint32_t match_length=0;
		uint32_t match_offset=0;
		int32_t i=0;
		for (j=0;j<preprocessed_data_length&&i<=((int32_t)(offset-window_offset+j))-MATCH_ALIGNMENT;i++){
			if (data[offset+j]==data[window_offset+i]){
				j++;
				continue;
			}
			if (j>match_length){
				match_length=j;
				match_offset=offset+j-window_offset-i;
			}
			j=kmp_search_table[j];
			if (j==0xffff){
				j=0;
			}
			else{
				i--;
			}
		}
		if (j>match_length){
			match_length=j;
			match_offset=offset+j-window_offset-i;
		}
		if (match_length==preprocessed_data_length){
			while (match_length<capped_data_length&&data[offset+match_length]==data[offset-match_offset+match_length]){
				match_length++;
			}
		}
		if (non_match_length==MAX_NON_MATCH_LENGTH||(non_match_length&&match_length>=MIN_MATCH_LENGTH)){
			out_length+=_encode_non_match(non_match_length,out+out_length);
			memcpy(out+out_length,data+offset-non_match_length,non_match_length);
			out_length+=non_match_length;
			non_match_length=0;
		}
		if (match_length<MIN_MATCH_LENGTH){
			non_match_length++;
			offset++;
		}
		else{
			out[out_length]=((match_length&0x7f)<<1)|1;
			out[out_length+1]=((match_offset&3)<<6)|(match_length>>7);
			out[out_length+2]=match_offset>>2;
			out_length+=3;
			offset+=match_length;
		}
	}
	if (non_match_length){
		out_length+=_encode_non_match(non_match_length,out+out_length);
		memcpy(out+out_length,data+offset-non_match_length,non_match_length);
		out_length+=non_match_length;
	}
	free(kmp_search_table);
	return out_length;
}
