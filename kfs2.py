import binascii
import struct



KFS2_SIGNATURE=0x544f4f523253464b
KFS2_BLOCKS_PER_INODE=64
KFS2_INODE_SIZE=64
KFS2_MAX_INODES=0x100000000
KFS2_BITMAP_LEVEL_COUNT=5
KFS2_MAX_DISK_SIZE=0x1000000000000
KFS2_BLOCK_SIZE=4096

KFS2_INODE_FLAG_FILE=0x0000
KFS2_INODE_FLAG_DIRECTORY=0x0001

KFS2_INODE_STORAGE_MASK=0x0006
KFS2_INODE_STORAGE_TYPE_INLINE=0x0000
KFS2_INODE_STORAGE_TYPE_SINGLE=0x0002
KFS2_INODE_STORAGE_TYPE_DOUBLE=0x0004
KFS2_INODE_STORAGE_TYPE_TRIPLE=0x0006



__all__=["KFS2FileBackend","format_partition","get_or_create_file"]



ffs=lambda x:(x&(-x)).bit_length()-1



class KFS2FileBackend(object):
	def __init__(self,file_path,block_size,start,end):
		self.start=start*block_size
		if (self.start&(KFS2_BLOCK_SIZE-1)):
			raise RuntimeError("Wrong Alignment")
		self.end=(end*block_size)&(-KFS2_BLOCK_SIZE)
		self.block_count=(self.end-self.start)//KFS2_BLOCK_SIZE
		if (self.block_count*KFS2_BLOCK_SIZE>KFS2_MAX_DISK_SIZE):
			raise RuntimeError("Partition too large")
		self._file=open(file_path,"r+b")

	def close(self):
		self._file.close()

	def read(self,length):
		if (self._file.tell()+length>=self.end):
			raise RuntimeError
		return self._file.read(length)

	def write(self,data):
		if (self._file.tell()+len(data)>=self.end):
			raise RuntimeError
		self._file.write(data)

	def seek(self,offset):
		offset+=self.start
		if (offset>=self.end):
			raise RuntimeError
		self._file.seek(offset)

	def read_u64(self):
		return struct.unpack("<Q",self.read(8))[0]

	def write_u64(self,value):
		self.write(struct.pack("<Q",value))



class KFS2RootBlock(object):
	def __init__(self,backend,block_count,inode_count,data_block_count,first_inode_block,first_data_block,first_bitmap_block,inode_allocation_bitmap_offsets,data_block_allocation_bitmap_offsets,inode_allocation_bitmap_highest_level_length,data_block_allocation_bitmap_highest_level_length,kernel_inode):
		self._backend=backend
		self.block_count=block_count
		self.inode_count=inode_count
		self.data_block_count=data_block_count
		self.first_inode_block=first_inode_block
		self.first_data_block=first_data_block
		self.first_bitmap_block=first_bitmap_block
		self.inode_allocation_bitmap_offsets=inode_allocation_bitmap_offsets
		self.data_block_allocation_bitmap_offsets=data_block_allocation_bitmap_offsets
		self.inode_allocation_bitmap_highest_level_length=inode_allocation_bitmap_highest_level_length
		self.data_block_allocation_bitmap_highest_level_length=data_block_allocation_bitmap_highest_level_length
		self.kernel_inode=kernel_inode

	def save(self):
		header=struct.pack(f"<QQQQQQQ{KFS2_BITMAP_LEVEL_COUNT}Q{KFS2_BITMAP_LEVEL_COUNT}QHHI",
			KFS2_SIGNATURE,
			self.block_count,
			self.inode_count,
			self.data_block_count,
			self.first_inode_block,
			self.first_data_block,
			self.first_bitmap_block,
			*self.inode_allocation_bitmap_offsets,
			*self.data_block_allocation_bitmap_offsets,
			self.inode_allocation_bitmap_highest_level_length,
			self.data_block_allocation_bitmap_highest_level_length,
			self.kernel_inode
		)
		self._backend.seek(0)
		self._backend.write(header+struct.pack("<I",binascii.crc32(header)))

	@staticmethod
	def load(backend):
		backend.seek(0)
		header=backend.read(148)
		crc=binascii.crc32(header[:-4])
		data=struct.unpack(f"<QQQQQQQ{KFS2_BITMAP_LEVEL_COUNT}Q{KFS2_BITMAP_LEVEL_COUNT}QHHII",header)
		if (data[0]!=KFS2_SIGNATURE or data[7+2*KFS2_BITMAP_LEVEL_COUNT+3]!=crc):
			raise RuntimeError
		return KFS2RootBlock(
			backend,
			data[1],
			data[2],
			data[3],
			data[4],
			data[5],
			data[6],
			data[7:7+KFS2_BITMAP_LEVEL_COUNT],
			data[7+KFS2_BITMAP_LEVEL_COUNT:7+2*KFS2_BITMAP_LEVEL_COUNT],
			data[7+2*KFS2_BITMAP_LEVEL_COUNT],
			data[7+2*KFS2_BITMAP_LEVEL_COUNT+1],
			data[7+2*KFS2_BITMAP_LEVEL_COUNT+2]
		)



class KFS2Node(object):
	def __init__(self,backend,offset,size,hard_link_count,flags,data):
		self._backend=backend
		self._offset=offset
		self.size=size
		self.hard_link_count=hard_link_count
		self.flags=flags
		self.data=bytearray(data)

	def save(self):
		header=struct.pack(f"<Q48sHH",
			self.size,
			self.data,
			self.hard_link_count,
			self.flags
		)
		self._backend.seek(self._offset)
		self._backend.write(header+struct.pack("<I",binascii.crc32(header)))

	@staticmethod
	def load(backend,offset):
		backend.seek(offset)
		header=backend.read(KFS2_INODE_SIZE)
		crc=binascii.crc32(header[:-4])
		data=struct.unpack(f"<Q48sHHI",header)
		if (data[4]!=crc):
			raise RuntimeError
		return KFS2Node(
			backend,
			offset,
			data[0],
			data[2],
			data[3],
			data[1]
		)

	@staticmethod
	def index_to_offset(root_block,index):
		return root_block.first_inode_block*KFS2_BLOCK_SIZE+index*KFS2_INODE_SIZE



class KFS2DirectoryEntry(object):
	def __init__(self,inode,size,name_length,type,name):
		self.inode=inode
		self.size=size
		self.name_length=name_length
		self.type=type
		self.name=name

	def encode(self):
		if (self.size&3):
			raise RuntimeError
		return struct.pack(f"<IHBB{self.name_length}s{self.size-self.name_length-8}x",
			self.inode,
			self.size,
			self.name_length,
			self.type,
			self.name
		)

	@staticmethod
	def decode(data):
		inode,size,name_length,type=struct.unpack("<IHBB",data[:8])
		if (size&3):
			raise RuntimeError
		return KFS2DirectoryEntry(inode,size,name_length,type,data[8:8+name_length])

	@staticmethod
	def get_entry_size_for_name(name):
		return (8+len(name)+3)&0xffc



def _compute_bitmap(size):
	out=[]
	for i in range(0,KFS2_BITMAP_LEVEL_COUNT):
		size=(size+63)>>6
		out.append(size)
	return out



def _compute_bitmap_offsets(inode_allocation_bitmap,data_block_allocation_bitmap):
	offset=0
	inode_allocation_bitmap_offsets=[]
	data_block_allocation_bitmap_offsets=[]
	for i in range(0,KFS2_BITMAP_LEVEL_COUNT):
		inode_allocation_bitmap_offsets.append(offset)
		offset+=inode_allocation_bitmap[i]
	for i in range(0,KFS2_BITMAP_LEVEL_COUNT):
		data_block_allocation_bitmap_offsets.append(offset)
		offset+=data_block_allocation_bitmap[i]
	return inode_allocation_bitmap_offsets,data_block_allocation_bitmap_offsets



def _init_bitmap(backend,base_offset,count,bitmap,bitmap_offsets):
	for i in range(0,KFS2_BITMAP_LEVEL_COUNT):
		backend.seek(base_offset+bitmap_offsets[i]*8)
		new_count=0
		for j in range(0,bitmap[i]):
			mask=0
			if (count>=64):
				mask=0xffffffffffffffff
				new_count=j
				count-=64
			elif (count):
				mask=(1<<count)-1
				new_count=j
				count=0
			backend.write(struct.pack("<Q",mask))
		count=new_count+1



def _alloc_inode(backend,root_block):
	offset=root_block.first_bitmap_block*KFS2_BLOCK_SIZE
	backend.seek(offset+root_block.inode_allocation_bitmap_offsets[KFS2_BITMAP_LEVEL_COUNT-1])
	idx=0
	while (True):
		mask=backend.read_u64()
		if (mask):
			idx=ffs(mask)|(idx<<6)
			break
		idx+=1
		if (idx==root_block.inode_allocation_bitmap_highest_level_length):
			raise RuntimeError
	for i in range(KFS2_BITMAP_LEVEL_COUNT-2,-1,-1):
		backend.seek(offset+root_block.inode_allocation_bitmap_offsets[i]+(idx<<3))
		idx=ffs(backend.read_u64())|(idx<<6)
	out=idx
	for i in range(0,KFS2_BITMAP_LEVEL_COUNT):
		idx>>=6
		k=offset+root_block.inode_allocation_bitmap_offsets[i]+(idx<<3)
		backend.seek(k)
		mask=backend.read_u64()
		mask&=mask-1
		backend.seek(k)
		backend.write_u64(mask)
		if (mask):
			break
	return out



def _init_node_as_file(backend,root_block,inode):
	out=KFS2Node(
		backend,
		KFS2Node.index_to_offset(root_block,inode),
		0,
		1,
		KFS2_INODE_FLAG_FILE|KFS2_INODE_STORAGE_TYPE_INLINE,
		[0]*48
	)
	return out



def _init_node_as_directory(backend,root_block,inode):
	out=KFS2Node(
		backend,
		KFS2Node.index_to_offset(root_block,inode),
		1,
		1,
		KFS2_INODE_FLAG_DIRECTORY|KFS2_INODE_STORAGE_TYPE_INLINE,
		KFS2DirectoryEntry(0,48,0,0,b"").encode()
	)
	return out



def format_partition(backend):
	inode_block_count=backend.block_count//KFS2_BLOCKS_PER_INODE
	inode_count=KFS2_BLOCK_SIZE//KFS2_INODE_SIZE*inode_block_count
	if (inode_count>KFS2_MAX_INODES):
		inode_count=KFS2_MAX_INODES
	inode_allocation_bitmap=_compute_bitmap(inode_count)
	inode_allocation_bitmap_entry_count=sum(inode_allocation_bitmap)
	first_inode_block=1
	first_data_block=first_inode_block+inode_block_count
	data_block_allocation_bitmap=_compute_bitmap(backend.block_count-first_data_block)
	data_block_allocation_bitmap_entry_count=sum(data_block_allocation_bitmap)
	first_bitmap_block=backend.block_count-((inode_allocation_bitmap_entry_count+data_block_allocation_bitmap_entry_count)*8+KFS2_BLOCK_SIZE-1)//KFS2_BLOCK_SIZE
	data_block_count=first_bitmap_block-first_data_block
	inode_allocation_bitmap_offsets,data_block_allocation_bitmap_offsets=_compute_bitmap_offsets(inode_allocation_bitmap,data_block_allocation_bitmap)
	root_block=KFS2RootBlock(
		backend,
		backend.block_count,
		inode_count,
		data_block_count,
		first_inode_block,
		first_data_block,
		first_bitmap_block,
		inode_allocation_bitmap_offsets,
		data_block_allocation_bitmap_offsets,
		inode_allocation_bitmap[KFS2_BITMAP_LEVEL_COUNT-1],
		data_block_allocation_bitmap[KFS2_BITMAP_LEVEL_COUNT-1],
		0
	)
	root_block.save()
	_init_bitmap(backend,first_bitmap_block*KFS2_BLOCK_SIZE,inode_count,inode_allocation_bitmap,inode_allocation_bitmap_offsets)
	_init_bitmap(backend,first_bitmap_block*KFS2_BLOCK_SIZE,data_block_count,data_block_allocation_bitmap,data_block_allocation_bitmap_offsets)
	if (_alloc_inode(backend,root_block)):
		raise RuntimeError
	_init_node_as_directory(backend,root_block,0).save()



def get_or_create_file(backend,path):
	root_block=KFS2RootBlock.load(backend)
	node=KFS2Node.load(backend,KFS2Node.index_to_offset(root_block,0))
	out=0
	path=path.split("/")
	for i in range(0,len(path)):
		name=path[i]
		if (not name):
			continue
		if (not (node.flags&KFS2_INODE_FLAG_DIRECTORY)):
			raise RuntimeError("Non-directory does not have children")
		storage=node.flags&KFS2_INODE_STORAGE_MASK
		new_entry_size=KFS2DirectoryEntry.get_entry_size_for_name(name)
		best_entry_padding=0xffffffff
		best_entry_offset=None
		offset=0
		child_found=False
		if (storage==KFS2_INODE_STORAGE_TYPE_INLINE):
			for j in range(0,node.size):
				entry=KFS2DirectoryEntry.decode(node.data[offset:])
				if (entry.name_length==0):
					padding=entry.size-new_entry_size
					if (best_entry_padding==0xffffffff or (padding>=0 and padding<best_entry_padding)):
						best_entry_padding=padding
						best_entry_offset=offset
				elif (entry.name==name):
					node=KFS2Node.load(backend,KFS2Node.index_to_offset(root_block,entry.inode))
					out=entry.inode
					child_found=True
					break
				offset+=entry.size
		elif (storage==KFS2_INODE_STORAGE_TYPE_SINGLE):
			raise RuntimeError("KFS2_INODE_STORAGE_TYPE_SINGLE")
		elif (storage==KFS2_INODE_STORAGE_TYPE_DOUBLE):
			raise RuntimeError("KFS2_INODE_STORAGE_TYPE_DOUBLE")
		else:
			raise RuntimeError("KFS2_INODE_STORAGE_TYPE_TRIPLE")
		if (child_found):
			continue
		child_inode=_alloc_inode(backend,root_block)
		if (best_entry_padding==0xffffffff):
			raise RuntimeError("Resize inode data")
		type=(KFS2_INODE_FLAG_FILE if i==len(path)-1 else KFS2_INODE_FLAG_DIRECTORY)
		if (best_entry_padding<12):
			new_entry_size+=best_entry_padding
			best_entry_padding=0
		if (storage==KFS2_INODE_STORAGE_TYPE_INLINE):
			node.data[best_entry_offset:best_entry_offset+new_entry_size]=KFS2DirectoryEntry(child_inode,new_entry_size,len(name),type,bytes(name,"utf-8")).encode()
			if (best_entry_padding):
				node.data[best_entry_offset+new_entry_size:best_entry_offset+new_entry_size+best_entry_padding]=KFS2DirectoryEntry(child_inode,best_entry_padding,0,0,b"").encode()
		elif (storage==KFS2_INODE_STORAGE_TYPE_SINGLE):
			raise RuntimeError("KFS2_INODE_STORAGE_TYPE_SINGLE")
		elif (storage==KFS2_INODE_STORAGE_TYPE_DOUBLE):
			raise RuntimeError("KFS2_INODE_STORAGE_TYPE_DOUBLE")
		else:
			raise RuntimeError("KFS2_INODE_STORAGE_TYPE_TRIPLE")
		if (type==KFS2_INODE_FLAG_DIRECTORY):
			node=_init_node_as_directory(backend,root_block,child_inode)
		else:
			node=_init_node_as_file(backend,root_block,child_inode)
		node.save()
		out=child_inode
	return out
