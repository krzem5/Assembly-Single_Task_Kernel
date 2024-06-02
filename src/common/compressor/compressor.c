#include <common/compressor/compressor.h>
#include <common/types.h>
#if BUILD_MODULE
#include <kernel/memory/amm.h>
#include <kernel/writer/writer.h>
#else
#include <stdio.h>
#include <stdlib.h>
#endif



#define MIN_MATCH_LENGTH 4
#define MAX_NON_MATCH_LENGTH ((1<<14)-1)
#define MAX_MATCH_LENGTH ((1<<13)-1)
#define MATCH_ALIGNMENT 8

#define WINDOW_SIZE ((1<<10)-1)
#define MAX_PREPROCESSED_MATCH_LENGTH ((1<<10)-1)

#define WINDOW_SIZE_FAST ((1<<5)-1)
#define MAX_PREPROCESSED_MATCH_LENGTH_FAST ((1<<5)-1)



static u32 _emit_data(const void* data,u32 length,compressor_output_t* out){
#if BUILD_MODULE
	writer_append(out->writer,data,length);
#else
	return fwrite(data,1,length,out->file);
#endif
}



static void _encode_non_match(u32 non_match_length,compressor_output_t* out){
	u8 value=((non_match_length&0x3f)<<2)|((non_match_length>63)<<1);
	_emit_data(&value,sizeof(u8),out);
	if (non_match_length>63){
		value=non_match_length>>6;
		_emit_data(&value,sizeof(u8),out);
	}
}



void compressor_compress(const u8* data,u32 data_length,u32 compression_level,compressor_output_t* out){
	if (compression_level==COMPRESSOR_COMPRESSION_LEVEL_NONE){
		for (u32 offset=0;offset<data_length;){
			u32 chunk=data_length-offset;
			if (chunk>MAX_NON_MATCH_LENGTH){
				chunk=MAX_NON_MATCH_LENGTH;
			}
			_encode_non_match(chunk,out);
			_emit_data(data+offset,chunk,out);
			offset+=chunk;
		}
		return;
	}
	u32 window_size=WINDOW_SIZE;
	u32 max_preprocessed_match_length=MAX_PREPROCESSED_MATCH_LENGTH;
	if (compression_level==COMPRESSOR_COMPRESSION_LEVEL_FAST){
		window_size=WINDOW_SIZE_FAST;
		max_preprocessed_match_length=MAX_PREPROCESSED_MATCH_LENGTH_FAST;
	}
#if BUILD_MODULE
	u16* kmp_search_table=amm_alloc(max_preprocessed_match_length*sizeof(u16));
#else
	u16* kmp_search_table=calloc(max_preprocessed_match_length,sizeof(u16));
#endif
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
			_encode_non_match(non_match_length,out);
			_emit_data(data+offset-non_match_length,non_match_length,out);
			non_match_length=0;
		}
		if (match_length<MIN_MATCH_LENGTH){
			non_match_length++;
			offset++;
		}
		else{
			u8 encoded_match[3]={
				((match_length&0x7f)<<1)|1,
				((match_offset&3)<<6)|(match_length>>7),
				match_offset>>2
			};
			_emit_data(encoded_match,3*sizeof(u8),out);
			offset+=match_length;
		}
	}
	if (non_match_length){
		_encode_non_match(non_match_length,out);
		_emit_data(data+offset-non_match_length,non_match_length,out);
	}
#if BUILD_MODULE
	amm_dealloc(kmp_search_table);
#else
	free(kmp_search_table);
#endif
}
