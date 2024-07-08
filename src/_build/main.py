import array
import binascii
import config
import hashlib
import initramfs
import linker
import os
import process_pool
import shlex
import signature
import struct
import subprocess
import sys
import test
import time



MODE_DEBUG=0
MODE_COVERAGE=1
MODE_RELEASE=2
mode=MODE_DEBUG
if ("--release" in sys.argv):
	mode=MODE_RELEASE
if ("--coverage" in sys.argv):
	mode=MODE_COVERAGE
if (len(sys.argv)>2 and sys.argv[1].startswith("__patch_")):
	mode={
		"debug": MODE_DEBUG,
		"coverage": MODE_COVERAGE,
		"release": MODE_RELEASE
	}[sys.argv[2]]



MODE_NAME={
	MODE_DEBUG: "debug",
	MODE_COVERAGE: "coverage",
	MODE_RELEASE: "release"
}[mode]
COVERAGE_FILE_REPORT_MARKER=0xb8bcbbbe41444347
COVERAGE_FILE_SUCCESS_MARKER=0xb0b4b0b44b4f4b4f



def option(name):
	if (not hasattr(option,"root")):
		root=config.parse(f"src/config/build_{MODE_NAME}.config")
		root.data.extend(config.parse("src/config/build_common.config").data)
		setattr(option,"root",root)
	try:
		out=option.root
		for part in name.split("."):
			out=next(out.find(part))
		return (out if out.type==config.CONFIG_TAG_TYPE_ARRAY else out.data)
	except StopIteration:
		return None



def _clear_if_obsolete(path,src_file_path,prefix):
	for file in os.listdir(path):
		if (not file.startswith(prefix) or os.path.exists(os.path.join(src_file_path,file[len(prefix):].split(".")[0]))):
			continue
		os.remove(os.path.join(path,file))



def _load_changed_files(hash_file_path,*file_directories):
	file_hash_list={}
	if (os.path.exists(hash_file_path)):
		with open(hash_file_path,"r") as rf:
			for line in rf.read().split("\n"):
				line=line.strip()
				if (not line):
					continue
				line=line.split(":")
				file_hash_list[line[0]]=line[1]
	changed_files=[]
	for file_directory in file_directories:
		for root,_,files in os.walk(file_directory):
			for file in files:
				file=os.path.join(root,file)
				with open(file,"rb") as rf:
					new_hash=hashlib.sha256(rf.read()).hexdigest()
				if (file not in file_hash_list or file_hash_list[file]!=new_hash):
					file_hash_list[file]=new_hash
					changed_files.append(file)
	return changed_files,file_hash_list



def _save_file_hash_list(file_hash_list,hash_file_path):
	with open(hash_file_path,"w") as wf:
		for k,v in file_hash_list.items():
			wf.write(f"{k}:{v}\n")



def _file_not_changed(changed_files,deps_file_path):
	if (not os.path.exists(deps_file_path) or not os.path.exists(deps_file_path[:-5])):
		return False
	with open(deps_file_path,"r") as rf:
		files=rf.read().replace("\\\n","").split(" ")[1:]
	for file in files:
		file=file.strip()
		if (not file or not file.startswith("src/")):
			continue
		if (file in changed_files or not os.path.exists(file)):
			return False
	return True



def _get_files(directories,suffixes=[".asm",".c"]):
	for directory in directories:
		for root,_,files in os.walk(directory):
			for file_name in files:
				if (suffixes is not None and ("." not in file_name or file_name[file_name.rindex("."):] not in suffixes)):
					continue
				yield os.path.join(root,file_name)



def _generate_header_files(directory):
	for root,_,files in os.walk(f"{directory}/rsrc"):
		for file_name in files:
			if (not os.path.exists(f"{directory}/_generated")):
				os.mkdir(f"{directory}/_generated")
				os.mkdir(f"{directory}/_generated/include")
			name=os.path.join(root,file_name).replace("/","_").replace(".","_")
			with open(os.path.join(root,file_name),"rb") as rf,open(f"{directory}/_generated/include/{name}.h","w") as wf:
				wf.write(f"#include <sys/types.h>\n\n\n\nstatic const u8 {name}[]={{")
				size=0
				while (True):
					line=rf.read(16)
					if (not line):
						break
					size+=len(line)
					wf.write("\n\t"+"".join([f"0x{e:02x}," for e in line]))
				wf.write(f"\n\t0x00,\n}};\n\n\n\nstatic const u32 {name}_length={size};\n")



def _get_kernel_build_name():
	root=config.parse("src/config/version.config")
	return "x86_64."+MODE_NAME+f"/{next(root.find('major')).data}.{next(root.find('minor')).data}.{next(root.find('patch')).data}-"+os.environ.get("GITHUB_SHA","local")[:7]



def _compile_stage(config_prefix,pool,changed_files,default_dependency_directory,name,dependencies,dependencies_are_libraries):
	if (dependencies is None):
		dependencies=option(config_prefix+".dependencies")
	included_directories=[f"-Isrc/{tag.name.replace('$NAME',name)}/include" for tag in option(config_prefix+".includes").iter()]+[f"-Isrc/{(tag.data if not dependencies_are_libraries and tag.data else default_dependency_directory)}/{tag.name}/include" for tag in dependencies.iter()]
	object_files=[]
	has_updates=False
	for file in _get_files([option(config_prefix+".src_file_directory").replace("$NAME",name)]+[f"src/common/{tag.name}" for tag in dependencies.iter() if (tag.data if tag.data else default_dependency_directory)=="common"]):
		object_file=option(config_prefix+".object_file_directory")+file.replace("/","#")+".o"
		object_files.append(object_file)
		if (_file_not_changed(changed_files,object_file+".deps")):
			pool.dispatch(object_file)
			continue
		pool.add([],object_file,"C "+file,shlex.split(option(config_prefix+".command.compile."+file.split(".")[-1]))+included_directories+["-D__UNIQUE_FILE_NAME__="+file.replace("/","_").split(".")[0],"-MD","-MT",object_file,"-MF",object_file+".deps","-o",object_file,file])
		has_updates=True
	dependency_object_files=[]
	dependency_link_requirements=[]
	if (dependencies_are_libraries):
		for tag in dependencies.iter():
			dependency_object_files.append((f"build/lib/lib{tag.name}.a" if tag.data=="static" else f"-l{tag.name}"))
			dependency_link_requirements.append(f"build/lib/lib{tag.name}.{('a' if tag.data=='static' else 'so')}")
	if (option(config_prefix+".command.link")):
		output_file_path=option(config_prefix+".output_file_path").replace("$NAME",name)
		if (has_updates or not os.path.exists(output_file_path)):
			pool.add(object_files+dependency_link_requirements,output_file_path,"L "+output_file_path,shlex.split(option(config_prefix+".command.link"))+["-o",output_file_path]+object_files+dependency_object_files)
			if (option(config_prefix+".command.patch")):
				pool.add([output_file_path],output_file_path,"P "+output_file_path,shlex.split(option(config_prefix+".command.patch").replace("$NAME",name)))
			has_updates=True
		else:
			pool.dispatch(output_file_path)
	if (option(config_prefix+".command.archive")):
		output_file_path=option(config_prefix+".archive_output_file_path").replace("$NAME",name)
		if (has_updates or not os.path.exists(output_file_path)):
			pool.add(object_files,output_file_path,"A "+output_file_path,shlex.split(option(config_prefix+".command.archive"))+[output_file_path]+object_files)
			has_updates=True
		else:
			pool.dispatch(output_file_path)
	return has_updates



def _compile():
	changed_files,file_hash_list=_load_changed_files(option("hash_file_path."+MODE_NAME),"src")
	pool=process_pool.ProcessPool(file_hash_list)
	out={"efi":False,"data":False}
	out["efi"]|=_compile_stage("uefi",pool,changed_files,"common","uefi",None,False)
	out["data"]|=_compile_stage("kernel_"+MODE_NAME,pool,changed_files,"common","kernel",None,False)
	for tag in config.parse("src/module/dependencies.config").iter():
		if (mode!=MODE_COVERAGE and tag.name.startswith("test")):
			continue
		out["data"]|=_compile_stage("module_"+MODE_NAME,pool,changed_files,"module",tag.name,tag,False)
	for tag in config.parse("src/lib/dependencies.config").iter():
		if (mode!=MODE_COVERAGE and tag.name.startswith("test")):
			continue
		_generate_header_files(f"src/lib/{tag.name}")
		out["data"]|=_compile_stage("lib_"+MODE_NAME,pool,changed_files,"lib",tag.name,tag,True)
	for tag in config.parse("src/user/dependencies.config").iter():
		if (mode!=MODE_COVERAGE and tag.name.startswith("test")):
			continue
		tag.data.append(config.ConfigTag(tag,b"runtime",config.CONFIG_TAG_TYPE_STRING,"static"))
		out["data"]|=_compile_stage("user_"+MODE_NAME,pool,changed_files,"lib",tag.name,tag,True)
	for tag in config.parse("src/tool/dependencies.config").iter():
		out["data"]|=_compile_stage("tool_"+MODE_NAME,pool,changed_files,"common",tag.name,tag,False)
	error=pool.wait()
	_save_file_hash_list(file_hash_list,option("hash_file_path."+MODE_NAME))
	if (error):
		sys.exit(1)
	return out



def _generate_shared_directory():
	for library in os.listdir("build/lib"):
		if (not library.endswith(".so")):
			continue
		with open(f"build/lib/{library}","rb") as rf,open(f"build/share/lib/{library}","wb") as wf:
			wf.write(rf.read())
		os.chmod(f"build/share/lib/{library}",0o775)
	if (os.path.islink("build/share/lib/ld.so")):
		os.unlink("build/share/lib/ld.so")
	os.symlink("/lib/liblinker.so","build/share/lib/ld.so")
	for program in os.listdir("build/user"):
		with open(f"build/user/{program}","rb") as rf,open(f"build/share/bin/{program}","wb") as wf:
			wf.write(rf.read())
		os.chmod(f"build/share/bin/{program}",0o775)



def _get_early_modules():
	return ["module_loader"]+[tag.name for tag in config.parse("src/config/module_order.config").iter() if tag.data=="early"]



def _execute_compressor_command(file_path):
	if (subprocess.run(["build/tool/compressor",file_path,option("compression_level"),file_path+".compressed"]).returncode!=0):
		sys.exit(1)



def _execute_kfs2_command(command):
	if (subprocess.run(["build/tool/kfs2","build/install_disk.img",f"{option('install_disk.block_size')}:93720:{option('install_disk.size')-34}"]+command).returncode!=0):
		sys.exit(1)



def _generate_install_disk(rebuild_uefi_partition,rebuild_data_partition):
	if (not os.path.exists("build/install_disk.img")):
		rebuild_uefi_partition=True
		rebuild_data_partition=True
		subprocess.run(["dd","if=/dev/zero","of=build/install_disk.img",f"bs={option('install_disk.block_size')}",f"count={option('install_disk.size')}"])
		subprocess.run(["parted","build/install_disk.img","-s","-a","minimal","mklabel","gpt"])
		subprocess.run(["parted","build/install_disk.img","-s","-a","minimal","mkpart","EFI","FAT16","34s","93719s"])
		subprocess.run(["parted","build/install_disk.img","-s","-a","minimal","mkpart","DATA","93720s",f"{option('install_disk.size')-34}s"])
		subprocess.run(["parted","build/install_disk.img","-s","-a","minimal","toggle","1","boot"])
		_execute_kfs2_command(["format"])
	if (not os.path.exists("build/partitions/efi.img")):
		rebuild_uefi_partition=True
		subprocess.run(["dd","if=/dev/zero","of=build/partitions/efi.img",f"bs={option('install_disk.block_size')}","count=93686"])
		subprocess.run(["mformat","-i","build/partitions/efi.img","-h","32","-t","32","-n","64","-c","1","-l","LABEL"])
		subprocess.run(["mmd","-i","build/partitions/efi.img","::/EFI","::/EFI/BOOT"])
	if (rebuild_uefi_partition):
		subprocess.run(["mcopy","-i","build/partitions/efi.img","-D","o","build/uefi/loader.efi","::/EFI/BOOT/BOOTX64.EFI"])
		subprocess.run(["dd","if=build/partitions/efi.img","of=build/install_disk.img",f"bs={option('install_disk.block_size')}","count=93686","seek=34","conv=notrunc"])
	if (rebuild_data_partition):
		files={}
		for module in _get_early_modules():
			files[f"/boot/module/{module}.mod"]=f"build/module/{module}.mod"
		files["/boot/module/module_order.config"]="src/config/module_order.config"
		files["/etc/fs_list.config"]="src/config/fs_list.config"
		initramfs.create("build/partitions/initramfs.img",files)
		_execute_compressor_command("build/kernel/kernel.bin")
		_execute_compressor_command("build/partitions/initramfs.img")
		_execute_kfs2_command(["mkdir","/bin","0755"])
		_execute_kfs2_command(["mkdir","/boot","0500"])
		_execute_kfs2_command(["mkdir","/boot/module","0700"])
		_execute_kfs2_command(["mkdir","/etc","0755"])
		_execute_kfs2_command(["mkdir","/lib","0755"])
		_execute_kfs2_command(["copy","/boot/kernel","0400","build/kernel/kernel.bin.compressed"])
		_execute_kfs2_command(["copy","/boot/initramfs","0400","build/partitions/initramfs.img.compressed"])
		_execute_kfs2_command(["copy","/boot/module/module_order.config","0600","src/config/module_order.config"])
		_execute_kfs2_command(["copy","/etc/fs_list.config","0644","src/config/fs_list.config"])
		_execute_kfs2_command(["link","/lib/ld.so","0755","/lib/liblinker.so"])
		for module in os.listdir("build/module"):
			_execute_kfs2_command(["copy",f"/boot/module/{module}","0400",f"build/module/{module}"])
		for library in os.listdir("build/lib"):
			if (not library.endswith(".so")):
				continue
			_execute_kfs2_command(["copy",f"/lib/{library}","0755",f"build/lib/{library}"])
		for program in os.listdir("build/user"):
			_execute_kfs2_command(["copy",f"/bin/{program}","0755",f"build/user/{program}"])



def _kvm_flags():
	if (not os.path.exists("/dev/kvm")):
		return []
	with open("/proc/cpuinfo","r") as rf:
		if (" vmx" not in rf.read()):
			return []
	with open("/sys/devices/system/clocksource/clocksource0/current_clocksource","r") as rf:
		if ("tsc" in rf.read() and not option("vm.bypass_kvm_lock")):
			print("\x1b[1m\x1b[38;2;231;72;86mKVM support disabled due to kernel TSC clock source\x1b[0m")
			return []
	return ["-accel","kvm"]



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



def _generate_coverage_report(vm_output_file_path,output_file_path):
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
					gcno_rf.seek(16)
					gcno_rf.read(struct.unpack("I",gcno_rf.read(4))[0]+4)
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
			invalid_chain=[]
			valid_chain=[]
			for block in function.blocks:
				# prev_dst=0
				# is_out_of_order=False
				for arc in block.next:
					if (not (arc.flags&ARC_FLAG_ON_TREE)):
						arc.count=function.counters[counter_index]
						counter_index+=1
						block.next_count-=1
						function.blocks[arc.dst].prev_count-=1
				# 	if (prev_dst>arc.dst):
				# 		is_out_of_order=True
				# 	prev_dst=arc.dst
				# if (is_out_of_order):
				# 	block.next=sorted(block.next,key=lambda arc:arc.dst)
				invalid_chain.append(block)
			while (invalid_chain or valid_chain):
				if (invalid_chain):
					block=invalid_chain.pop(0)
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
					valid_chain.append(block)
				if (valid_chain):
					block=valid_chain.pop(0)
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
						if (dst_block.count!=-1):
							if (dst_block.prev_count==1 and dst_block not in valid_chain):
								valid_chain.append(dst_block)
						else:
							if (not dst_block.prev_count and dst_block not in invalid_chain):
								invalid_chain.append(dst_block)
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
						if (src_block.count!=-1):
							if (src_block.next_count==1 and src_block not in valid_chain):
								valid_chain.append(src_block)
						else:
							if (not src_block.next_count and src_block not in invalid_chain):
								invalid_chain.append(src_block)
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



def _execute_vm():
	if (not os.path.exists("/tmp/tpm")):
		os.mkdir("/tmp/tpm")
	subprocess.Popen(["swtpm","socket","--tpmstate","dir=/tmp/tpm/","--ctrl","type=unixio,path=/tmp/swtpm.sock","--tpm2","--log","level=0"])
	while (not os.path.exists("/tmp/swtpm.sock")):
		time.sleep(0.01)
	if (option("vm.file_server")):
		subprocess.Popen((["/usr/libexec/virtiofsd",f"--socket-group={os.getlogin()}"] if not os.getenv("GITHUB_ACTIONS","") else ["sudo","build/external/virtiofsd"])+["--socket-path=build/vm/virtiofsd.sock","--shared-dir","build/share","--inode-file-handles=mandatory"])
	if (not os.path.exists("build/vm/hdd.qcow2")):
		if (subprocess.run(["qemu-img","create","-q","-f","qcow2","build/vm/hdd.qcow2","16G"]).returncode!=0):
			sys.exit(1)
	if (not os.path.exists("build/vm/ssd.qcow2")):
		if (subprocess.run(["qemu-img","create","-q","-f","qcow2","build/vm/ssd.qcow2","8G"]).returncode!=0):
			sys.exit(1)
	if (not os.path.exists("build/vm/OVMF_CODE.fd")):
		if (os.path.exists("/usr/share/OVMF/OVMF_CODE.fd")):
			if (subprocess.run(["cp","/usr/share/OVMF/OVMF_CODE.fd","build/vm/OVMF_CODE.fd"]).returncode!=0):
				sys.exit(1)
		else:
			if (subprocess.run(["cp","/usr/share/OVMF/OVMF_CODE_4M.fd","build/vm/OVMF_CODE.fd"]).returncode!=0):
				sys.exit(1)
	if (not os.path.exists("build/vm/OVMF_VARS.fd")):
		if (os.path.exists("/usr/share/OVMF/OVMF_VARS.fd")):
			if (subprocess.run(["cp","/usr/share/OVMF/OVMF_VARS.fd","build/vm/OVMF_VARS.fd"]).returncode!=0):
				sys.exit(1)
		else:
			if (subprocess.run(["cp","/usr/share/OVMF/OVMF_VARS_4M.fd","build/vm/OVMF_VARS.fd"]).returncode!=0):
				sys.exit(1)
	subprocess.run(([] if not os.getenv("GITHUB_ACTIONS","") else ["sudo"])+[
		"qemu-system-x86_64",
		# "-d","trace:virtio*,trace:virtio_blk*",
		# "-d","trace:virtio*,trace:virtio_gpu*",
		# "-d","trace:virtio*,trace:vhost*,trace:virtqueue*",
		# "-d","trace:usb*",
		# "-d","trace:nvme*,trace:pci_nvme*",
		# "-d","trace:tpm*",
		# "--no-reboot",
		# "-d","guest_errors",
		# "-d","int,cpu_reset",
		# Bios
		"-drive","if=pflash,format=raw,unit=0,file=build/vm/OVMF_CODE.fd,readonly=on",
		"-drive","if=pflash,format=raw,unit=1,file=build/vm/OVMF_VARS.fd",
		# Drive files
		"-drive","file=build/vm/hdd.qcow2,if=none,id=hdd",
		"-drive","file=build/vm/ssd.qcow2,if=none,id=ssd",
		"-drive","file=build/install_disk.img,if=none,id=bootusb,format=raw",
		# Drives
		"-device","ahci,id=ahci",
		"-device","ide-hd,drive=hdd,bus=ahci.0",
		"-device","nvme,serial=00112233,drive=ssd",
		# USB
		"-device","nec-usb-xhci,id=xhci",
		"-device","usb-storage,bus=xhci.0,drive=bootusb,bootindex=0",
		# Network
		"-netdev","user,hostfwd=tcp::10023-:22,id=network",
		"-device","e1000,netdev=network",
		"-object","filter-dump,id=network-filter,netdev=network,file=build/vm/network.dat",
		# Memory
		"-m","2G,slots=2,maxmem=4G",
		"-object","memory-backend-memfd,size=1G,id=mem0,share=on", # share=on is required by virtiofsd
		"-object","memory-backend-memfd,size=1G,id=mem1,share=on",
		# CPU
		"-cpu","Skylake-Client-v4,tsc,invtsc,avx,avx2,bmi1,bmi2,pdpe1gb",
		"-smp","4,sockets=2,cores=1,threads=2,maxcpus=4",
		"-device","intel-iommu",
		# NUMA
		"-numa","node,nodeid=0,memdev=mem0",
		"-numa","node,nodeid=1,memdev=mem1",
		"-numa","cpu,node-id=0,socket-id=0",
		"-numa","cpu,node-id=1,socket-id=1",
		"-numa","hmat-lb,initiator=0,target=0,hierarchy=memory,data-type=access-latency,latency=5",
		"-numa","hmat-lb,initiator=0,target=0,hierarchy=memory,data-type=access-bandwidth,bandwidth=128M",
		"-numa","hmat-lb,initiator=0,target=1,hierarchy=memory,data-type=access-latency,latency=10",
		"-numa","hmat-lb,initiator=0,target=1,hierarchy=memory,data-type=access-bandwidth,bandwidth=64M",
		"-numa","hmat-lb,initiator=1,target=1,hierarchy=memory,data-type=access-latency,latency=5",
		"-numa","hmat-lb,initiator=1,target=1,hierarchy=memory,data-type=access-bandwidth,bandwidth=128M",
		"-numa","hmat-lb,initiator=1,target=0,hierarchy=memory,data-type=access-latency,latency=10",
		"-numa","hmat-lb,initiator=1,target=0,hierarchy=memory,data-type=access-bandwidth,bandwidth=64M",
		"-numa","hmat-cache,node-id=0,size=10K,level=1,associativity=direct,policy=write-back,line=8",
		"-numa","hmat-cache,node-id=1,size=10K,level=1,associativity=direct,policy=write-back,line=8",
		"-numa","dist,src=0,dst=1,val=20",
		# Graphics
		*(["-device","virtio-vga-gl,xres=1280,yres=960","-display","sdl,gl=on"] if option("vm.display") else ["-display","none"]),
		# Shared filesystem
		*(["-chardev","socket,id=virtio-fs-sock,path=build/vm/virtiofsd.sock","-device","vhost-user-fs-pci,queue-size=1024,chardev=virtio-fs-sock,tag=build-fs"] if option("vm.file_server") else []),
		# Serial
		"-serial","mon:stdio",
		"-serial",("file:build/raw_coverage" if mode==MODE_COVERAGE else "null"),
		# Config
		"-machine","q35,hmat=on",
		"-uuid","00112233-4455-6677-8899-aabbccddeeff",
		"-smbios","type=2,serial=SERIAL_NUMBER",
		# TPM
		"-chardev","socket,id=tpm,path=/tmp/swtpm.sock",
		"-tpmdev","emulator,id=tpm0,chardev=tpm",
		"-device","tpm-tis,tpmdev=tpm0",
		# Debugging
		*([] if mode!=MODE_DEBUG else ["-gdb","tcp::9000"]),
	]+_kvm_flags())
	if (os.path.exists("build/vm/virtiofsd.sock")):
		os.remove("build/vm/virtiofsd.sock")
	if (os.path.exists("build/vm/virtiofsd.sock.pid")):
		os.remove("build/vm/virtiofsd.sock.pid")
	if (os.path.exists("/tmp/tpm/swtpm.sock")):
		os.remove("/tmp/tpm/swtpm.sock")
	if (mode==MODE_COVERAGE):
		_generate_coverage_report("build/raw_coverage","build/coverage.lcov")
		os.remove("build/raw_coverage")



empty_directories=option("build_directories.empty").data[:]
if (os.path.exists("build/last_mode")):
	with open("build/last_mode","r") as rf:
		if (int(rf.read())!=mode):
			empty_directories.extend(option("build_directories.empty_after_mode_change").data)
for dir in option("build_directories.required").iter():
	if (not os.path.exists(dir.data)):
		os.mkdir(dir.data)
for file in _get_files(map(lambda e:e.data,empty_directories),suffixes=None):
	os.remove(file)
for tag in option("build_directories.delete_obsolete").iter():
	_clear_if_obsolete(next(tag.find("path")).data,next(tag.find("src_path")).data,next(tag.find("prefix")).data)
for tag in option("keys").iter():
	signature.load_key(tag.name,tag.data)
if (len(sys.argv)>1 and sys.argv[1]=="__patch_kernel"):
	linker.patch_kernel(sys.argv[3],sys.argv[4],time.time_ns(),_get_kernel_build_name())
	sys.exit(0)
if (len(sys.argv)>1 and sys.argv[1]=="__patch_module_or_library"):
	linker.patch_module_or_library(sys.argv[3],sys.argv[4])
	sys.exit(0)
with open("build/last_mode","w") as wf:
	wf.write(f"{mode}\n")
if (mode==MODE_COVERAGE):
	test.generate_test_resource_files()
install_disk_rebuild_parts=_compile()
_generate_shared_directory()
if ("--share" in sys.argv):
	sys.exit(0)
_generate_install_disk(install_disk_rebuild_parts["efi"],install_disk_rebuild_parts["data"])
if ("--run" not in sys.argv):
	sys.exit(0)
_execute_vm()
for root,_,files in os.walk("build/share",followlinks=False):
	for file in files:
		if (os.path.islink(os.path.join(root,file))):
			continue
		os.chmod(os.path.join(root,file),0o664)
