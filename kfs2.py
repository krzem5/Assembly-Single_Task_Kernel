import array
import binascii
import struct



KFS2_SIGNATURE=0x544f4f523253464b
KFS2_BLOCKS_PER_INODE=64
KFS2_INODE_SIZE=64
KFS2_MAX_INODES=0xffffffff
KFS2_BITMAP_LEVEL_COUNT=5
KFS2_MAX_DISK_SIZE=0x1000000000000
KFS2_BLOCK_SIZE=4096

KFS2_INODE_TYPE_FILE=0x0000
KFS2_INODE_TYPE_DIRECTORY=0x0001
KFS2_INODE_TYPE_MASK=0x0001

KFS2_INODE_STORAGE_MASK=0x000e
KFS2_INODE_STORAGE_TYPE_INLINE=0x0000
KFS2_INODE_STORAGE_TYPE_SINGLE=0x0002
KFS2_INODE_STORAGE_TYPE_DOUBLE=0x0004
KFS2_INODE_STORAGE_TYPE_TRIPLE=0x0006
KFS2_INODE_STORAGE_TYPE_QUADRUPLE=0x0008



__all__=["KFS2FileBackend","format_partition","get_inode","set_file_content","set_kernel_inode"]



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
		return bytearray(self._file.read(length))

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
	def __init__(self,backend,block_count,inode_count,data_block_count,first_inode_block,first_data_block,first_bitmap_block,inode_allocation_bitmap_offsets,data_block_allocation_bitmap_offsets,inode_allocation_bitmap_highest_level_length,data_block_allocation_bitmap_highest_level_length,kernel_inode,initramfs_inode):
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
		self.initramfs_inode=initramfs_inode

	def save(self):
		header=struct.pack(f"<QQQQQQQ{KFS2_BITMAP_LEVEL_COUNT}Q{KFS2_BITMAP_LEVEL_COUNT}QHHII",
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
			self.kernel_inode,
			self.initramfs_inode
		)
		self._backend.seek(0)
		self._backend.write(header+struct.pack("<I",binascii.crc32(header)))

	@staticmethod
	def load(backend):
		backend.seek(0)
		header=backend.read(152)
		crc=binascii.crc32(header[:-4])
		data=struct.unpack(f"<QQQQQQQ{KFS2_BITMAP_LEVEL_COUNT}Q{KFS2_BITMAP_LEVEL_COUNT}QHHIII",header)
		if (data[0]!=KFS2_SIGNATURE or data[7+2*KFS2_BITMAP_LEVEL_COUNT+4]!=crc):
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
			data[7+2*KFS2_BITMAP_LEVEL_COUNT+2],
			data[7+2*KFS2_BITMAP_LEVEL_COUNT+3]
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



class KFS2NodeDataProviderChunk(object):
	def __init__(self,offset,data,length):
		self.offset=offset
		self.data=data
		self.length=length



class KFS2NodeDataProvider(object):
	def __init__(self,backend,root_block,node,flags=None,node_data=None):
		self._backend=backend
		self._data_offset=(root_block.first_data_block if isinstance(root_block,KFS2RootBlock) else root_block)*KFS2_BLOCK_SIZE
		self.node=node
		self.flags=(flags if flags is not None else node.flags)
		self.data=(node_data if node_data is not None else node.data)

	def resize(self,size):
		if (self.node.size):
			old_data_provider=KFS2NodeDataProvider(self._backend,self._data_offset,None,self.flags,self.data)
			raise RuntimeError("Unimplemented")
		root_block=KFS2RootBlock.load(self._backend)
		self.node.size=size
		self.node.flags&=~KFS2_INODE_STORAGE_MASK
		if (size<=48):
			self.node.flags|=KFS2_INODE_STORAGE_TYPE_INLINE
			return
		if (size<=6*KFS2_BLOCK_SIZE):
			self.node.flags|=KFS2_INODE_STORAGE_TYPE_SINGLE
			raise RuntimeError("Init KFS2_INODE_STORAGE_TYPE_SINGLE")
		if (size<=KFS2_BLOCK_SIZE**2//8):
			self.node.flags|=KFS2_INODE_STORAGE_TYPE_DOUBLE
			buffer=array.array("Q")
			for i in range(0,KFS2_BLOCK_SIZE//8):
				if (i*KFS2_BLOCK_SIZE<size):
					buffer.append(_alloc_data_block(self._backend,root_block))
				else:
					buffer.append(0)
			data_block=_alloc_data_block(self._backend,root_block)
			self._backend.seek(self._data_offset+data_block*KFS2_BLOCK_SIZE)
			self._backend.write(buffer.tobytes())
			self.data[:8]=struct.pack("<Q",data_block)
			return
		if (size<=KFS2_BLOCK_SIZE**3//64):
			self.node.flags|=KFS2_INODE_STORAGE_TYPE_TRIPLE
			raise RuntimeError("Init KFS2_INODE_STORAGE_TYPE_TRIPLE")
		if (size<=KFS2_BLOCK_SIZE**4//512):
			self.node.flags|=KFS2_INODE_STORAGE_TYPE_QUADRUPLE
			raise RuntimeError("Init KFS2_INODE_STORAGE_TYPE_QUADRUPLE")
		raise RuntimeError("size too large")

	def get_chunk_at_offset(self,offset):
		if (offset>=self.node.size):
			return None
		storage=self.node.flags&KFS2_INODE_STORAGE_MASK
		if (storage==KFS2_INODE_STORAGE_TYPE_INLINE):
			if (offset>=48):
				return None
			return KFS2NodeDataProviderChunk(0,self.data,48)
		elif (storage==KFS2_INODE_STORAGE_TYPE_SINGLE):
			raise RuntimeError("KFS2_INODE_STORAGE_TYPE_SINGLE")
		elif (storage==KFS2_INODE_STORAGE_TYPE_DOUBLE):
			index=offset//KFS2_BLOCK_SIZE
			if (index>=KFS2_BLOCK_SIZE//8):
				return None
			self._backend.seek(self._data_offset+struct.unpack("<Q",self.data[:8])[0]*KFS2_BLOCK_SIZE+index*8)
			self._backend.seek(self._data_offset+self._backend.read_u64()*KFS2_BLOCK_SIZE)
			return KFS2NodeDataProviderChunk(index*KFS2_BLOCK_SIZE,self._backend.read(KFS2_BLOCK_SIZE),KFS2_BLOCK_SIZE)
		elif (storage==KFS2_INODE_STORAGE_TYPE_TRIPLE):
			raise RuntimeError("KFS2_INODE_STORAGE_TYPE_TRIPLE")
		else:
			raise RuntimeError("KFS2_INODE_STORAGE_TYPE_QUADRUPLE")

	def save_chunk(self,chunk):
		if (chunk.offset>=self.node.size):
			raise RuntimeError("Wrong offset")
		storage=self.node.flags&KFS2_INODE_STORAGE_MASK
		if (storage==KFS2_INODE_STORAGE_TYPE_INLINE):
			if (chunk.offset!=0):
				raise RuntimeError("Wrong offset")
			self.data[:]=chunk.data
		elif (storage==KFS2_INODE_STORAGE_TYPE_SINGLE):
			raise RuntimeError("KFS2_INODE_STORAGE_TYPE_SINGLE")
		elif (storage==KFS2_INODE_STORAGE_TYPE_DOUBLE):
			index=chunk.offset//KFS2_BLOCK_SIZE
			if (index>=KFS2_BLOCK_SIZE//8 or chunk.offset&(KFS2_BLOCK_SIZE-1)):
				raise RuntimeError("Wrong offset")
			self._backend.seek(self._data_offset+struct.unpack("<Q",self.data[:8])[0]*KFS2_BLOCK_SIZE+index*8)
			self._backend.seek(self._data_offset+self._backend.read_u64()*KFS2_BLOCK_SIZE)
			self._backend.write(chunk.data)
		elif (storage==KFS2_INODE_STORAGE_TYPE_TRIPLE):
			raise RuntimeError("KFS2_INODE_STORAGE_TYPE_TRIPLE")
		else:
			raise RuntimeError("KFS2_INODE_STORAGE_TYPE_QUADRUPLE")



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



def _alloc_data_block(backend,root_block):
	offset=root_block.first_bitmap_block*KFS2_BLOCK_SIZE
	backend.seek(offset+root_block.data_block_allocation_bitmap_offsets[KFS2_BITMAP_LEVEL_COUNT-1])
	idx=0
	while (True):
		mask=backend.read_u64()
		if (mask):
			idx=ffs(mask)|(idx<<6)
			break
		idx+=1
		if (idx==root_block.data_block_allocation_bitmap_highest_level_length):
			raise RuntimeError
	for i in range(KFS2_BITMAP_LEVEL_COUNT-2,-1,-1):
		backend.seek(offset+root_block.data_block_allocation_bitmap_offsets[i]+(idx<<3))
		idx=ffs(backend.read_u64())|(idx<<6)
	out=idx
	for i in range(0,KFS2_BITMAP_LEVEL_COUNT):
		idx>>=6
		k=offset+root_block.data_block_allocation_bitmap_offsets[i]+(idx<<3)
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
		KFS2_INODE_TYPE_FILE|KFS2_INODE_STORAGE_TYPE_INLINE,
		[0]*48
	)
	return out



def _init_node_as_directory(backend,root_block,inode):
	out=KFS2Node(
		backend,
		KFS2Node.index_to_offset(root_block,inode),
		1,
		1,
		KFS2_INODE_TYPE_DIRECTORY|KFS2_INODE_STORAGE_TYPE_INLINE,
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
		0,
		0
	)
	root_block.save()
	_init_bitmap(backend,first_bitmap_block*KFS2_BLOCK_SIZE,inode_count,inode_allocation_bitmap,inode_allocation_bitmap_offsets)
	_init_bitmap(backend,first_bitmap_block*KFS2_BLOCK_SIZE,data_block_count,data_block_allocation_bitmap,data_block_allocation_bitmap_offsets)
	if (_alloc_inode(backend,root_block)):
		raise RuntimeError
	_init_node_as_directory(backend,root_block,0).save()



def get_inode(backend,path):
	root_block=KFS2RootBlock.load(backend)
	node=KFS2Node.load(backend,KFS2Node.index_to_offset(root_block,0))
	out=0
	path=path.split("/")
	for i in range(0,len(path)):
		name=path[i]
		if (not name):
			continue
		if ((node.flags&KFS2_INODE_TYPE_MASK)!=KFS2_INODE_TYPE_DIRECTORY):
			raise RuntimeError("Non-directory does not have children")
		storage=node.flags&KFS2_INODE_STORAGE_MASK
		new_entry_size=KFS2DirectoryEntry.get_entry_size_for_name(name)
		best_entry_padding=0xffffffff
		best_entry_offset=None
		data_provider=KFS2NodeDataProvider(backend,root_block,node)
		offset=0
		child_found=False
		while (not child_found):
			chunk=data_provider.get_chunk_at_offset(offset)
			if (chunk is None):
				break
			while (offset<chunk.offset+chunk.length):
				entry=KFS2DirectoryEntry.decode(chunk.data[offset-chunk.offset:])
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
		if (child_found):
			continue
		child_inode=_alloc_inode(backend,root_block)
		if (best_entry_padding==0xffffffff):
			raise RuntimeError("Resize inode data")
		node.size+=1
		type=(KFS2_INODE_TYPE_FILE if i==len(path)-1 else KFS2_INODE_TYPE_DIRECTORY)
		if (best_entry_padding<12):
			new_entry_size+=best_entry_padding
			best_entry_padding=0
		chunk=data_provider.get_chunk_at_offset(best_entry_offset)
		best_entry_offset-=chunk.offset
		chunk.data[best_entry_offset:best_entry_offset+new_entry_size]=KFS2DirectoryEntry(child_inode,new_entry_size,len(name),type,bytes(name,"utf-8")).encode()
		if (best_entry_padding):
			chunk.data[best_entry_offset+new_entry_size:best_entry_offset+new_entry_size+best_entry_padding]=KFS2DirectoryEntry(child_inode,best_entry_padding,0,0,b"").encode()
		data_provider.save_chunk(chunk)
		if (type==KFS2_INODE_TYPE_DIRECTORY):
			node=_init_node_as_directory(backend,root_block,child_inode)
		else:
			node=_init_node_as_file(backend,root_block,child_inode)
		node.save()
		out=child_inode
	return out



def set_file_content(backend,inode,content):
	root_block=KFS2RootBlock.load(backend)
	if (inode>=root_block.inode_count):
		raise RuntimeError
	node=KFS2Node.load(backend,KFS2Node.index_to_offset(root_block,inode))
	data_provider=KFS2NodeDataProvider(backend,root_block,node)
	if (node.size<len(content)):
		data_provider.resize(len(content))
	offset=0
	while (offset<len(content)):
		chunk=data_provider.get_chunk_at_offset(offset)
		length=len(content)-offset
		if (length>chunk.length):
			length=chunk.length
		chunk.data[:length]=content[offset:offset+length]
		for i in range(length,chunk.length):
			chunk.data[i]=0
		data_provider.save_chunk(chunk)
		offset+=length
	node.save()



def set_kernel_inode(backend,kernel_inode):
	root_block=KFS2RootBlock.load(backend)
	if (kernel_inode>=root_block.inode_count):
		raise RuntimeError
	root_block.kernel_inode=kernel_inode
	root_block.save()



def set_initramfs_inode(backend,initramfs_inode):
	root_block=KFS2RootBlock.load(backend)
	if (initramfs_inode>=root_block.inode_count):
		raise RuntimeError
	root_block.initramfs_inode=initramfs_inode
	root_block.save()
