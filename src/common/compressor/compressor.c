#include <common/compressor/compressor.h>
#include <common/types.h>
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



static u32 _encode_non_match(u32 non_match_length,u8* out){
	out[0]=((non_match_length&0x3f)<<2)|((non_match_length>63)<<1);
	if (non_match_length>63){
		out[1]=non_match_length>>6;
		return 2;
	}
	return 1;
}



u32 compressor_get_max_compressed_size(u32 data_length){
	return (data_length+MAX_NON_MATCH_LENGTH-1)/MAX_NON_MATCH_LENGTH*(MAX_NON_MATCH_LENGTH+2);
}



u32 compressor_compress(const u8* data,u32 data_length,u32 compression_level,u8* out){
	u32 out_length=0;
	if (compression_level==COMPRESSOR_COMPRESSION_LEVEL_NONE){
		for (u32 offset=0;offset<data_length;){
			u32 chunk=data_length-offset;
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
	u32 window_size=WINDOW_SIZE;
	u32 max_preprocessed_match_length=MAX_PREPROCESSED_MATCH_LENGTH;
	if (compression_level==COMPRESSOR_COMPRESSION_LEVEL_FAST){
		window_size=WINDOW_SIZE_FAST;
		max_preprocessed_match_length=MAX_PREPROCESSED_MATCH_LENGTH_FAST;
	}
	u16* kmp_search_table=calloc(max_preprocessed_match_length,sizeof(u16));
	u32 non_match_length=0;
	u32 offset=0;
	while (offset<data_length){
		u32 capped_data_length=data_length-offset;
		if (capped_data_length>MAX_MATCH_LENGTH){
			capped_data_length=MAX_MATCH_LENGTH;
		}
		u32 preprocessed_data_length=capped_data_length;
		if (preprocessed_data_length>max_preprocessed_match_length){
			preprocessed_data_length=max_preprocessed_match_length;
		}
		u32 window_offset=(offset<window_size?0:offset-window_size);
		kmp_search_table[0]=0xffff;
		u32 j=0;
		for (u32 i=1;i<preprocessed_data_length;i++){
			if (data[offset+i]==data[offset+j]){
				kmp_search_table[i]=kmp_search_table[j];
			}
			else{
				kmp_search_table[i]=j;
				for (;j!=0xffff&&data[offset+i]!=data[offset+j];j=kmp_search_table[j]);
			}
			j=(j+1)&0xffff;
		}
		u32 match_length=0;
		u32 match_offset=0;
		s32 i=0;
		for (j=0;j<preprocessed_data_length&&i<=((s32)(offset-window_offset+j))-MATCH_ALIGNMENT;i++){
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
