#include <common/fat32/api.h>
#include <common/fat32/util.h>
#include <stdio.h>



typedef	struct _SHORT_NAME{
	char prefix[9];
	char suffix[4];
} short_name_t;



typedef struct _DATA_CHUNK{
	u32 offset;
	u32 cluster;
	u32 last_valid_cluster;
	void* data;
} data_chunk_t;



static const u64 _fat32_valid_character_names[4]={
	0x03ff23fb00000000ull,0x68000001c7ffffffull,
	0xffffffffffffffffull,0xffffffdfffffffffull
};



static bool _parse_short_name(const char* name,u32 name_length,short_name_t* out){
	if (!name_length){
		return 0;
	}
	u32 i=0;
	for (;name_length;name_length--){
		char c=*name;
		if (c=='.'){
			break;
		}
		if (i>=8||!(_fat32_valid_character_names[c>>6]&(1ull<<(c&63)))){
			return 0;
		}
		out->prefix[i]=c;
		name++;
		i++;
	}
	for (;i<8;i++){
		out->prefix[i]=' ';
	}
	out->prefix[8]=0;
	if (*name!='.'){
		out->suffix[0]=' ';
		out->suffix[1]=' ';
		out->suffix[2]=' ';
		out->suffix[3]=0;
		return 1;
	}
	name++;
	name_length--;
	i=0;
	for (;name_length;name_length--){
		char c=*name;
		if (i>=3||!(_fat32_valid_character_names[c>>6]&(1ull<<(c&63)))){
			return 0;
		}
		out->suffix[i]=c;
		name++;
		i++;
	}
	for (;i<3;i++){
		out->suffix[i]=' ';
	}
	out->suffix[3]=0;
	return 1;
}



static void _copy_from_padded(const char* src,char* dst,u32 length){
	for (;length&&src[length-1]==32;length--);
	mem_copy(dst,src,length);
	dst[length]=0;
}



static u32 _get_cluster_value(fat32_filesystem_t* fs,u32 cluster){
	if (cluster<2||cluster>=(fs->sector_count-fs->first_cluster_lba)/(fs->cluster_size/FAT32_SECTOR_SIZE)){
		return 0x0fffffff;
	}
	u32 i=cluster/(FAT32_SECTOR_SIZE/sizeof(u32));
	if (!(fs->fat_bitmap[i>>6]&(1ull<<i))){
		if (fs->config.read_callback(fs->config.ctx,fs->config.start_lba+fs->reserved_sectors+i,((void*)(fs->fat_data))+i*FAT32_SECTOR_SIZE,1)!=1){
			return 0x0fffffff;
		}
		fs->fat_bitmap[i>>6]|=1ull<<i;
	}
	return fs->fat_data[cluster]&0x0fffffff;
}



static void _set_cluster_value(fat32_filesystem_t* fs,u32 cluster,u32 value){
	fs->fat_data[cluster]=value;
	u32 i=cluster/(FAT32_SECTOR_SIZE/sizeof(u32));
	if (fs->config.write_callback(fs->config.ctx,fs->config.start_lba+fs->reserved_sectors+i,((void*)(fs->fat_data))+i*FAT32_SECTOR_SIZE,1)!=1);
}



static u32 _alloc_cluster(fat32_filesystem_t* fs){
	u32 cluster_count=(fs->sector_count-fs->first_cluster_lba)/(fs->cluster_size/FAT32_SECTOR_SIZE);
	for (u32 cluster=2;cluster<cluster_count;cluster++){
		if (_get_cluster_value(fs,cluster)){
			continue;
		}
		_set_cluster_value(fs,cluster,0x0fffffff);
		return cluster;
	}
	return 0;
}



static u32 _get_next_cluster(fat32_filesystem_t* fs,u32 cluster){
	u32 next=_get_cluster_value(fs,cluster);
	return (next<2||next>=(fs->sector_count-fs->first_cluster_lba)/(fs->cluster_size/FAT32_SECTOR_SIZE)?0:next);
}



static void _chunk_init(data_chunk_t* out){
	out->offset=0;
	out->cluster=0;
	out->last_valid_cluster=0;
	out->data=NULL;
}



static void _chunk_deinit(fat32_filesystem_t* fs,data_chunk_t* chunk){
	chunk->offset=0;
	chunk->cluster=0;
	chunk->last_valid_cluster=0;
	if (chunk->data){
		fs->config.dealloc_callback(chunk->data,(fs->cluster_size+4095)>>12);
	}
	chunk->data=NULL;
}



static bool _chunk_read(fat32_filesystem_t* fs,fat32_node_t* node,u64 offset,bool fetch_data,data_chunk_t* chunk){
	if (!node->cluster){
		return 0;
	}
	offset/=fs->cluster_size;
	if (chunk->cluster&&chunk->offset==offset){
		return 1;
	}
	if (chunk->offset>offset){
		chunk->offset=0;
		chunk->cluster=node->cluster;
	}
	else if (!chunk->cluster){
		chunk->offset=0;
		chunk->cluster=node->cluster;
	}
	for (;chunk->offset<offset;chunk->offset++){
		chunk->cluster=_get_next_cluster(fs,chunk->cluster);
		if (!chunk->cluster){
			chunk->offset=0;
			chunk->cluster=0;
			return 0;
		}
	}
	if (!chunk->data){
		chunk->data=fs->config.alloc_callback((fs->cluster_size+4095)>>12);
	}
	if (!fetch_data||fs->config.read_callback(fs->config.ctx,fs->first_cluster_lba+chunk->cluster-2,chunk->data,fs->cluster_size/FAT32_SECTOR_SIZE)==fs->cluster_size/FAT32_SECTOR_SIZE){
		chunk->last_valid_cluster=chunk->cluster;
		return 1;
	}
	chunk->offset=0;
	chunk->cluster=0;
	return 0;
}



bool fat32_filesystem_init(const fat32_filesystem_config_t* config,fat32_filesystem_t* out){
	u32 block_count=config->end_lba-config->start_lba+1;
	if (config->block_size!=FAT32_SECTOR_SIZE||block_count<32){
		return 0;
	}
	u8 buffer[512];
	if (config->read_callback(config->ctx,config->start_lba,buffer,1)!=1||buffer[0]!=0xeb||buffer[2]!=0x90||buffer[21]!=0xf8||(buffer[66]!=0x28&&buffer[66]!=0x29)||buffer[510]!=0x55||buffer[511]!=0xaa){
		return 0;
	}
	if (*((const u16*)(buffer+11))!=FAT32_SECTOR_SIZE||*((const u16*)(buffer+17))||*((const u16*)(buffer+19))||*((const u16*)(buffer+22))||*((const u16*)(buffer+42))){
		return 0;
	}
	out->config=*config;
	out->cluster_size=buffer[13]*FAT32_SECTOR_SIZE;
	out->reserved_sectors=buffer[14];
	out->fat_count=buffer[16];
	out->sector_count=*((const u32*)(buffer+32));
	out->sectors_per_fat=*((const u32*)(buffer+36));
	out->root_directory_cluster=*((const u32*)(buffer+44));
	out->first_cluster_lba=config->start_lba+out->reserved_sectors+out->fat_count*out->sectors_per_fat;
	u64 bitmap_size=(out->sectors_per_fat*FAT32_SECTOR_SIZE/sizeof(u32)+63)&(-64);
	void* data=config->alloc_callback((bitmap_size+out->sectors_per_fat*FAT32_SECTOR_SIZE+4095)>>12);
	out->fat_bitmap=data;
	out->fat_data=data+bitmap_size;
	return 1;
}



void fat32_filesystem_deinit(fat32_filesystem_t* fs){
	return;
}



bool fat32_filesystem_get_root(fat32_filesystem_t* fs,fat32_node_t* out){
	mem_fill(out,sizeof(fat32_node_t),0);
	out->flags=FAT32_NODE_FLAG_DIRECTORY;
	out->cluster=fs->root_directory_cluster;
	return 1;
}



bool fat32_node_create(fat32_filesystem_t* fs,fat32_node_t* parent,const char* name,u32 name_length,u32 flags,fat32_node_t* out){
	short_name_t short_name;
	if (!(parent->flags&FAT32_NODE_FLAG_DIRECTORY)||!_parse_short_name(name,name_length,&short_name)){
		return 0;
	}
	data_chunk_t chunk;
	u64 offset=0;
	u8* entry=NULL;
	for (_chunk_init(&chunk);_chunk_read(fs,parent,offset,1,&chunk);offset+=32){
		entry=chunk.data+(offset&(fs->cluster_size-1));
		if (!entry[0]){
			mem_fill(entry,32,0);
_fill_directory_entry:
			mem_copy(entry,short_name.prefix,8);
			mem_copy(entry+8,short_name.suffix,3);
			entry[11]=((flags&FAT32_NODE_FLAG_READONLY)?0x01:0x00)|((flags&FAT32_NODE_FLAG_DIRECTORY)?0x10:0x00);
			mem_fill(out,sizeof(fat32_node_t),0);
			out->pointer=(((u64)(chunk.cluster))*fs->cluster_size)|(offset&(fs->cluster_size-1));
			out->flags=flags;
			bool out=(fs->config.write_callback(fs->config.ctx,fs->first_cluster_lba+chunk.cluster-2,chunk.data,fs->cluster_size/FAT32_SECTOR_SIZE)==fs->cluster_size/FAT32_SECTOR_SIZE);
			_chunk_deinit(fs,&chunk);
			return out;
		}
	}
	u32 cluster=_alloc_cluster(fs);
	if (chunk.last_valid_cluster){
		_set_cluster_value(fs,chunk.last_valid_cluster,cluster);
		mem_fill(chunk.data,fs->cluster_size,0);
	}
	else{
		parent->cluster=cluster;
		fat32_node_flush(fs,parent);
		chunk.data=fs->config.alloc_callback((fs->cluster_size+4095)>>12);
	}
	chunk.cluster=cluster;
	offset=((u64)cluster)*fs->cluster_size;
	entry=chunk.data;
	goto _fill_directory_entry;
}



bool fat32_node_delete(fat32_filesystem_t* fs,fat32_node_t* node){
	panic("fat32_node_delete");
}



bool fat32_node_lookup(fat32_filesystem_t* fs,fat32_node_t* parent,const char* name,u32 name_length,fat32_node_t* out){
	short_name_t short_name;
	if (!(parent->flags&FAT32_NODE_FLAG_DIRECTORY)||!_parse_short_name(name,name_length,&short_name)){
		return 0;
	}
	data_chunk_t chunk;
	u64 offset=0;
	for (_chunk_init(&chunk);_chunk_read(fs,parent,offset,1,&chunk);offset+=32){
		const u8* entry=chunk.data+(offset&(fs->cluster_size-1));
		if (!entry[0]){
			break;
		}
		if (entry[0]==0x05||entry[0]=='.'||entry[0]==0xe5||(entry[11]&0xc8)){
			continue;
		}
		for (u32 i=0;i<8;i++){
			if (entry[i]!=short_name.prefix[i]){
				goto _check_next_entry;
			}
		}
		for (u32 i=0;i<3;i++){
			if (entry[8+i]!=short_name.suffix[i]){
				goto _check_next_entry;
			}
		}
		out->pointer=(((u64)(chunk.cluster))*fs->cluster_size)|(offset&(fs->cluster_size-1));
		out->flags=((entry[11]&0x01)?FAT32_NODE_FLAG_READONLY:0)|((entry[11]&0x10)?FAT32_NODE_FLAG_DIRECTORY:0);
		out->cluster=(*((const u16*)(entry+26)))|((*((const u16*)(entry+20)))<<16);
		out->size=*((const u32*)(entry+28));
		u16 date_bits=*((const u16*)(entry+18));
		out->time_access.year=1980+(date_bits>>9);
		out->time_access.month=(date_bits>>8)&15;
		out->time_access.day=date_bits&31;
		out->time_access.hour=0;
		out->time_access.minute=0;
		out->time_access.second=0;
		out->time_access.nanosecond=0;
		u16 time_bits=*((const u16*)(entry+22));
		date_bits=*((const u16*)(entry+24));
		out->time_modify.year=1980+(date_bits>>9);
		out->time_modify.month=(date_bits>>8)&15;
		out->time_modify.day=date_bits&31;
		out->time_modify.hour=time_bits>>11;
		out->time_modify.minute=(time_bits>>5)&63;
		out->time_modify.second=(time_bits&31)<<1;
		out->time_modify.nanosecond=0;
		time_bits=*((const u16*)(entry+14));
		date_bits=*((const u16*)(entry+16));
		out->time_birth.year=1980+(date_bits>>9);
		out->time_birth.month=(date_bits>>8)&15;
		out->time_birth.day=date_bits&31;
		out->time_birth.hour=time_bits>>11;
		out->time_birth.minute=(time_bits>>5)&63;
		out->time_birth.second=(time_bits&31)*2+entry[13]/100;
		out->time_birth.nanosecond=(entry[13]%100)*1000000;
		_chunk_deinit(fs,&chunk);
		return 1;
_check_next_entry:
	}
	_chunk_deinit(fs,&chunk);
	return 0;
}



u64 fat32_node_iterate(fat32_filesystem_t* fs,fat32_node_t* parent,u64 pointer,char* buffer,u32* buffer_length){
	data_chunk_t chunk;
	for (_chunk_init(&chunk);_chunk_read(fs,parent,pointer,1,&chunk);){
		const u8* entry=chunk.data+(pointer&(fs->cluster_size-1));
		if (!entry[0]){
			break;
		}
		pointer+=32;
		if (entry[0]==0x05||entry[0]=='.'||entry[0]==0xe5||(entry[11]&0xc8)){
			continue;
		}
		char entry_name[13];
		_copy_from_padded((const char*)entry,entry_name,8);
		if (!(entry[11]&0x10)){
			char* end=entry_name;
			for (;*end;end++);
			*end='.';
			_copy_from_padded((const char*)(entry+8),end+1,3);
		}
		u32 length=0;
		for (;entry_name[length];length++);
		if (length>*buffer_length){
			length=*buffer_length;
		}
		mem_copy(buffer,entry_name,length);
		*buffer_length=length;
		_chunk_deinit(fs,&chunk);
		return pointer;
	}
	_chunk_deinit(fs,&chunk);
	return 0;
}



bool fat32_node_link(fat32_filesystem_t* fs,fat32_node_t* parent,fat32_node_t* child,const char* name,u32 name_length){
	panic("fat32_node_link");
}



bool fat32_node_unlink(fat32_filesystem_t* fs,fat32_node_t* parent,fat32_node_t* child,const char* name,u32 name_length){
	panic("fat32_node_unlink");
}



u64 fat32_node_read(fat32_filesystem_t* fs,fat32_node_t* node,u64 offset,void* buffer,u64 size){
	if (node->flags&FAT32_NODE_FLAG_DIRECTORY){
		return 0;
	}
	panic("fat32_node_read");
}



u64 fat32_node_write(fat32_filesystem_t* fs,fat32_node_t* node,u64 offset,const void* buffer,u64 size){
	if ((node->flags&FAT32_NODE_FLAG_DIRECTORY)||offset>=node->size){
		return 0;
	}
	if (offset+size>=node->size){
		size=node->size-offset;
	}
	if (!size){
		return 0;
	}
	u64 out=size;
	data_chunk_t chunk;
	_chunk_init(&chunk);
	size+=offset;
	while (offset<size){
		_chunk_read(fs,node,offset,(offset&(fs->cluster_size-1))||(size-offset<fs->cluster_size),&chunk);
		u64 padding=offset&(fs->cluster_size-1);
		u64 write_size=(fs->cluster_size-padding>size-offset?size-offset:fs->cluster_size-padding);
		mem_copy(chunk.data+padding,buffer,write_size);
		buffer+=write_size;
		offset+=write_size;
		if (fs->config.write_callback(fs->config.ctx,fs->first_cluster_lba+chunk.cluster-2,chunk.data,fs->cluster_size/FAT32_SECTOR_SIZE)!=fs->cluster_size/FAT32_SECTOR_SIZE){
			out-=size-(offset-write_size);
			break;
		}
	}
	_chunk_deinit(fs,&chunk);
	return out;
}



u64 fat32_node_resize(fat32_filesystem_t* fs,fat32_node_t* node,u64 size){
	if (node->flags&FAT32_NODE_FLAG_DIRECTORY){
		return 0;
	}
	if ((node->size+fs->cluster_size-1)/fs->cluster_size==(size+fs->cluster_size-1)/fs->cluster_size){
		node->size=size;
		fat32_node_flush(fs,node);
		return size;
	}
	if (node->size>size){
		panic("fat32_node_resize: shrink");
	}
	void* clear_buffer=fs->config.alloc_callback((fs->cluster_size+4095)>>12);
	u32 remaining=(size+fs->cluster_size-1)/fs->cluster_size;
	if (!node->cluster){
		node->cluster=_alloc_cluster(fs);
		fat32_node_flush(fs,node);
		if (fs->config.write_callback(fs->config.ctx,fs->first_cluster_lba+node->cluster-2,clear_buffer,fs->cluster_size/FAT32_SECTOR_SIZE)!=fs->cluster_size/FAT32_SECTOR_SIZE){
			goto _cleanup;
		}
	}
	for (u32 cluster=node->cluster;remaining;remaining--){
		u32 next_cluster=_get_next_cluster(fs,cluster);
		if (next_cluster){
			cluster=next_cluster;
			continue;
		}
		next_cluster=_alloc_cluster(fs);
		_set_cluster_value(fs,cluster,next_cluster);
		cluster=next_cluster;
		if (fs->config.write_callback(fs->config.ctx,fs->first_cluster_lba+cluster-2,clear_buffer,fs->cluster_size/FAT32_SECTOR_SIZE)!=fs->cluster_size/FAT32_SECTOR_SIZE){
			goto _cleanup;
		}
	}
	node->size=size;
	fat32_node_flush(fs,node);
_cleanup:
	fs->config.dealloc_callback(clear_buffer,(fs->cluster_size+4095)>>12);
	return node->size;
}



bool fat32_node_flush(fat32_filesystem_t* fs,fat32_node_t* node){
	if (!node->pointer){
		return 0;
	}
	bool out=0;
	void* buffer=fs->config.alloc_callback((fs->cluster_size+4095)>>12);
	if (fs->config.read_callback(fs->config.ctx,fs->first_cluster_lba+node->pointer/fs->cluster_size-2,buffer,fs->cluster_size/FAT32_SECTOR_SIZE)!=fs->cluster_size/FAT32_SECTOR_SIZE){
		goto _cleanup;
	}
	u8* entry=buffer+(node->pointer&(fs->cluster_size-1));
	entry[11]&=0xfe;
	if (node->flags&FAT32_NODE_FLAG_READONLY){
		entry[11]|=0x01;
	}
	entry[13]=(node->time_birth.second&1)*100+node->time_birth.nanosecond/1000000;
	*((u16*)(entry+14))=(node->time_birth.hour<<11)|(node->time_birth.minute<<5)|(node->time_birth.second>>1);
	*((u16*)(entry+16))=((node->time_birth.year-1980)<<9)|(node->time_birth.month<<8)|node->time_birth.day;
	*((u16*)(entry+18))=((node->time_access.year-1980)<<9)|(node->time_access.month<<8)|node->time_access.day;
	*((u16*)(entry+20))=node->cluster>>16;
	*((u16*)(entry+22))=(node->time_modify.hour<<11)|(node->time_modify.minute<<5)|(node->time_modify.second>>1);
	*((u16*)(entry+24))=((node->time_modify.year-1980)<<9)|(node->time_modify.month<<8)|node->time_modify.day;
	*((u16*)(entry+26))=node->cluster;
	*((u32*)(entry+28))=node->size;
	if (fs->config.write_callback(fs->config.ctx,fs->first_cluster_lba+node->pointer/fs->cluster_size-2,buffer,fs->cluster_size/FAT32_SECTOR_SIZE)!=fs->cluster_size/FAT32_SECTOR_SIZE){
		goto _cleanup;
	}
	out=1;
_cleanup:
	fs->config.dealloc_callback(buffer,(fs->cluster_size+4095)>>12);
	return out;
}
