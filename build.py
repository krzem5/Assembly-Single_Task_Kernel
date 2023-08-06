import hashlib
import os
import socket
import struct
import subprocess
import sys
import threading
import time



OBJECT_FILE_SUFFIX=(".release.o" if "--release" in sys.argv else ".o")
EXTRA_COMPILER_OPTIONS=(["-O3","-g0","-fdata-sections","-ffunction-sections","-fomit-frame-pointer"] if "--release" in sys.argv else ["-O0","-g","-fno-omit-frame-pointer"])
EXTRA_ASSEMBLY_COMPILER_OPTIONS=(["-O3"] if "--release" in sys.argv else ["-O0","-g"])
EXTRA_LINKER_OPTIONS=(["-O3","--gc-sections"] if "--release" in sys.argv else ["-O0","-g"])
HASH_FILE_PATH="build/hashes.txt"
USER_HASH_FILE_PATH=("build/user_hashes.release.txt" if "--release" in sys.argv else "build/user_hashes.txt")
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



def _load_changed_files(hash_file_path,file_directory):
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



def _split_file(src_file_path,dst_low_file_path,dst_high_file_path,offset):
	if (offset&4095):
		raise RuntimeError("'offset' must be page-aligned")
	with open(src_file_path,"rb") as rf,open(dst_low_file_path,"wb") as wf_low,open(dst_high_file_path,"wb") as wf_high:
		while (offset):
			wf_low.write(rf.read(4096))
			offset-=4096
		while (True):
			chunk=rf.read(4096)
			if (not chunk):
				break
			wf_high.write(chunk)



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



def _compile_user_files(file_directory,changed_files,file_hash_list):
	object_files=[]
	error=False
	for root,_,files in os.walk(file_directory):
		for file_name in files:
			suffix=file_name[file_name.rindex("."):]
			if (suffix not in SOURCE_FILE_SUFFIXES):
				continue
			file=os.path.join(root,file_name)
			object_file=f"build/objects/{file.replace('/','#')}"+OBJECT_FILE_SUFFIX
			object_files.append(object_file)
			if (_file_not_changed(changed_files,object_file+".d")):
				continue
			command=None
			if (suffix==".c"):
				command=["gcc","-fno-common","-fno-builtin","-nostdlib","-ffreestanding","-fno-pie","-fno-pic","-m64","-Wall","-Werror","-c","-o",object_file,"-c",file,"-DNULL=((void*)0)",f"-I{file_directory}/include",f"-I{USER_FILE_DIRECTORY}/runtime/include"]+EXTRA_COMPILER_OPTIONS
			else:
				command=["nasm","-f","elf64","-Wall","-Werror","-O3","-o",object_file,file]
			if (subprocess.run(command+["-MD","-MT",object_file,"-MF",object_file+".d"]).returncode!=0):
				del file_hash_list[file]
				error=True
	if (error):
		_save_file_hash_list(file_hash_list,USER_HASH_FILE_PATH)
		sys.exit(1)
	return object_files



def _l2tpv3_worker():
	recv_socket=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
	send_socket=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
	recv_socket.bind(("127.0.0.1",7556))
	while (1):
		data,_=recv_socket.recvfrom(8192)
		if (len(data)<8):
			continue
		flags,version=struct.unpack("<BB",data[:2])
		if (version==3 and (flags&1)==0):
			data=data[:8]+data[14:20]+data[8:14]+data[20:]
			send_socket.sendto(data,("127.0.0.1",7555))



def _start_l2tpv3_thread():
	thread=threading.Thread(target=_l2tpv3_worker)
	thread.daemon=True
	thread.start()



if (not os.path.exists("build")):
	os.mkdir("build")
if (not os.path.exists("build/iso")):
	os.mkdir("build/iso")
if (not os.path.exists("build/objects")):
	os.mkdir("build/objects")
if (not os.path.exists("build/stages")):
	os.mkdir("build/stages")
version=_generate_kernel_version(KERNEL_VERSION_FILE_PATH)
changed_files,file_hash_list=_load_changed_files(HASH_FILE_PATH,KERNEL_FILE_DIRECTORY)
object_files=[]
error=False
for root,_,files in os.walk(KERNEL_FILE_DIRECTORY):
	for file_name in files:
		suffix=file_name[file_name.rindex("."):]
		if (suffix not in SOURCE_FILE_SUFFIXES):
			continue
		file=os.path.join(root,file_name)
		object_file=f"build/objects/{file.replace('/','#')}.o"
		object_files.append(object_file)
		if (_file_not_changed(changed_files,object_file+".d")):
			continue
		command=None
		if (suffix==".c"):
			command=["gcc","-mcmodel=large","-mno-red-zone","-mno-mmx","-mno-sse","-mno-sse2","-fno-lto","-fno-pie","-fno-common","-fno-builtin","-fno-stack-protector","-fno-asynchronous-unwind-tables","-nostdinc","-nostdlib","-ffreestanding","-m64","-Wall","-Werror","-c","-ftree-loop-distribute-patterns","-O3","-g0","-fomit-frame-pointer","-DNULL=((void*)0)","-o",object_file,"-c",file,f"-I{KERNEL_FILE_DIRECTORY}/include"]
		else:
			command=["nasm","-f","elf64","-Wall","-Werror","-o",object_file,file]+EXTRA_ASSEMBLY_COMPILER_OPTIONS
		if (subprocess.run(command+["-MD","-MT",object_file,"-MF",object_file+".d"]).returncode!=0):
			del file_hash_list[file]
			error=True
_save_file_hash_list(file_hash_list,HASH_FILE_PATH)
os.remove(KERNEL_VERSION_FILE_PATH)
if (error or subprocess.run(["ld","-melf_x86_64","-o","build/kernel.elf","-T","src/kernel/linker.ld","-O3"]+object_files).returncode!=0 or subprocess.run(["objcopy","-S","-O","binary","build/kernel.elf","build/kernel.bin"]).returncode!=0):
	sys.exit(1)
kernel_symbols=_read_kernel_symbols("build/kernel.elf")
_split_file("build/kernel.bin","build/stages/stage3.bin","build/iso/kernel.bin",kernel_symbols["__KERNEL_CORE_END__"]-kernel_symbols["__KERNEL_START__"])
kernel_core_size=_get_file_size("build/stages/stage3.bin")
if (subprocess.run(["nasm","src/bootloader/stage2.asm","-f","bin","-Wall","-Werror","-O3","-o","build/stages/stage2.bin",f"-D__KERNEL_CORE_SIZE__={kernel_core_size}"]).returncode!=0):
	sys.exit(1)
stage2_size=_get_file_size("build/stages/stage2.bin")
if (subprocess.run(["nasm","src/bootloader/stage1.asm","-f","bin","-Wall","-Werror","-O3","-o","build/stages/stage1.bin",f"-D__BOOTLOADER_STAGE2_SIZE__={stage2_size}",f"-D__BOOTLOADER_VERSION__={version}"]).returncode!=0):
	sys.exit(1)
stage1_size=_get_file_size("build/stages/stage1.bin")
with open("build/iso/core.bin","wb") as wf:
	_copy_file("build/stages/stage1.bin",wf)
	_copy_file("build/stages/stage2.bin",wf)
	_copy_file("build/stages/stage3.bin",wf)
with open("build/iso/os.img","wb") as wf:
	_copy_file("build/iso/core.bin",wf)
	_pad_file(wf,OS_IMAGE_SIZE-kernel_core_size-stage2_size-stage1_size)
changed_files,file_hash_list=_load_changed_files(USER_HASH_FILE_PATH,USER_FILE_DIRECTORY)
runtime_object_files=_compile_user_files(USER_FILE_DIRECTORY+"/runtime",changed_files,file_hash_list)
for program in os.listdir(USER_FILE_DIRECTORY):
	if (program=="runtime"):
		continue
	object_files=runtime_object_files+_compile_user_files(USER_FILE_DIRECTORY+"/"+program,changed_files,file_hash_list)
	if (error or subprocess.run(["ld","-melf_x86_64","-o",f"build/iso/{program}.elf"]+object_files+EXTRA_LINKER_OPTIONS).returncode!=0):
		sys.exit(1)
_save_file_hash_list(file_hash_list,USER_HASH_FILE_PATH)
with open("build/iso/startup.txt","w") as wf:
	wf.write("/install.elf\n")
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
		"-boot","order=dc",
		"-drive","file=build/hdd.qcow2,if=none,id=hdd",
		"-drive","file=build/ssd.qcow2,if=none,id=ssd",
		"-drive","file=build/os.iso,index=0,media=cdrom,readonly=true,id=cd",
		"-device","ahci,id=ahci",
		"-device","ide-hd,drive=hdd,bus=ahci.0",
		"-device","nvme,serial=00112233,drive=ssd",
		"-m","1G",
		"-netdev","l2tpv3,id=network,src=127.0.0.1,dst=127.0.0.1,udp=on,srcport=7555,dstport=7556,rxsession=0xffffffff,txsession=0xffffffff,counter=off",
		"-device","e1000,netdev=network",
		"-cpu","host,tsc,invtsc","-smp","2","-accel","kvm",
		"-nographic","-display","none"
	])
