#include <kernel/drive/drive.h>
#include <kernel/fs/fs.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs2/node.h>
#define KERNEL_LOG_NAME "kfs2"



#define KFS2_BLOCK_SIZE 4096

#define KFS2_ROOT_BLOCK_SIGNATURE 0x544f4f523253464b
#define KFS2_BITMAP_LEVEL_COUNT 5

#define KFS2_INODE_GET_BLOCK_INDEX(inode) ((inode)/(KFS2_BLOCK_SIZE/sizeof(kfs2_node_t)))
#define KFS2_INODE_GET_NODE_INDEX(inode) ((inode)%(KFS2_BLOCK_SIZE/sizeof(kfs2_node_t)))

#define KFS2_INODE_TYPE_FILE 0x0000
#define KFS2_INODE_TYPE_DIRECTORY 0x0001
#define KFS2_INODE_TYPE_MASK 0x0001

#define KFS2_INODE_STORAGE_MASK 0x000e
#define KFS2_INODE_STORAGE_TYPE_INLINE 0x0000
#define KFS2_INODE_STORAGE_TYPE_SINGLE 0x0002
#define KFS2_INODE_STORAGE_TYPE_DOUBLE 0x0004
#define KFS2_INODE_STORAGE_TYPE_TRIPLE 0x0006
#define KFS2_INODE_STORAGE_TYPE_QUADRUPLE 0x0008



typedef struct __attribute__((packed)) _KFS2_ROOT_BLOCK{
	u64 signature;
	u64 block_count;
	u64 inode_count;
	u64 data_block_count;
	u64 first_inode_block;
	u64 first_data_block;
	u64 first_bitmap_block;
	u64 inode_allocation_bitmap_offsets[KFS2_BITMAP_LEVEL_COUNT];
	u64 data_block_allocation_bitmap_offsets[KFS2_BITMAP_LEVEL_COUNT];
	u16 inode_allocation_bitmap_highest_level_length;
	u16 data_block_allocation_bitmap_highest_level_length;
	u32 kernel_inode;
	u32 initramfs_inode;
	u32 crc;
} kfs2_root_block_t;



typedef struct __attribute__((packed)) _KFS2_NODE{
	u64 size;
	union{
		u8 inline_[48];
		u64 single[6];
		u64 double_;
		u64 triple;
		u64 quadruple;
	} data;
	u16 hard_link_count;
	u16 flags;
	union{
		u32 crc;
		u32 _inode;
	};
} kfs2_node_t;



typedef struct __attribute__((packed)) _KFS2_DIRECTORY_ENTRY{
	u32 inode;
	u16 size;
	u8 name_length;
	u8 type;
	char name[];
} kfs2_directory_entry_t;



typedef struct _KFS2_DATA_CHUNK{
	u64 offset;
	void* data;
	u16 length;
} kfs2_data_chunk_t;



typedef struct _KFS2_FS_EXTRA_DATA{
	kfs2_root_block_t root_block;
	u32 block_size_shift;
} kfs2_fs_extra_data_t;



typedef struct _KFS2_VFS_NODE{
	vfs2_node_t node;
	kfs2_node_t kfs2_node;
} kfs2_vfs_node_t;



PMM_DECLARE_COUNTER(OMM_KFS2_NODE);
PMM_DECLARE_COUNTER(OMM_KFS2_EDATA);
PMM_DECLARE_COUNTER(KFS2_CHUNK);



static omm_allocator_t _kfs2_vfs_node_allocator=OMM_ALLOCATOR_INIT_STRUCT("kfs2_node",sizeof(kfs2_vfs_node_t),8,4,PMM_COUNTER_OMM_KFS2_NODE);
static omm_allocator_t _kfs2_fs_extra_data_allocator=OMM_ALLOCATOR_INIT_STRUCT("kfs2_extra_data",sizeof(kfs2_fs_extra_data_t),8,1,PMM_COUNTER_OMM_KFS2_EDATA);



static const u32 _kfs2_crc_table[256]={
	0x00000000,0x77073096,0xee0e612c,0x990951ba,0x076dc419,0x706af48f,0xe963a535,0x9e6495a3,
	0x0edb8832,0x79dcb8a4,0xe0d5e91e,0x97d2d988,0x09b64c2b,0x7eb17cbd,0xe7b82d07,0x90bf1d91,
	0x1db71064,0x6ab020f2,0xf3b97148,0x84be41de,0x1adad47d,0x6ddde4eb,0xf4d4b551,0x83d385c7,
	0x136c9856,0x646ba8c0,0xfd62f97a,0x8a65c9ec,0x14015c4f,0x63066cd9,0xfa0f3d63,0x8d080df5,
	0x3b6e20c8,0x4c69105e,0xd56041e4,0xa2677172,0x3c03e4d1,0x4b04d447,0xd20d85fd,0xa50ab56b,
	0x35b5a8fa,0x42b2986c,0xdbbbc9d6,0xacbcf940,0x32d86ce3,0x45df5c75,0xdcd60dcf,0xabd13d59,
	0x26d930ac,0x51de003a,0xc8d75180,0xbfd06116,0x21b4f4b5,0x56b3c423,0xcfba9599,0xb8bda50f,
	0x2802b89e,0x5f058808,0xc60cd9b2,0xb10be924,0x2f6f7c87,0x58684c11,0xc1611dab,0xb6662d3d,
	0x76dc4190,0x01db7106,0x98d220bc,0xefd5102a,0x71b18589,0x06b6b51f,0x9fbfe4a5,0xe8b8d433,
	0x7807c9a2,0x0f00f934,0x9609a88e,0xe10e9818,0x7f6a0dbb,0x086d3d2d,0x91646c97,0xe6635c01,
	0x6b6b51f4,0x1c6c6162,0x856530d8,0xf262004e,0x6c0695ed,0x1b01a57b,0x8208f4c1,0xf50fc457,
	0x65b0d9c6,0x12b7e950,0x8bbeb8ea,0xfcb9887c,0x62dd1ddf,0x15da2d49,0x8cd37cf3,0xfbd44c65,
	0x4db26158,0x3ab551ce,0xa3bc0074,0xd4bb30e2,0x4adfa541,0x3dd895d7,0xa4d1c46d,0xd3d6f4fb,
	0x4369e96a,0x346ed9fc,0xad678846,0xda60b8d0,0x44042d73,0x33031de5,0xaa0a4c5f,0xdd0d7cc9,
	0x5005713c,0x270241aa,0xbe0b1010,0xc90c2086,0x5768b525,0x206f85b3,0xb966d409,0xce61e49f,
	0x5edef90e,0x29d9c998,0xb0d09822,0xc7d7a8b4,0x59b33d17,0x2eb40d81,0xb7bd5c3b,0xc0ba6cad,
	0xedb88320,0x9abfb3b6,0x03b6e20c,0x74b1d29a,0xead54739,0x9dd277af,0x04db2615,0x73dc1683,
	0xe3630b12,0x94643b84,0x0d6d6a3e,0x7a6a5aa8,0xe40ecf0b,0x9309ff9d,0x0a00ae27,0x7d079eb1,
	0xf00f9344,0x8708a3d2,0x1e01f268,0x6906c2fe,0xf762575d,0x806567cb,0x196c3671,0x6e6b06e7,
	0xfed41b76,0x89d32be0,0x10da7a5a,0x67dd4acc,0xf9b9df6f,0x8ebeeff9,0x17b7be43,0x60b08ed5,
	0xd6d6a3e8,0xa1d1937e,0x38d8c2c4,0x4fdff252,0xd1bb67f1,0xa6bc5767,0x3fb506dd,0x48b2364b,
	0xd80d2bda,0xaf0a1b4c,0x36034af6,0x41047a60,0xdf60efc3,0xa867df55,0x316e8eef,0x4669be79,
	0xcb61b38c,0xbc66831a,0x256fd2a0,0x5268e236,0xcc0c7795,0xbb0b4703,0x220216b9,0x5505262f,
	0xc5ba3bbe,0xb2bd0b28,0x2bb45a92,0x5cb36a04,0xc2d7ffa7,0xb5d0cf31,0x2cd99e8b,0x5bdeae1d,
	0x9b64c2b0,0xec63f226,0x756aa39c,0x026d930a,0x9c0906a9,0xeb0e363f,0x72076785,0x05005713,
	0x95bf4a82,0xe2b87a14,0x7bb12bae,0x0cb61b38,0x92d28e9b,0xe5d5be0d,0x7cdcefb7,0x0bdbdf21,
	0x86d3d2d4,0xf1d4e242,0x68ddb3f8,0x1fda836e,0x81be16cd,0xf6b9265b,0x6fb077e1,0x18b74777,
	0x88085ae6,0xff0f6a70,0x66063bca,0x11010b5c,0x8f659eff,0xf862ae69,0x616bffd3,0x166ccf45,
	0xa00ae278,0xd70dd2ee,0x4e048354,0x3903b3c2,0xa7672661,0xd06016f7,0x4969474d,0x3e6e77db,
	0xaed16a4a,0xd9d65adc,0x40df0b66,0x37d83bf0,0xa9bcae53,0xdebb9ec5,0x47b2cf7f,0x30b5ffe9,
	0xbdbdf21c,0xcabac28a,0x53b39330,0x24b4a3a6,0xbad03605,0xcdd70693,0x54de5729,0x23d967bf,
	0xb3667a2e,0xc4614ab8,0x5d681b02,0x2a6f2b94,0xb40bbe37,0xc30c8ea1,0x5a05df1b,0x2d02ef8d,
};



extern filesystem_type_t FILESYSTEM_TYPE_KFS2;



static u32 _calculate_crc(const void* data,u32 length){
	const u8* ptr=data;
	u32 out=0xffffffff;
	for (u32 i=0;i<length;i++){
		out=_kfs2_crc_table[(out&0xff)^ptr[i]]^(out>>8);
	}
	return ~out;
}



static inline _Bool _verify_crc(const void* data,u32 length){
	return _calculate_crc(data,length-4)==*((u32*)(data+length-4));
}



static vfs2_node_t* _load_inode(filesystem2_t* fs,const vfs2_node_name_t* name,u32 inode){
	u8 buffer[4096];
	partition2_t* partition=fs->partition;
	drive2_t* drive=partition->drive;
	kfs2_fs_extra_data_t* extra_data=fs->extra_data;
	if (drive->read_write(drive->extra_data,partition->start_lba+((extra_data->root_block.first_inode_block+KFS2_INODE_GET_BLOCK_INDEX(inode))<<extra_data->block_size_shift),buffer,1<<extra_data->block_size_shift)!=(1<<extra_data->block_size_shift)){
		return NULL;
	}
	kfs2_node_t* node=(void*)(buffer+KFS2_INODE_GET_NODE_INDEX(inode)*sizeof(kfs2_node_t));
	if (!_verify_crc(node,sizeof(kfs2_node_t))){
		return NULL;
	}
	kfs2_vfs_node_t* out=(kfs2_vfs_node_t*)vfs2_node_create(fs,name);
	if ((node->flags&KFS2_INODE_TYPE_MASK)==KFS2_INODE_TYPE_DIRECTORY){
		out->node.flags=(out->node.flags&(~VFS2_NODE_TYPE_MASK))|VFS2_NODE_TYPE_DIRECTORY;
	}
	out->kfs2_node=*node;
	out->kfs2_node._inode=inode;
	return (vfs2_node_t*)out;
}



static void _node_get_chunk_at_offset(kfs2_vfs_node_t* node,u64 offset,kfs2_data_chunk_t* out){
	kfs2_fs_extra_data_t* extra_data=node->node.fs->extra_data;
	partition2_t* partition=node->node.fs->partition;
	drive2_t* drive=partition->drive;
	switch (node->kfs2_node.flags&KFS2_INODE_STORAGE_MASK){
		case KFS2_INODE_STORAGE_TYPE_INLINE:
			if (offset>=48){
				panic("_node_get_chunk_at_offset: invalid offset");
			}
			out->offset=0;
			out->data=node->kfs2_node.data.inline_;
			out->length=48;
			break;
		case KFS2_INODE_STORAGE_TYPE_SINGLE:
			{
				u64 index=offset/KFS2_BLOCK_SIZE;
				if (index>=6){
					panic("_node_get_chunk_at_offset: invalid offset");
				}
				if (!out->data){
					out->data=(void*)(pmm_alloc(1,PMM_COUNTER_KFS2_CHUNK,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
				}
				out->offset=offset&(-KFS2_BLOCK_SIZE);
				if (drive->read_write(drive->extra_data,partition->start_lba+((extra_data->root_block.first_data_block+node->kfs2_node.data.single[index])<<extra_data->block_size_shift),out->data,1<<extra_data->block_size_shift)!=(1<<extra_data->block_size_shift)){
					panic("_node_get_chunk_at_offset: I/O error");
				}
				out->length=KFS2_BLOCK_SIZE;
				break;
			}
		case KFS2_INODE_STORAGE_TYPE_DOUBLE:
			panic("KFS2_INODE_STORAGE_TYPE_DOUBLE");
			break;
		case KFS2_INODE_STORAGE_TYPE_TRIPLE:
			panic("KFS2_INODE_STORAGE_TYPE_TRIPLE");
			break;
		case KFS2_INODE_STORAGE_TYPE_QUADRUPLE:
			panic("KFS2_INODE_STORAGE_TYPE_QUADRUPLE");
			break;
	}
}



static void _node_dealloc_chunk(kfs2_data_chunk_t* chunk){
	if (chunk->data&&chunk->length==KFS2_BLOCK_SIZE){
		pmm_dealloc(((u64)(chunk->data))-VMM_HIGHER_HALF_ADDRESS_OFFSET,1,PMM_COUNTER_KFS2_CHUNK);
	}
	chunk->offset=0;
	chunk->data=NULL;
	chunk->length=0;
}



static vfs2_node_t* _kfs2_create(void){
	kfs2_vfs_node_t* out=omm_alloc(&_kfs2_vfs_node_allocator);
	out->kfs2_node._inode=0xffffffff;
	return (vfs2_node_t*)out;
}



static void _kfs2_delete(vfs2_node_t* node){
	omm_dealloc(&_kfs2_vfs_node_allocator,node);
}



static vfs2_node_t* _kfs2_lookup(vfs2_node_t* node,const vfs2_node_name_t* name){
	kfs2_vfs_node_t* kfs2_node=(kfs2_vfs_node_t*)node;
	if (kfs2_node->kfs2_node._inode==0xffffffff||!kfs2_node->kfs2_node.size){
		return NULL;
	}
	kfs2_data_chunk_t chunk={
		0,
		NULL,
		0
	};
	u64 offset=0;
	while (offset<kfs2_node->kfs2_node.size){
		if (offset-chunk.offset>=chunk.length){
			_node_get_chunk_at_offset(kfs2_node,offset,&chunk);
		}
		kfs2_directory_entry_t* entry=(kfs2_directory_entry_t*)(chunk.data+offset-chunk.offset);
		if (entry->name_length!=name->length){
			goto _skip_entry;
		}
		for (u16 i=0;i<name->length;i++){
			if (entry->name[i]!=name->data[i]){
				goto _skip_entry;
			}
		}
		_node_dealloc_chunk(&chunk);
		return _load_inode(node->fs,name,entry->inode);
_skip_entry:
		offset+=entry->size;
	}
	_node_dealloc_chunk(&chunk);
	return NULL;
}



static _Bool _kfs2_link(vfs2_node_t* node,vfs2_node_t* parent){
	panic("_kfs2_link");
}



static _Bool _kfs2_unlink(vfs2_node_t* node){
	panic("_kfs2_unlink");
}



static s64 _kfs2_read(vfs2_node_t* node,u64 offset,void* buffer,u64 size){
	kfs2_vfs_node_t* kfs2_node=(kfs2_vfs_node_t*)node;
	if (kfs2_node->kfs2_node._inode==0xffffffff||offset>=kfs2_node->kfs2_node.size){
		return 0;
	}
	if (offset+size>=kfs2_node->kfs2_node.size){
		size=kfs2_node->kfs2_node.size-offset;
	}
	if (!size){
		return 0;
	}
	u64 out=size;
	kfs2_data_chunk_t chunk={
		0,
		NULL,
		0
	};
	size+=offset;
	while (offset<size){
		if (offset-chunk.offset>=chunk.length){
			_node_get_chunk_at_offset(kfs2_node,offset,&chunk);
		}
		u64 padding=offset-chunk.offset;
		memcpy(buffer,chunk.data+padding,chunk.length-padding);
		offset+=chunk.length-padding;
	}
	_node_dealloc_chunk(&chunk);
	return out;
}



static s64 _kfs2_write(vfs2_node_t* node,u64 offset,const void* buffer,u64 size){
	panic("_kfs2_write");
}



static s64 _kfs2_resize(vfs2_node_t* node,s64 size,u32 flags){
	panic("_kfs2_resize");
}



static void _kfs2_flush(vfs2_node_t* node){
	panic("_kfs2_flush");
}



static vfs2_functions_t _kfs2_functions={
	_kfs2_create,
	_kfs2_delete,
	_kfs2_lookup,
	_kfs2_link,
	_kfs2_unlink,
	_kfs2_read,
	_kfs2_write,
	_kfs2_resize,
	_kfs2_flush
};



static void _kfs2_fs_deinit(filesystem2_t* fs){
	omm_dealloc(&_kfs2_fs_extra_data_allocator,fs->extra_data);
	panic("_kfs2_deinit_callback");
}



static filesystem2_t* _kfs2_fs_load(partition2_t* partition){
	drive2_t* drive=partition->drive;
	if (drive->block_size>4096){
		return NULL;
	}
	u8 buffer[4096];
	if (drive->read_write(drive->extra_data,partition->start_lba,buffer,1)!=1){
		return NULL;
	}
	kfs2_root_block_t* root_block=(kfs2_root_block_t*)buffer;
	if (root_block->signature!=KFS2_ROOT_BLOCK_SIGNATURE||!_verify_crc(root_block,sizeof(kfs2_root_block_t))){
		return NULL;
	}
	kfs2_fs_extra_data_t* extra_data=omm_alloc(&_kfs2_fs_extra_data_allocator);
	extra_data->root_block=*root_block;
	extra_data->block_size_shift=63-__builtin_clzll(KFS2_BLOCK_SIZE/drive->block_size);
	filesystem2_t* out=fs_create(FILESYSTEM_TYPE_KFS2);
	out->functions=&_kfs2_functions;
	out->partition=partition;
	out->extra_data=extra_data;
	vfs2_node_name_t* root_name=vfs2_name_alloc("<root>",0);
	out->root=_load_inode(out,root_name,0);
	vfs2_name_dealloc(root_name);
	out->root->flags|=VFS2_NODE_FLAG_PERMANENT;
	return out;
}



FILESYSTEM_DECLARE_TYPE(
	KFS2,
	_kfs2_fs_deinit,
	_kfs2_fs_load
);
