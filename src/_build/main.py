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



def _compile_stage(config_prefix,pool,changed_files,default_dependency_directory="common",patch_command=None,name="<null>",dependencies=None,dependencies_are_libraries=False,dependencies_are_link_requirements=False):
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
	extra_link_requirements=[]
	if (dependencies_are_libraries):
		for tag in dependencies.iter():
			dependency_object_files.append((f"build/lib/lib{tag.name}.a" if tag.data=="static" else f"-l{tag.name}"))
			extra_link_requirements.append(f"build/lib/lib{tag.name}.{("a" if tag.data=="static" else "so")}")
		if (not dependencies_are_link_requirements):
			extra_link_requirements.clear()
	if (option(config_prefix+".command.link")):
		output_file_path=option(config_prefix+".output_file_path").replace("$NAME",name)
		if (has_updates or not os.path.exists(output_file_path)):
			pool.add(object_files+extra_link_requirements,output_file_path,"L "+output_file_path,shlex.split(option(config_prefix+".command.link"))+["-o",output_file_path]+object_files+dependency_object_files)
			pool.add([output_file_path],output_file_path,"P "+output_file_path,([patch_command,output_file_path] if patch_command is not None else shlex.split(option(config_prefix+".command.patch"))))
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



def _compile_uefi():
	changed_files,file_hash_list=_load_changed_files(option("uefi.hash_file_path"),"src/uefi","src/common")
	pool=process_pool.ProcessPool(file_hash_list)
	if (not _compile_stage("uefi",pool,changed_files)):
		return False
	error=pool.wait()
	_save_file_hash_list(file_hash_list,option("uefi.hash_file_path"))
	if (error):
		sys.exit(1)
	return True



def _compile_kernel():
	config_prefix="kernel_"+MODE_NAME
	changed_files,file_hash_list=_load_changed_files(option(config_prefix+".hash_file_path"),"src/kernel","src/common")
	pool=process_pool.ProcessPool(file_hash_list)
	if (not _compile_stage(config_prefix,pool,changed_files,patch_command=lambda output_file_path:linker.link_kernel(output_file_path,"build/kernel/kernel.bin",time.time_ns(),_get_kernel_build_name()))):
		return False
	error=pool.wait()
	_save_file_hash_list(file_hash_list,option(config_prefix+".hash_file_path"))
	if (error):
		sys.exit(1)
	return True



def _compile_modules():
	config_prefix="module_"+MODE_NAME
	changed_files,file_hash_list=_load_changed_files(option(config_prefix+".hash_file_path"),"src/module","src/kernel/include","src/common")
	pool=process_pool.ProcessPool(file_hash_list)
	out=False
	for tag in config.parse("src/module/dependencies.config").iter():
		if (mode!=MODE_COVERAGE and tag.name.startswith("test")):
			continue
		out|=_compile_stage(config_prefix,pool,changed_files,default_dependency_directory="module",patch_command=lambda output_file_path:linker.link_module_or_library(output_file_path,"module"),name=tag.name,dependencies=tag)
	error=pool.wait()
	_save_file_hash_list(file_hash_list,option(config_prefix+".hash_file_path"))
	if (error):
		sys.exit(1)
	return out



def _compile_libraries():
	config_prefix="lib_"+MODE_NAME
	changed_files,file_hash_list=_load_changed_files(option(config_prefix+".hash_file_path"),"src/lib")
	pool=process_pool.ProcessPool(file_hash_list)
	out=False
	for tag in config.parse("src/lib/dependencies.config").iter():
		if (mode!=MODE_COVERAGE and tag.name.startswith("test")):
			continue
		_generate_header_files(f"src/lib/{tag.name}")
		out|=_compile_stage(config_prefix,pool,changed_files,default_dependency_directory="lib",patch_command=lambda output_file_path:linker.link_module_or_library(output_file_path,"user"),name=tag.name,dependencies=tag,dependencies_are_libraries=True,dependencies_are_link_requirements=True)
	error=pool.wait()
	_save_file_hash_list(file_hash_list,option(config_prefix+".hash_file_path"))
	if (error):
		sys.exit(1)
	return out



def _compile_user_programs():
	config_prefix="user_"+MODE_NAME
	changed_files,file_hash_list=_load_changed_files(option(config_prefix+".hash_file_path"),"src/user","src/lib")
	pool=process_pool.ProcessPool(file_hash_list)
	out=False
	for tag in config.parse("src/user/dependencies.config").iter():
		if (mode!=MODE_COVERAGE and tag.name.startswith("test")):
			continue
		tag.data.append(config.ConfigTag(tag,b"runtime",config.CONFIG_TAG_TYPE_STRING,"static"))
		out|=_compile_stage(config_prefix,pool,changed_files,default_dependency_directory="lib",patch_command=lambda output_file_path:linker.link_module_or_library(output_file_path,"user"),name=tag.name,dependencies=tag,dependencies_are_libraries=True)
	error=pool.wait()
	_save_file_hash_list(file_hash_list,option(config_prefix+".hash_file_path"))
	if (error):
		sys.exit(1)
	return out



def _compile_all_tools():
	config_prefix="tool_"+MODE_NAME
	changed_files,file_hash_list=_load_changed_files(option(config_prefix+".hash_file_path"),"src/tool","src/common")
	pool=process_pool.ProcessPool(file_hash_list)
	for tag in config.parse("src/tool/dependencies.config").iter():
		_compile_stage(config_prefix,pool,changed_files,patch_command=lambda output_file_path:None,name=tag.name,dependencies=tag)
	error=pool.wait()
	_save_file_hash_list(file_hash_list,option(config_prefix+".hash_file_path"))
	if (error):
		sys.exit(1)



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



def _generate_install_disk(rebuild_uefi,rebuild_kernel,rebuild_modules,rebuild_libraries,rebuild_user_programs):
	rebuild_uefi_partition=rebuild_uefi
	rebuild_data_partition=rebuild_kernel|rebuild_modules|rebuild_libraries|rebuild_user_programs
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



def _generate_coverage_report(vm_output_file_path,output_file_path):
	for file in _get_files(["build/objects"],suffixes=[".gcda"]):
		os.remove(file)
	file_list=set()
	success=False
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
			file_list.add(file_name)
			gcno_file_name=file_name[:-5]+".gcno"
			stat=os.stat(gcno_file_name)
			os.utime(gcno_file_name,times=(stat.st_atime,time.time()))
			with open(gcno_file_name,"rb") as gcno_rf:
				stamp=struct.unpack("III",gcno_rf.read(12))[2]
			present_data={}
			if (os.path.exists(file_name)):
				with open(file_name,"rb") as out_rf:
					out_rf.read(16)
					while (True):
						buffer=out_rf.read(28)
						if (not buffer):
							break
						_,_,id_,lineno_checksum,cfg_checksum,_,counter_byte_count=struct.unpack("IIIIIII",buffer)
						present_data[(id_,lineno_checksum,cfg_checksum)]=out_rf.read(counter_byte_count)
			with open(file_name,"wb") as wf:
				wf.write(b"adcg")
				wf.write(struct.pack("III",version,stamp,checksum))
				for i in range(0,struct.unpack("I",rf.read(4))[0]):
					id_,lineno_checksum,cfg_checksum,counter_count=struct.unpack("IIII",rf.read(16))
					wf.write(struct.pack("IIIIIII",0x01000000,12,id_,lineno_checksum,cfg_checksum,0x01a10000,counter_count<<3))
					counters=rf.read(counter_count<<3)
					present_counters=present_data.get((id_,lineno_checksum,cfg_checksum),None)
					if (present_counters is not None):
						dst=array.array("Q")
						dst.frombytes(counters)
						src=array.array("Q")
						src.frombytes(present_counters)
						for i in range(0,counter_count):
							dst[i]+=src[i]
						counters=dst.tobytes()
					wf.write(counters)
	if (not success):
		sys.exit(1)
	with open(output_file_path,"w") as wf:
		wf.write("TN:\n")
		function_stats=None
		for line in subprocess.run(["gcov-12","-b","-t"]+list(file_list),stdout=subprocess.PIPE).stdout.decode("utf-8").split("\n"):
			line=line.strip().split(":")
			if (len(line)<2):
				if (line[0].startswith("function ")):
					name,count=line[0][9:].split(" called ")
					function_stats=(name.strip(),int(count.split(" ")[0].strip()))
				continue
			code_line=line[1].strip()
			if ("%" in code_line):
				continue
			if (not code_line or code_line=="0"):
				if (len(line)<4):
					continue
				if (line[2]=="Source"):
					wf.write(f"SF:{line[3]}\n")
				continue
			code_line=int(code_line)
			type_=line[0].strip()
			if (function_stats):
				name,_=function_stats
				function_stats=None
				wf.write(f"FN:{code_line},{name}\n")
			if (line[0].isdigit()):
				wf.write(f"DA:{code_line},{line[0]}\n")
			elif (line[0]=="#####"):
				wf.write(f"DA:{code_line},0\n")



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
with open("build/last_mode","w") as wf:
	wf.write(f"{mode}\n")
for tag in option("keys").iter():
	signature.load_key(tag.name,tag.data)
if (mode==MODE_COVERAGE):
	test.generate_test_resource_files()
rebuild_uefi=_compile_uefi()
rebuild_kernel=_compile_kernel()
rebuild_modules=_compile_modules()
rebuild_libraries=_compile_libraries()
rebuild_user_programs=_compile_user_programs()
_compile_all_tools()
_generate_shared_directory()
if ("--share" in sys.argv):
	sys.exit(0)
_generate_install_disk(rebuild_uefi,rebuild_kernel,rebuild_modules,rebuild_libraries,rebuild_user_programs)
if ("--run" not in sys.argv):
	sys.exit(0)
_execute_vm()
for root,_,files in os.walk("build/share",followlinks=False):
	for file in files:
		if (os.path.islink(os.path.join(root,file))):
			continue
		os.chmod(os.path.join(root,file),0o664)
