import binascii
import struct
import sys
import uuid



GPT_TABLE_HEADER_SIGNATURE=0x5452415020494645
GPT_FLAGS={
	"boot": 0
}

PARITION_TABLE_ENTRY_COUNT=128



def _parse_guid(guid):
	if (len(guid)!=36):
		raise RuntimeError("Invalid GUID")
	out=[]
	j=0
	for i in range(0,36):
		if (0x842100&(1<<i)):
			if (guid[i]!="-"):
				raise RuntimeError("Invalid GUID")
			continue
		if (not (j&1)):
			out.append(0)
		out[j>>1]<<=4
		c=ord(guid[i])
		if (48<=c and c<=57):
			out[j>>1]|=c-48
		elif (65<=c and c<=90):
			out[j>>1]|=c-55
		elif (97<=c and c<=122):
			out[j>>1]|=c-87
		else:
			raise RuntimeError("Invalid GUID")
		j+=1
	out[0],out[3]=out[3],out[0]
	out[1],out[2]=out[2],out[1]
	out[4],out[5]=out[5],out[4]
	out[6],out[7]=out[7],out[6]
	return out



def _add_partition(entry_buffer,i,name,guid,start,end,flags):
	entry_buffer[i:i+128]=struct.pack("<16B16BQQQ72B",
		*_parse_guid(guid),
		*uuid.uuid4().bytes,
		start,
		end,
		sum([GPT_FLAGS[k] for k in flags]),
		*name.ljust(36,"\x00").encode("utf-16le")
	)



def generate(file_path,block_size,block_count,partitions):
	partition_table_size=(PARITION_TABLE_ENTRY_COUNT*128+block_size-1)&(-block_size);
	with open(file_path,"wb") as wf:
		buffer=bytearray(block_size)
		for i in range(0,block_count):
			wf.write(buffer)
	with open(file_path,"r+b") as wf:
		wf.seek(446)
		wf.write(b"\x00\x00\x02\x00\xee\xff\xff\xff"+struct.pack("<II",1,block_count-1))
		wf.seek(510)
		wf.write(b"\x55\xaa")
		entry_buffer=bytearray(partition_table_size)
		for i,part in enumerate(partitions.iter()):
			end=next(part.find("end")).data
			flags=list(part.find("flags"))
			_add_partition(entry_buffer,i*128,next(part.find("name")).data,next(part.find("guid")).data,next(part.find("start")).data,(end+block_count if end<0 else end),([e.name for e in flags[0].iter()] if flags else []))
		wf.seek(block_size*2)
		wf.write(entry_buffer)
		wf.seek(block_size*(block_count-partition_table_size//block_size-1))
		wf.write(entry_buffer)
		buffer=bytearray(struct.pack("<QIIIIQQQQ16BQIII",
			GPT_TABLE_HEADER_SIGNATURE,
			0x00010000,
			struct.calcsize("<QIIIIQQQQ16BQIII"),
			0,
			0,
			1,
			block_count-1,
			2+partition_table_size//block_size,
			block_count-partition_table_size//block_size-2,
			*uuid.uuid4().bytes,
			2,
			PARITION_TABLE_ENTRY_COUNT,
			128,
			binascii.crc32(entry_buffer)
		))
		buffer[16:20]=struct.pack("<I",binascii.crc32(buffer))
		wf.seek(block_size)
		wf.write(buffer)
		buffer[16:20]=b"\x00\x00\x00\x00"
		buffer[24:40]=struct.pack("<QQ",block_count-1,1)
		buffer[72:80]=struct.pack("<Q",block_count-partition_table_size//block_size-1)
		buffer[16:20]=struct.pack("<I",binascii.crc32(buffer))
		wf.seek((block_count-1)*block_size)
		wf.write(buffer)
