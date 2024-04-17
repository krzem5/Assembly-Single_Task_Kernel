#include <compressor/compressor.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>



#define MIN_MATCH_LENGTH 4
#define MAX_NON_MATCH_LENGTH ((1<<14)-1)
#define MAX_MATCH_LENGTH ((1<<13)-1)
#define MATCH_ALIGNMENT 8

#define WINDOW_SIZE ((1<<10)-1)
#define MAX_PREPROCESSED_MATCH_LENGTH ((1<<10)-1)

#define WINDOW_SIZE_FAST ((1<<5)-1)
#define MAX_PREPROCESSED_MATCH_LENGTH_FAST ((1<<5)-1)



static _Bool _encode_non_match(uint32_t non_match_length,FILE* out){
	uint8_t buffer[2]={
		((non_match_length&0x3f)<<2)|((non_match_length>63)<<1),
		non_match_length>>6
	};
	uint32_t count=(non_match_length>63?2:1);
	return fwrite(buffer,1,count,out)==count;
}



_Bool compressor_compress(FILE* in,uint32_t compression_level,FILE* out){
	fseek(in,0,SEEK_END);
	uint32_t length=ftell(in);
	fseek(in,0,SEEK_SET);
	uint8_t header[4]={length,length>>8,length>>16,length>>24};
	if (fwrite(header,1,4,out)!=4){
		return 0;
	}
	uint8_t* data=malloc(length);
	if (fread(data,1,length,in)!=length){
		return 0;
	}
	if (compression_level==COMPRESSOR_COMPRESSION_LEVEL_NONE){
		for (uint32_t offset=0;offset<length;){
			uint32_t chunk=length-offset;
			if (chunk>MAX_NON_MATCH_LENGTH){
				chunk=MAX_NON_MATCH_LENGTH;
			}
			if (!_encode_non_match(chunk,out)||fwrite(data+offset,1,chunk,out)!=chunk){
				return 0;
			}
			offset+=chunk;
		}
		goto _cleanup;
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
	while (offset<length){
		uint32_t capped_data_length=length-offset;
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
			if (!_encode_non_match(non_match_length,out)||fwrite(data+offset-non_match_length,1,non_match_length,out)!=non_match_length){
				return 0;
			}
			non_match_length=0;
		}
		if (match_length<MIN_MATCH_LENGTH){
			non_match_length++;
			offset++;
		}
		else{
			uint8_t buffer[3]={
				((match_length&0x7f)<<1)|1,
				((match_offset&3)<<6)|(match_length>>7),
				match_offset>>2
			};
			if (fwrite(buffer,1,3,out)!=3){
				return 0;
			}
			offset+=match_length;
		}
	}
	if (non_match_length){
		if (!_encode_non_match(non_match_length,out)||fwrite(data+offset-non_match_length,1,non_match_length,out)!=non_match_length){
			return 0;
		}
	}
	free(kmp_search_table);
_cleanup:
	free(data);
	return 1;
}
