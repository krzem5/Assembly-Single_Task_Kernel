import binascii
import struct



KFS2_SIGNATURE=0x544f4f523253464b
KFS2_BLOCKS_PER_INODE=64
KFS2_INODE_SIZE=128
KFS2_MAX_INODES=0x100000000
KFS2_BITMAP_LEVEL_COUNT=5
KFS2_MAX_DISK_SIZE=0x1000000000000



__all__=["KFS2FileBackend","format_partition"]



ffs=lambda x:(x&(-x)).bit_length()-1



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



class KFS2FileBackend(object):
	def __init__(self,file_path,block_size,start,end):
		self.block_size=block_size
		self.start=start*block_size
		self.end=end*block_size
		self.block_count=end-start
		if (self.block_count*block_size>KFS2_MAX_DISK_SIZE):
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
	offset=root_block.first_bitmap_block*backend.block_size
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



def format_partition(backend):
	inode_block_count=backend.block_count//KFS2_BLOCKS_PER_INODE
	inode_count=backend.block_size//KFS2_INODE_SIZE*inode_block_count
	if (inode_count>KFS2_MAX_INODES):
		inode_count=KFS2_MAX_INODES
	inode_allocation_bitmap=_compute_bitmap(inode_count)
	inode_allocation_bitmap_entry_count=sum(inode_allocation_bitmap)
	first_inode_block=1
	first_data_block=first_inode_block+inode_block_count
	data_block_allocation_bitmap=_compute_bitmap(backend.block_count-first_data_block)
	data_block_allocation_bitmap_entry_count=sum(data_block_allocation_bitmap)
	first_bitmap_block=backend.block_count-((inode_allocation_bitmap_entry_count+data_block_allocation_bitmap_entry_count)*8+backend.block_size-1)//backend.block_size
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
	_init_bitmap(backend,first_bitmap_block*backend.block_size,inode_count,inode_allocation_bitmap,inode_allocation_bitmap_offsets)
	_init_bitmap(backend,first_bitmap_block*backend.block_size,data_block_count,data_block_allocation_bitmap,data_block_allocation_bitmap_offsets)
	if (_alloc_inode(backend,root_block)):
		raise RuntimeError
	######################################
	root_block.kernel_inode=1
	root_block.save()
	######################################
