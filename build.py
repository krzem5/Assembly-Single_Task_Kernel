import array
import hashlib
import os
import socket
import struct
import subprocess
import sys
import threading
import time



MODE_NORMAL=0
MODE_COVERAGE=1
MODE_RELEASE=2



mode=MODE_NORMAL
if ("--release" in sys.argv):
	mode=MODE_RELEASE
if ("--coverage" in sys.argv):
	mode=MODE_COVERAGE



KERNEL_OBJECT_FILE_SUFFIX={
	MODE_NORMAL: ".kernel.o",
	MODE_COVERAGE: ".kernel.coverage.o",
	MODE_RELEASE: ".kernel.o"
}[mode]
KERNEL_EXTRA_COMPILER_OPTIONS={
	MODE_NORMAL: [],
	MODE_COVERAGE: ["--coverage","-fprofile-arcs","-ftest-coverage","-fprofile-info-section","-fprofile-update=atomic","-DKERNEL_COVERAGE_ENABLED=1","-O1"],
	MODE_RELEASE: []
}[mode]
KERNEL_EXTRA_LINKER_OPTIONS={
	MODE_NORMAL: ["-T","src/kernel/linker.ld"],
	MODE_COVERAGE: ["-T","src/kernel/linker_coverage.ld","-g"],
	MODE_RELEASE: ["-T","src/kernel/linker.ld"]
}[mode]
USER_OBJECT_FILE_SUFFIX={
	MODE_NORMAL: ".user.o",
	MODE_COVERAGE: ".user.o",
	MODE_RELEASE: ".user.release.o"
}[mode]
USER_EXTRA_COMPILER_OPTIONS={
	MODE_NORMAL: ["-O0","-g","-fno-omit-frame-pointer"],
	MODE_COVERAGE: ["-O0","-g","-fno-omit-frame-pointer","-DKERNEL_COVERAGE_ENABLED=1"],
	MODE_RELEASE: ["-O3","-g0","-fdata-sections","-ffunction-sections","-fomit-frame-pointer"]
}[mode]
USER_EXTRA_ASSEMBLY_COMPILER_OPTIONS={
	MODE_NORMAL: ["-O0","-g"],
	MODE_COVERAGE: ["-O0","-g"],
	MODE_RELEASE: ["-O3"]
}[mode]
USER_EXTRA_LINKER_OPTIONS={
	MODE_NORMAL: ["-O0","-g"],
	MODE_COVERAGE: ["-O0","-g"],
	MODE_RELEASE: ["-O3","--gc-sections"]
}[mode]
KERNEL_HASH_FILE_PATH={
	MODE_NORMAL: "build/hashes.txt",
	MODE_COVERAGE: "build/hashes.coverage.txt",
	MODE_RELEASE: "build/hashes.txt"
}[mode]
USER_HASH_FILE_SUFFIX={
	MODE_NORMAL: ".user.txt",
	MODE_COVERAGE: ".user.txt",
	MODE_RELEASE: ".user.release.txt"
}[mode]
SOURCE_FILE_SUFFIXES=[".asm",".c"]
KERNEL_FILE_DIRECTORY="src/kernel"
KERNEL_VERSION_FILE_PATH="src/kernel/include/kernel/_version.h"
USER_FILE_DIRECTORY="src/user"
OS_IMAGE_SIZE=1440*1024



def _generate_kernel_version(version_file_path):
	version=time.time_ns()
	with open(version_file_path,"w") as wf:
		wf.write(f"#ifndef _KERNEL__VERSION_H_\n#define _KERNEL__VERSION_H_ 1\n\n\n\n#define KERNEL_VERSION 0x{version:16x}ull\n\n\n\n#endif\n")
	return version



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
	if (not os.path.exists(deps_file_path)):
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



def _read_kernel_symbols(file_path):
	out={}
	for line in subprocess.run(["nm","-f","bsd",file_path],stdout=subprocess.PIPE).stdout.decode("utf-8").split("\n"):
		line=line.strip().split(" ")
		if (len(line)<3 or line[2].startswith("__func__")):
			continue
		out[line[2]]=int(line[0],16)
	return out



def _split_kernel_file(src_file_path,core_file_path,kernel_file_path,core_end,end):
	if ((core_end|end)&4095):
		raise RuntimeError("'core_end' and 'end' must be page-aligned")
	end-=core_end
	with open(src_file_path,"rb") as rf,open(core_file_path,"wb") as wf_core,open(kernel_file_path,"wb") as wf:
		while (core_end):
			wf_core.write(rf.read(4096))
			core_end-=4096
		while (end):
			wf.write(rf.read(4096))
			end-=4096



def _build_static_idt(file_path,kernel_symbols):
	with open(file_path,"r+b") as wf:
		wf.seek(kernel_symbols["_idt_data"]-kernel_symbols["__KERNEL_CORE_END__"]-kernel_symbols["__KERNEL_OFFSET__"])
		for i in range(0,256):
			address=kernel_symbols[f"_isr_entry_{i}"]
			wf.write(struct.pack("<HIHQ",address&0xffff,0x8e000008,(address>>16)&0xffff,address>>32))



def _get_file_size(file_path):
	return os.stat(file_path).st_size



def _copy_file(file_path,wf):
	with open(file_path,"rb") as rf:
		while (True):
			chunk=rf.read(4096)
			if (not chunk):
				return
			wf.write(chunk)



def _pad_file(wf,count):
	while (count):
		length=(4096 if count>4096 else count)
		wf.write(b"\x00"*length)
		count-=length



def _compile_user_files(program):
	hash_file_path=f"build/hashes."+program+USER_HASH_FILE_SUFFIX
	changed_files,file_hash_list=_load_changed_files(hash_file_path,USER_FILE_DIRECTORY+"/"+program,USER_FILE_DIRECTORY+"/runtime")
	object_files=[]
	error=False
	for root,_,files in os.walk(USER_FILE_DIRECTORY+"/"+program):
		for file_name in files:
			suffix=file_name[file_name.rindex("."):]
			if (suffix not in SOURCE_FILE_SUFFIXES):
				continue
			file=os.path.join(root,file_name)
			object_file=f"build/objects/{file.replace('/','#')}"+USER_OBJECT_FILE_SUFFIX
			object_files.append(object_file)
			if (_file_not_changed(changed_files,object_file+".d")):
				continue
			command=None
			if (suffix==".c"):
				command=["gcc","-fno-common","-fno-builtin","-nostdlib","-ffreestanding","-fno-pie","-fno-pic","-m64","-Wall","-Werror","-c","-o",object_file,"-c",file,"-DNULL=((void*)0)",f"-I{USER_FILE_DIRECTORY}/{program}/include",f"-I{USER_FILE_DIRECTORY}/runtime/include"]+USER_EXTRA_COMPILER_OPTIONS
			else:
				command=["nasm","-f","elf64","-Wall","-Werror","-O3","-o",object_file,file]+USER_EXTRA_ASSEMBLY_COMPILER_OPTIONS
			if (subprocess.run(command+["-MD","-MT",object_file,"-MF",object_file+".d"]).returncode!=0):
				del file_hash_list[file]
				error=True
	_save_file_hash_list(file_hash_list,hash_file_path)
	if (error):
		sys.exit(1)
	return object_files



def _generate_coverage_report(vm_output_file_path,gcno_file_directory,output_file_path):
	with open(vm_output_file_path,"rb") as rf:
		while (True):
			buffer=rf.read(12)
			if (not buffer):
				break
			version,checksum,file_name_length=struct.unpack("III",buffer)
			file_name=rf.read(file_name_length).decode("utf-8")
			with open(file_name[:-5]+".gcno","rb") as gcno_rf:
				stamp=struct.unpack("III",gcno_rf.read(12))[2]
			with open(file_name,"wb") as wf:
				wf.write(b"adcg")
				wf.write(struct.pack("III",version,stamp,checksum))
				for i in range(0,struct.unpack("I",rf.read(4))[0]):
					id_,lineno_checksum,cfg_checksum,counter_count=struct.unpack("IIII",rf.read(16))
					wf.write(struct.pack("IIIIIII",0x01000000,12,id_,lineno_checksum,cfg_checksum,0x01a10000,counter_count<<3))
					wf.write(rf.read(counter_count<<3))
	subprocess.run(["lcov","-c","-d","build/objects","--gcov-tool","gcov-12","-o",output_file_path])



def _process_packet(buffer):
	if (buffer[0]&1):
		return None
	type=buffer[0]>>1
	if (type==0x00):
		return bytearray([0x01])
	if (type==0x01):
		return bytearray([0x03,0x02,0x00,0x22,0x11,0x33,0x44,0x66,0x55,0x77,0x88,0xaa,0x99,0xbb,0xcc,0xee,0xdd,0xff])+b"TEST_SERIAL_NUMBER\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
	return None



def _l2tpv3_worker():
	mac_address=b"\x5b\x9c\xc4\x41\x84\x2c"
	recv_socket=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
	send_socket=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
	recv_socket.bind(("127.0.0.1",7556))
	while (1):
		data,_=recv_socket.recvfrom(8192)
		if (len(data)<8):
			continue
		flags,version=struct.unpack("<BB",data[:2])
		if (version==3 and (flags&1)==0):
			buffer=_process_packet(bytearray(data[22:]))
			if (buffer is not None):
				data=data[:8]+data[14:20]+mac_address+data[20:22]+buffer
				send_socket.sendto(data,("127.0.0.1",7555))



def _start_l2tpv3_thread():
	thread=threading.Thread(target=_l2tpv3_worker)
	thread.daemon=True
	thread.start()



if (not os.path.exists("build")):
	os.mkdir("build")
if (not os.path.exists("build/iso")):
	os.mkdir("build/iso")
if (not os.path.exists("build/iso/kernel")):
	os.mkdir("build/iso/kernel")
if (not os.path.exists("build/objects")):
	os.mkdir("build/objects")
if (not os.path.exists("build/stages")):
	os.mkdir("build/stages")
version=_generate_kernel_version(KERNEL_VERSION_FILE_PATH)
changed_files,file_hash_list=_load_changed_files(KERNEL_HASH_FILE_PATH,KERNEL_FILE_DIRECTORY)
object_files=[]
error=False
for root,_,files in os.walk(KERNEL_FILE_DIRECTORY):
	for file_name in files:
		suffix=file_name[file_name.rindex("."):]
		if (suffix not in SOURCE_FILE_SUFFIXES):
			continue
		file=os.path.join(root,file_name)
		object_file=f"build/objects/{file.replace('/','#')}"+KERNEL_OBJECT_FILE_SUFFIX
		object_files.append(object_file)
		if (_file_not_changed(changed_files,object_file+".d")):
			continue
		command=None
		if (suffix==".c"):
			command=["gcc-12","-mcmodel=large","-mno-red-zone","-mno-mmx","-mno-sse","-mno-sse2","-fno-lto","-fno-pie","-fno-common","-fno-builtin","-fno-stack-protector","-fno-asynchronous-unwind-tables","-nostdinc","-nostdlib","-ffreestanding","-m64","-Wall","-Werror","-c","-ftree-loop-distribute-patterns","-O3","-g0","-fomit-frame-pointer","-DNULL=((void*)0)","-o",object_file,"-c",file,f"-I{KERNEL_FILE_DIRECTORY}/include"]+KERNEL_EXTRA_COMPILER_OPTIONS
		else:
			command=["nasm","-f","elf64","-O3","-Wall","-Werror","-o",object_file,file]
		if (subprocess.run(command+["-MD","-MT",object_file,"-MF",object_file+".d"]).returncode!=0):
			del file_hash_list[file]
			error=True
_save_file_hash_list(file_hash_list,KERNEL_HASH_FILE_PATH)
os.remove(KERNEL_VERSION_FILE_PATH)
if (error or subprocess.run(["ld","-z","noexecstack","-melf_x86_64","-o","build/kernel.elf","-O3"]+KERNEL_EXTRA_LINKER_OPTIONS+object_files).returncode!=0 or subprocess.run(["objcopy","-S","-O","binary","build/kernel.elf","build/kernel.bin"]).returncode!=0):
	sys.exit(1)
kernel_symbols=_read_kernel_symbols("build/kernel.elf")
_split_kernel_file("build/kernel.bin","build/stages/stage3.bin","build/iso/kernel/kernel.bin",kernel_symbols["__KERNEL_CORE_END__"]-kernel_symbols["__KERNEL_START__"],kernel_symbols["__KERNEL_END__"]-kernel_symbols["__KERNEL_START__"])
_build_static_idt("build/iso/kernel/kernel.bin",kernel_symbols)
kernel_core_size=_get_file_size("build/stages/stage3.bin")
if (subprocess.run(["nasm","src/bootloader/stage2.asm","-f","bin","-Wall","-Werror","-O3","-o","build/stages/stage2.bin",f"-D__KERNEL_CORE_SIZE__={kernel_core_size}"]).returncode!=0):
	sys.exit(1)
stage2_size=_get_file_size("build/stages/stage2.bin")
if (subprocess.run(["nasm","src/bootloader/stage1.asm","-f","bin","-Wall","-Werror","-O3","-o","build/stages/stage1.bin",f"-D__BOOTLOADER_STAGE2_SIZE__={stage2_size}",f"-D__BOOTLOADER_VERSION__={version}"]).returncode!=0):
	sys.exit(1)
stage1_size=_get_file_size("build/stages/stage1.bin")
with open("build/iso/kernel/core.bin","wb") as wf:
	_copy_file("build/stages/stage1.bin",wf)
	_copy_file("build/stages/stage2.bin",wf)
	_copy_file("build/stages/stage3.bin",wf)
with open("build/iso/os.img","wb") as wf:
	_copy_file("build/iso/kernel/core.bin",wf)
	_pad_file(wf,OS_IMAGE_SIZE-kernel_core_size-stage2_size-stage1_size)
runtime_object_files=_compile_user_files("runtime")
for program in os.listdir(USER_FILE_DIRECTORY):
	if (program=="runtime"):
		continue
	object_files=runtime_object_files+_compile_user_files(program)
	if (subprocess.run(["ld","-z","noexecstack","-melf_x86_64","-o",f"build/iso/kernel/{program}.elf"]+object_files+USER_EXTRA_LINKER_OPTIONS).returncode!=0):
		sys.exit(1)
with open("build/iso/kernel/startup.txt","w") as wf:
	wf.write(("/kernel/coverage.elf\n" if mode==MODE_COVERAGE else "/kernel/install.elf\n"))
if (subprocess.run(["genisoimage","-q","-V","INSTALL DRIVE","-input-charset","iso8859-1","-o","build/os.iso","-b","os.img","-hide","os.img","build/iso"]).returncode!=0):
	sys.exit(1)
if ("--run" in sys.argv):
	if (not os.path.exists("build/hdd.qcow2")):
		if (subprocess.run(["qemu-img","create","-q","-f","qcow2","build/hdd.qcow2","16G"]).returncode!=0):
			sys.exit(1)
	if (not os.path.exists("build/ssd.qcow2")):
		if (subprocess.run(["qemu-img","create","-q","-f","qcow2","build/ssd.qcow2","8G"]).returncode!=0):
			sys.exit(1)
	_start_l2tpv3_thread()
	subprocess.run([
		"qemu-system-x86_64",
		# Boot
		"-boot","order=dc",
		# Drive files
		"-drive","file=build/hdd.qcow2,if=none,id=hdd",
		"-drive","file=build/ssd.qcow2,if=none,id=ssd",
		"-drive","file=build/os.iso,index=0,media=cdrom,readonly=true,id=cd",
		# Drives
		"-device","ahci,id=ahci",
		"-device","ide-hd,drive=hdd,bus=ahci.0",
		"-device","nvme,serial=00112233,drive=ssd",
		# Network
		"-netdev","l2tpv3,id=network,src=127.0.0.1,dst=127.0.0.1,udp=on,srcport=7555,dstport=7556,rxsession=0xffffffff,txsession=0xffffffff,counter=off",
		"-device","e1000,netdev=network",
		# Memory
		"-m","1G,slots=2,maxmem=2G",
		"-object","memory-backend-ram,size=512M,id=mem0",
		"-object","memory-backend-ram,size=512M,id=mem1",
		# CPU
		"-cpu","Skylake-Client-v1,tsc,invtsc,avx,avx2,bmi1,bmi2,pdpe1gb",
		"-smp","4,sockets=2,maxcpus=4",
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
		"-display","none",
		# Serial
		"-serial","mon:stdio",
		"-serial",("file:build/raw_coverage" if mode==MODE_COVERAGE else "null"),
		# Config
		# "-accel","kvm",
		"-machine","hmat=on",
		"-uuid","00112233-4455-6677-8899-aabbccddeeff",
		"-smbios","type=2,serial=SERIAL_NUMBER"
	])
	# if (mode==MODE_COVERAGE):
_generate_coverage_report("build/raw_coverage","build/objects/","build/coverage.lcov")
