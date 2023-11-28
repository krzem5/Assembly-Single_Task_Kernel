import array



MIN_MATCH_LENGTH=4
MAX_NON_MATCH_LENGTH=(1<<15)-1
MAX_MATCH_LENGTH=(1<<13)-1
MATCH_ALIGNMENT=8

WINDOW_SIZE=(1<<10)-1
MAX_PREPROCESSED_MATCH_LENGTH=(1<<10)-1

WINDOW_SIZE_FAST=(1<<3)-1
MAX_PREPROCESSED_MATCH_LENGTH_FAST=(1<<3)-1

COMPRESSION_LEVEL_NONE=0
COMPRESSION_LEVEL_FAST=1
COMPRESSION_LEVEL_FULL=2



__all__=["COMPRESSION_LEVEL_NONE","COMPRESSION_LEVEL_FAST","COMPRESSION_LEVEL_FULL","compress"]



def compress(data,compression_level,out):
	offset=0
	length=len(data)
	out.write(bytearray([length&0xff,(length>>8)&0xff,(length>>16)&0xff,length>>24]))
	if (compression_level==COMPRESSION_LEVEL_NONE):
		out.write(data)
		# while (offset<length):
		# 	chunk=length-offset
		# 	if (chunk>MAX_NON_MATCH_LENGTH):
		# 		chunk=MAX_NON_MATCH_LENGTH
		# 	out.write(bytearray([(chunk&0x7f)<<1]))
		# 	if (chunk>>6):
		# 		out.write(bytearray([chunk>>7]))
		# 	out.write(data[offset:offset+chunk])
		# 	offset+=chunk
		return
	elif (compression_level==COMPRESSION_LEVEL_FAST):
		window_size=WINDOW_SIZE_FAST
		max_preprocessed_match_length=MAX_PREPROCESSED_MATCH_LENGTH_FAST
	else:
		window_size=WINDOW_SIZE
		max_preprocessed_match_length=MAX_PREPROCESSED_MATCH_LENGTH
	kmp_search_table=array.array("H")
	kmp_search_table.fromlist([0]*max_preprocessed_match_length)
	non_match_length=0
	while (offset<length):
		capped_data_length=length-offset
		if (capped_data_length>MAX_MATCH_LENGTH):
			capped_data_length=MAX_MATCH_LENGTH
		preprocessed_data_length=capped_data_length
		if (preprocessed_data_length>max_preprocessed_match_length):
			preprocessed_data_length=max_preprocessed_match_length
		window_offset=(0 if offset<window_size else offset-window_size)
		kmp_search_table[0]=0xffff
		j=0
		for i in range(1,preprocessed_data_length):
			if (data[offset+i]==data[offset+j]):
				kmp_search_table[i]=kmp_search_table[j]
			else:
				kmp_search_table[i]=j
				while (j!=0xffff and data[offset+i]!=data[offset+j]):
					j=kmp_search_table[j]
			j=(j+1)&0xffff
		match_length=0
		match_offset=0
		i=0
		j=0
		while (j<preprocessed_data_length and i<=offset-window_offset+j-MATCH_ALIGNMENT):
			if (data[offset+j]==data[window_offset+i]):
				j+=1
				i+=1;continue
			if (j>match_length):
				match_length=j
				match_offset=offset+j-window_offset-i
			j=kmp_search_table[j]
			if (j==0xffff):
				j=0
			else:
				i-=1
			i+=1;continue
		if (j>match_length):
			match_length=j
			match_offset=offset+j-window_offset-i
		if (match_length==preprocessed_data_length):
			while (match_length<capped_data_length and data[offset+match_length]==data[offset-match_offset+match_length]):
				match_length+=1
		if (non_match_length==MAX_NON_MATCH_LENGTH or (non_match_length and match_length>=MIN_MATCH_LENGTH)):
			out.write(bytearray([(non_match_length&0x7f)<<1]))
			if (non_match_length>>6):
				out.write(bytearray([non_match_length>>7]))
			out.write(data[offset-non_match_length:offset])
			non_match_length=0
		if (match_length<MIN_MATCH_LENGTH):
			non_match_length+=1
			offset+=1
		else:
			out.write(bytearray([
				((match_length&0x7f)<<1)|1,
				((match_offset&3)<<6)|(match_length>>7),
				match_offset>>2
			]))
			offset+=match_length
	if (non_match_length):
		out.write(bytearray([(non_match_length&0x7f)<<1]))
		if (non_match_length>>6):
			out.write(bytearray([non_match_length>>7]))
		out.write(data[offset-non_match_length:offset])
