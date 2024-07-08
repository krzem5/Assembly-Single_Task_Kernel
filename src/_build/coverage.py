import array
import os
import struct



COVERAGE_FILE_REPORT_MARKER=0xb8bcbbbe41444347
COVERAGE_FILE_SUCCESS_MARKER=0xb0b4b0b44b4f4b4f

ARC_FLAG_ON_TREE=0x01



class SourceFile(object):
	def __init__(self,name):
		self.name=name
		self.functions={}



class CoverageFile(object):
	def __init__(self,name):
		self.name=name
		self.functions={}



class CoverageFunction(object):
	def __init__(self,lineno_checksum,cfg_checksum,name,src,start_line):
		self.lineno_checksum=lineno_checksum
		self.cfg_checksum=cfg_checksum
		self.name=name
		self.src=src
		self.start_line=start_line
		self.blocks=None
		self.counters=[]



class CoverageFunctionBlock(object):
	def __init__(self):
		self.lines={}
		self.prev=[]
		self.next=[]
		self.prev_count=0
		self.next_count=0
		self.count=-1



class CoverageFunctionBlockArc(object):
	def __init__(self,src,dst,flags):
		self.src=src
		self.dst=dst
		self.flags=flags
		self.count=-1



def __builtin_ffsll(x):
	return (x&-x).bit_length()



def generate(vm_output_file_path,output_file_path):
	success=False
	source_files={}
	coverage_files={}
	with open(vm_output_file_path,"rb") as rf:
		while (True):
			buffer=rf.read(8)
			if (len(buffer)<8):
				break
			marker=struct.unpack("<Q",buffer)[0]
			if (marker==COVERAGE_FILE_SUCCESS_MARKER):
				success=True
				continue
			if (marker!=COVERAGE_FILE_REPORT_MARKER):
				rf.seek(rf.tell()-7)
				continue
			version,checksum,file_name_length=struct.unpack("III",rf.read(12))
			file_name=rf.read(file_name_length).decode("utf-8")
			if (file_name in coverage_files):
				file=coverage_files[file_name]
			else:
				file=CoverageFile(file_name)
				coverage_files[file_name]=file
				with open(file_name[:-5]+".gcno","rb") as gcno_rf:
					gcno_rf.read(struct.unpack("16xI",gcno_rf.read(20))[0]+4)
					function=None
					while (True):
						buffer=gcno_rf.read(8)
						if (len(buffer)<8):
							break
						tag,length=struct.unpack("II",buffer)
						if (tag==0x01000000):
							ident,lineno_checksum,cfg_checksum=struct.unpack("III",gcno_rf.read(12))
							name=gcno_rf.read(struct.unpack("I",gcno_rf.read(4))[0]-1).decode("utf-8")
							gcno_rf.read(5)
							src=gcno_rf.read(struct.unpack("I",gcno_rf.read(4))[0]-1).decode("utf-8")
							gcno_rf.read(1)
							start_line=struct.unpack("I",gcno_rf.read(4))[0]
							gcno_rf.read(12)
							if (src not in source_files):
								source_files[src]=SourceFile(src)
							if (name in source_files[src].functions):
								fn=source_files[src].functions[name]
								if (fn.lineno_checksum!=lineno_checksum or fn.cfg_checksum!=cfg_checksum):
									raise RuntimeError("Duplicated function name")
								function=None
								file.functions[ident]=fn
							else:
								function=CoverageFunction(lineno_checksum,cfg_checksum,name,src,start_line)
								source_files[src].functions[name]=function
								file.functions[ident]=function
						elif (function is None):
							gcno_rf.seek(gcno_rf.tell()+length)
						elif (tag==0x01410000):
							function.blocks=[CoverageFunctionBlock() for _ in range(0,struct.unpack("I",gcno_rf.read(4))[0])]
						elif (tag==0x01430000):
							src=struct.unpack("I",gcno_rf.read(4))[0]
							function.blocks[src].index=src
							for i in range(0,(length//4-1)//2):
								dst,flags=struct.unpack("II",gcno_rf.read(8))
								if (not (flags&ARC_FLAG_ON_TREE)):
									function.counters.append(0)
								arc=CoverageFunctionBlockArc(src,dst,flags)
								function.blocks[src].next.append(arc)
								function.blocks[src].next_count+=1
								function.blocks[dst].prev.append(arc)
								function.blocks[dst].prev_count+=1
						elif (tag==0x01450000):
							block=function.blocks[struct.unpack("I",gcno_rf.read(4))[0]]
							current_src=function.src
							while (current_src):
								line=struct.unpack("I",gcno_rf.read(4))[0]
								if (not line):
									current_src=gcno_rf.read(struct.unpack("I",gcno_rf.read(4))[0])[:-1].decode("utf-8")
									continue
								if (current_src not in block.lines):
									block.lines[current_src]=[]
								block.lines[current_src].append(line)
						else:
							raise RuntimeError
			for i in range(0,struct.unpack("I",rf.read(4))[0]):
				ident,lineno_checksum,cfg_checksum,counter_count=struct.unpack("IIII",rf.read(16))
				function=file.functions[ident]
				if (function.lineno_checksum!=lineno_checksum or function.cfg_checksum!=cfg_checksum):
					raise RuntimeError("Duplicated identification number")
				counters=array.array("Q")
				counters.frombytes(rf.read(counter_count<<3))
				for i in range(0,counter_count):
					function.counters[i]+=counters[i]
	if (not success):
		sys.exit(1)
	for file in source_files.values():
		for function in file.functions.values():
			function.blocks[0].prev_count=0xffffffff
			function.blocks[1].next_count=0xffffffff
			counter_index=0
			LENGTH=(len(function.blocks)+63)>>6
			unsolved_blocks=[0 for _ in range(0,LENGTH)]
			solved_blocks=[0 for _ in range(0,LENGTH)]
			remaining_blocks=len(function.blocks)
			for src in range(0,len(function.blocks)):
				unsolved_blocks[src>>6]|=1<<(src&63)
				for arc in function.blocks[src].next:
					if (arc.flags&ARC_FLAG_ON_TREE):
						continue
					arc.count=function.counters[counter_index]
					counter_index+=1
					function.blocks[src].next_count-=1
					function.blocks[arc.dst].prev_count-=1
			has_processed_input=True
			while (has_processed_input):
				has_processed_input=False
				i=0
				while (i<LENGTH):
					if (not unsolved_blocks[i]):
						i+=1
						continue
					has_processed_input=True
					j=(i<<6)|(__builtin_ffsll(unsolved_blocks[i])-1)
					unsolved_blocks[i]&=unsolved_blocks[i]-1
					block=function.blocks[j]
					total=0
					if (not block.next_count):
						for arc in block.next:
							total+=arc.count
					elif (not block.prev_count):
						for arc in block.prev:
							total+=arc.count
					else:
						continue
					block.count=total
					solved_blocks[i]|=1<<(j&63)
				i=0
				while (i<LENGTH):
					if (not solved_blocks[i]):
						i+=1
						continue
					has_processed_input=True
					j=(i<<6)|(__builtin_ffsll(solved_blocks[i])-1)
					solved_blocks[i]&=solved_blocks[i]-1
					block=function.blocks[j]
					if (block.next_count==1):
						total=block.count
						inv_arc=None
						for arc in block.next:
							if (arc.count==-1):
								inv_arc=arc
							else:
								total-=arc.count
						inv_arc.count=total
						block.next_count=0
						dst_block=function.blocks[inv_arc.dst]
						dst_block.prev_count-=1
						if (dst_block.count!=-1 and dst_block.prev_count==1):
							solved_blocks[inv_arc.dst>>6]|=1<<(inv_arc.dst&63)
						elif (dst_block.count==-1 and not dst_block.prev_count):
							unsolved_blocks[inv_arc.dst>>6]|=1<<(inv_arc.dst&63)
					elif (block.prev_count==1):
						total=block.count
						inv_arc=None
						for arc in block.prev:
							if (arc.count==-1):
								inv_arc=arc
							else:
								total-=arc.count
						inv_arc.count=total
						block.prev_count=0
						src_block=function.blocks[inv_arc.src]
						src_block.next_count-=1
						if (src_block.count!=-1 and src_block.next_count==1):
							solved_blocks[inv_arc.src>>6]|=1<<(inv_arc.src&63)
						elif (src_block.count==-1 and not src_block.next_count):
							unsolved_blocks[inv_arc.src>>6]|=1<<(inv_arc.src&63)
			for block in function.blocks:
				if (block.count==-1):
					raise RuntimeError("Unsolved graph")
	out={}
	for file in source_files.values():
		for function in file.functions.values():
			for block in function.blocks:
				for src,lines in block.lines.items():
					if (src not in out):
						out[src]={}
					for line in lines:
						if (line not in out[src]):
							out[src][line]=block.count
						else:
							out[src][line]+=block.count
	with open(output_file_path,"w") as wf:
		wf.write("TN:\n")
		for src,lines in out.items():
			wf.write(f"SF:{src}\n")
			if (src in source_files):
				for function in source_files[src].functions.values():
					wf.write(f"FN:{function.start_line},{function.name}\n")
			for line,count in lines.items():
				wf.write(f"DA:{line},{count}\n")
