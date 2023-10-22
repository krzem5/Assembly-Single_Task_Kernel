import array
import binascii
import hashlib
import initramfs
import kfs2
import os
import socket
import struct
import subprocess
import sys
import threading
import time



BYPASS_KVM_LOCK=False



MODE_NORMAL=0
MODE_COVERAGE=1
MODE_RELEASE=2



mode=MODE_NORMAL
if ("--release" in sys.argv):
	mode=MODE_RELEASE
if ("--coverage" in sys.argv):
	mode=MODE_COVERAGE



BUILD_DIRECTORIES=[
	"build",
	"build/hashes",
	"build/hashes/kernel",
	"build/hashes/modules",
	"build/hashes/uefi",
	"build/hashes/user",
	"build/initramfs",
	"build/initramfs/boot",
	"build/initramfs/module",
	"build/module",
	"build/objects",
	"build/objects/kernel",
	"build/objects/kernel_coverage",
	"build/objects/kernel_debug",
	"build/objects/modules",
	"build/objects/modules_debug",
	"build/objects/uefi",
	"build/objects/user",
	"build/objects/user_debug",
	"build/partitions",
	"build/uefi",
	"build/user",
	"build/vm",
	"src/kernel/_generated",
	"src/user/runtime/_generated",
	"src/user/runtime/_generated/include",
	"src/user/runtime/_generated/include/user",
	"src/user/runtime/user/_generated"
]
SYSCALL_SOURCE_FILE_PATH="src/syscalls.txt"
UEFI_HASH_FILE_PATH="build/hashes/uefi/uefi.txt"
UEFI_FILE_DIRECTORY="src/uefi/"
UEFI_OBJECT_FILE_DIRECTORY="build/objects/uefi/"
KERNEL_HASH_FILE_PATH={
	MODE_NORMAL: "build/hashes/kernel/debug.txt",
	MODE_COVERAGE: "build/hashes/kernel/coverage.txt",
	MODE_RELEASE: "build/hashes/kernel/release.txt"
}[mode]
KERNEL_OBJECT_FILE_DIRECTORY={
	MODE_NORMAL: "build/objects/kernel_debug/",
	MODE_COVERAGE: "build/objects/kernel_coverage/",
	MODE_RELEASE: "build/objects/kernel/"
}[mode]
KERNEL_EXTRA_COMPILER_OPTIONS={
	MODE_NORMAL: ["-ggdb","-O1"],
	MODE_COVERAGE: ["--coverage","-fprofile-arcs","-ftest-coverage","-fprofile-info-section","-fprofile-update=atomic","-DKERNEL_COVERAGE_ENABLED=1","-O1"],
	MODE_RELEASE: ["-O3","-g0"]
}[mode]
KERNEL_EXTRA_LINKER_PREPROCESSING_OPTIONS={
	MODE_NORMAL: ["-D_KERNEL_DEBUG_BUILD_"],
	MODE_COVERAGE: ["-D_KERNEL_COVERAGE_BUILD_"],
	MODE_RELEASE: []
}[mode]
KERNEL_EXTRA_LINKER_OPTIONS={
	MODE_NORMAL: ["-g"],
	MODE_COVERAGE: ["-g"],
	MODE_RELEASE: []
}[mode]
MODULE_HASH_FILE_SUFFIX={
	MODE_NORMAL: ".txt",
	MODE_COVERAGE: ".txt",
	MODE_RELEASE: ".release.txt"
}[mode]
MODULE_OBJECT_FILE_DIRECTORY={
	MODE_NORMAL: "build/objects/modules_debug/",
	MODE_COVERAGE: "build/objects/modules_debug/",
	MODE_RELEASE: "build/objects/modules/"
}[mode]
MODULE_EXTRA_COMPILER_OPTIONS={
	MODE_NORMAL: ["-ggdb","-O1"],
	MODE_COVERAGE: ["--coverage","-fprofile-arcs","-ftest-coverage","-fprofile-info-section","-fprofile-update=atomic","-DKERNEL_COVERAGE_ENABLED=1","-O1"],
	MODE_RELEASE: ["-O3","-g0"]
}[mode]
MODULE_EXTRA_LINKER_OPTIONS={
	MODE_NORMAL: ["-g"],
	MODE_COVERAGE: ["-g"],
	MODE_RELEASE: []
}[mode]
USER_HASH_FILE_SUFFIX={
	MODE_NORMAL: ".txt",
	MODE_COVERAGE: ".txt",
	MODE_RELEASE: ".release.txt"
}[mode]
USER_OBJECT_FILE_DIRECTORY={
	MODE_NORMAL: "build/objects/user_debug/",
	MODE_COVERAGE: "build/objects/user/",
	MODE_RELEASE: "build/objects/user/"
}[mode]
USER_EXTRA_COMPILER_OPTIONS={
	MODE_NORMAL: ["-O0","-ggdb","-fno-omit-frame-pointer"],
	MODE_COVERAGE: ["-O0","-ggdb","-fno-omit-frame-pointer","-DKERNEL_COVERAGE_ENABLED=1"],
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
SOURCE_FILE_SUFFIXES=[".asm",".c"]
KERNEL_FILE_DIRECTORY="src/kernel"
KERNEL_SYMBOL_FILE_PATH="build/kernel_symbols.c"
MODULE_FILE_DIRECTORY="src/modules"
USER_FILE_DIRECTORY="src/user"
OS_IMAGE_SIZE=1440*1024
INSTALL_DISK_SIZE=262144
INSTALL_DISK_BLOCK_SIZE=512
INITRAMFS_SIZE=512
COVERAGE_FILE_REPORT_MARKER=0xb8bcbbbe41444347



def _generate_syscalls(file_path):
	syscalls=[]
	with open(file_path,"r") as rf:
		for line in rf.read().split("\n"):
			line=line.strip()
			if (not line):
				continue
			name=line.split("(")[0].strip()
			args=tuple(line.split("(")[1].split(")")[0].strip().split(","))
			if (args==("void",)):
				args=tuple()
			attrs=line.split(")")[1].split("->")[0].strip()
			ret=line.split("->")[1].strip()
			syscalls.append((name,args,attrs,ret))
	syscalls=sorted(syscalls,key=lambda e:e[0])
	syscalls.insert(0,("invalid",tuple(),"","void"))
	with open("src/user/runtime/user/_generated/syscall.asm","w") as wf:
		wf.write("[bits 64]\n")
		for i,(name,args,_,ret) in enumerate(syscalls):
			wf.write(f"\n\n\nsection .text._syscall_{name} exec nowrite\nglobal _syscall_{name}\n_syscall_{name}:\n\tmov rax, {i}\n")
			if (len(args)>3):
				wf.write("\tmov r8, rcx\n")
			wf.write("\tsyscall\n\tret\n")
	with open("src/user/runtime/_generated/include/user/syscall.h","w") as wf:
		wf.write("#ifndef _USER_SYSCALL_H_\n#define _USER_SYSCALL_H_ 1\n#include <user/types.h>\n\n\n\n")
		for name,args,attrs,ret in syscalls:
			wf.write(f"{ret} {'__attribute__(('+attrs+')) ' if attrs else ''}_syscall_{name}({','.join(args) if args else 'void'});\n\n\n\n")
		wf.write("#endif\n")
	with open("src/kernel/_generated/syscalls.c","w") as wf:
		wf.write("#include <kernel/syscall/syscall.h>\n#include <kernel/types.h>\n\n\n\n")
		for name,_,_,_ in syscalls:
			wf.write(f"extern void syscall_{name}(syscall_registers_t* regs);\n")
		wf.write(f"\n\n\nconst u64 _syscall_count={len(syscalls)};\n\n\n\nconst void* _syscall_handlers[{len(syscalls)}]={{\n")
		for name,_,_,_ in syscalls:
			wf.write(f"\tsyscall_{name},\n")
		wf.write("};\n")



def _generate_kernel_version():
	version=time.time_ns()
	with open("src/kernel/_generated/version.c","w") as wf:
		wf.write(f"#include <kernel/types.h>\n\n\n\nconst u64 __version=0x{version:016x};\n")
	return version



def _read_file(file_path):
	with open(file_path,"rb") as rf:
		return rf.read()


def _copy_file(src,dst):
	with open(src,"rb") as rf,open(dst,"wb") as wf:
		wf.write(rf.read())



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
		if (len(line)<3 or line[2].startswith("__func__") or "." in line[2]):
			continue
		out[line[2]]=int(line[0],16)
	return out



def _read_extracted_object_file_symbols(object_file,out):
	with open(object_file+".syms","r") as rf:
		for symbol in rf.read().split("\n"):
			symbol=symbol.strip()
			if (symbol):
				out.append(symbol)



def _extract_object_file_symbol_names(object_file,out):
	with open(object_file+".syms","w") as wf:
		for line in subprocess.run(["nm","-f","bsd","--defined-only",object_file],stdout=subprocess.PIPE).stdout.decode("utf-8").split("\n"):
			line=line.strip().split(" ")
			if (len(line)<3 or line[1].lower() in "vw" or line[2].startswith("__func__") or "." in line[2]):
				continue
			out.append(line[2])
			wf.write(line[2]+"\n")



def _generate_symbol_file(kernel_symbols,file_path):
	with open(file_path,"w") as wf:
		wf.write("typedef unsigned long long int u64;\nconst u64 kernel_symbols[]={\n")
		for symbol in sorted(kernel_symbols):
			wf.write(f"\t0,(u64)\"{symbol}\",\n")
		wf.write("\t0,0\n};\n")
	object_file=KERNEL_OBJECT_FILE_DIRECTORY+file_path.replace("/","#")+".o"
	if (subprocess.run(["gcc-12","-mcmodel=large","-fno-lto","-fno-pie","-fno-common","-fno-builtin","-nostdinc","-nostdlib","-ffreestanding","-m64","-Wall","-Werror","-O3","-g0","-o",object_file,"-c",file_path]).returncode!=0):
		sys.exit(1)
	os.remove(file_path)
	return object_file



def _patch_kernel(file_path,kernel_symbols):
	address_offset=kernel_symbols["__KERNEL_SECTION_kernel_START__"]
	with open(file_path,"r+b") as wf:
		wf.seek(kernel_symbols["_idt_data"]-address_offset)
		for i in range(0,256):
			address=kernel_symbols[f"_isr_entry_{i}"]
			ist=0
			if (i==14):
				ist=1
			elif (i==32):
				ist=2
			wf.write(struct.pack("<HIHQ",address&0xffff,0x8e000008|(ist<<16),(address>>16)&0xffff,address>>32))
		offset=kernel_symbols["kernel_symbols"]-address_offset
		while (True):
			wf.seek(offset+8)
			name_address=struct.unpack("<Q",wf.read(8))[0]
			if (not name_address):
				break
			wf.seek(name_address-address_offset)
			name=""
			while (True):
				char=wf.read(1)[0]
				if (not char):
					break
				name+=chr(char)
			wf.seek(offset)
			wf.write(struct.pack("<Q",kernel_symbols[name]))
			offset+=16



def _compile_module(module):
	hash_file_path=f"build/hashes/modules/"+module+MODULE_HASH_FILE_SUFFIX
	changed_files,file_hash_list=_load_changed_files(hash_file_path,MODULE_FILE_DIRECTORY+"/"+module,KERNEL_FILE_DIRECTORY+"/include")
	object_files=[]
	error=False
	for root,_,files in os.walk(MODULE_FILE_DIRECTORY+"/"+module):
		for file_name in files:
			suffix=file_name[file_name.rindex("."):]
			if (suffix not in SOURCE_FILE_SUFFIXES):
				continue
			file=os.path.join(root,file_name)
			object_file=MODULE_OBJECT_FILE_DIRECTORY+file.replace("/","#")+".o"
			object_files.append(object_file)
			if (_file_not_changed(changed_files,object_file+".deps")):
				continue
			command=None
			if (suffix==".c"):
				command=["gcc-12","-fno-common","-fno-builtin","-nostdlib","-fno-omit-frame-pointer","-fno-asynchronous-unwind-tables","-ffreestanding","-fplt","-fno-pie","-fpic","-m64","-Wall","-Werror","-c","-o",object_file,"-c",file,"-DNULL=((void*)0)",f"-I{MODULE_FILE_DIRECTORY}/{module}/include",f"-I{KERNEL_FILE_DIRECTORY}/include"]+MODULE_EXTRA_COMPILER_OPTIONS
			else:
				command=["nasm","-f","elf64","-Wall","-Werror","-O3","-o",object_file,file]+MODULE_EXTRA_ASSEMBLY_COMPILER_OPTIONS
			print(file)
			if (subprocess.run(command+["-MD","-MT",object_file,"-MF",object_file+".deps"]).returncode!=0):
				del file_hash_list[file]
				error=True
	_save_file_hash_list(file_hash_list,hash_file_path)
	if (error or subprocess.run(["ld","-znoexecstack","-melf_x86_64","-Bsymbolic","-r","-o",f"build/module/{module}.mod"]+object_files+MODULE_EXTRA_LINKER_OPTIONS).returncode!=0):
		sys.exit(1)



def _compile_user_files(program):
	hash_file_path=f"build/hashes/user/"+program+USER_HASH_FILE_SUFFIX
	changed_files,file_hash_list=_load_changed_files(hash_file_path,USER_FILE_DIRECTORY+"/"+program,USER_FILE_DIRECTORY+"/runtime")
	object_files=[]
	error=False
	for root,_,files in os.walk(USER_FILE_DIRECTORY+"/"+program):
		for file_name in files:
			suffix=file_name[file_name.rindex("."):]
			if (suffix not in SOURCE_FILE_SUFFIXES):
				continue
			file=os.path.join(root,file_name)
			object_file=USER_OBJECT_FILE_DIRECTORY+file.replace("/","#")+".o"
			object_files.append(object_file)
			if (_file_not_changed(changed_files,object_file+".deps")):
				continue
			command=None
			if (suffix==".c"):
				command=["gcc-12","-fno-common","-fno-builtin","-nostdlib","-ffreestanding","-fno-pie","-fno-pic","-m64","-Wall","-Werror","-c","-o",object_file,"-c",file,"-DNULL=((void*)0)",f"-I{USER_FILE_DIRECTORY}/{program}/include",f"-I{USER_FILE_DIRECTORY}/runtime/include",f"-I{USER_FILE_DIRECTORY}/runtime/_generated/include"]+USER_EXTRA_COMPILER_OPTIONS
			else:
				command=["nasm","-f","elf64","-Wall","-Werror","-O3","-o",object_file,file]+USER_EXTRA_ASSEMBLY_COMPILER_OPTIONS
			print(file)
			if (subprocess.run(command+["-MD","-MT",object_file,"-MF",object_file+".deps"]).returncode!=0):
				del file_hash_list[file]
				error=True
	_save_file_hash_list(file_hash_list,hash_file_path)
	if (error):
		sys.exit(1)
	return object_files



def _generate_coverage_report(vm_output_file_path,output_file_path):
	for file in os.listdir(KERNEL_OBJECT_FILE_DIRECTORY):
		if (file.endswith(".gcda")):
			os.remove(os.path.join(KERNEL_OBJECT_FILE_DIRECTORY,file))
	for file in os.listdir(MODULE_OBJECT_FILE_DIRECTORY):
		if (file.endswith(".gcda")):
			os.remove(os.path.join(MODULE_OBJECT_FILE_DIRECTORY,file))
	with open(vm_output_file_path,"rb") as rf:
		while (True):
			buffer=rf.read(8)
			if (len(buffer)<8):
				break
			if (struct.unpack("<Q",buffer)[0]!=COVERAGE_FILE_REPORT_MARKER):
				rf.seek(rf.tell()-7)
				continue
			version,checksum,file_name_length=struct.unpack("III",rf.read(12))
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
	subprocess.run(["lcov","-c","-d",KERNEL_OBJECT_FILE_DIRECTORY,"-d",MODULE_OBJECT_FILE_DIRECTORY,"--gcov-tool","gcov-12","-o",output_file_path])



def _process_packet(buffer):
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



def _kvm_flags():
	if (not os.path.exists("/dev/kvm")):
		return []
	with open("/proc/cpuinfo","r") as rf:
		if (" vmx" not in rf.read()):
			return []
	with open("/sys/devices/system/clocksource/clocksource0/current_clocksource","r") as rf:
		if ("tsc" in rf.read() and not BYPASS_KVM_LOCK):
			print("\x1b[1m\x1b[38;2;231;72;86mKVM support disabled due to kernel TSC clock source\x1b[0m")
			return []
	return ["-accel","kvm"]



for dir in BUILD_DIRECTORIES:
	if (not os.path.exists(dir)):
		os.mkdir(dir)
_generate_syscalls(SYSCALL_SOURCE_FILE_PATH)
#####################################################################################################################################
changed_files,file_hash_list=_load_changed_files(UEFI_HASH_FILE_PATH,UEFI_FILE_DIRECTORY)
object_files=[]
rebuild_uefi_partition=False
error=False
for root,_,files in os.walk(UEFI_FILE_DIRECTORY):
	for file_name in files:
		suffix=file_name[file_name.rindex("."):]
		if (suffix not in SOURCE_FILE_SUFFIXES):
			continue
		file=os.path.join(root,file_name)
		object_file=UEFI_OBJECT_FILE_DIRECTORY+file.replace("/","#")+".o"
		object_files.append(object_file)
		if (_file_not_changed(changed_files,object_file+".deps")):
			continue
		rebuild_uefi_partition=True
		command=None
		if (suffix==".c"):
			command=["gcc-12","-I/usr/include/efi","-I/usr/include/efi/x86_64","-fno-stack-protector","-ffreestanding","-O3","-fpic","-fshort-wchar","-mno-red-zone","-maccumulate-outgoing-args","-DGNU_EFI_USE_MS_ABI","-Dx86_64","-m64","-Wall","-Werror","-DNULL=((void*)0)","-o",object_file,"-c",file,f"-I{UEFI_FILE_DIRECTORY}/include"]
		else:
			command=["nasm","-f","elf64","-O3","-Wall","-Werror","-o",object_file,file]
		print(file)
		if (subprocess.run(command+["-MD","-MT",object_file,"-MF",object_file+".deps"]).returncode!=0):
			del file_hash_list[file]
			error=True
_save_file_hash_list(file_hash_list,UEFI_HASH_FILE_PATH)
if (error or subprocess.run(["ld","-nostdlib","-znocombreloc","-znoexecstack","-fshort-wchar","-T","/usr/lib/elf_x86_64_efi.lds","-shared","-Bsymbolic","-o","build/uefi/loader.so","/usr/lib/gcc/x86_64-linux-gnu/12/libgcc.a"]+object_files).returncode!=0 or subprocess.run(["objcopy","-j",".text","-j",".sdata","-j",".data","-j",".dynamic","-j",".dynsym","-j",".rel","-j",".rela","-j",".reloc","-S","--target=efi-app-x86_64","build/uefi/loader.so","build/uefi/loader.efi"]).returncode!=0):
	sys.exit(1)
#####################################################################################################################################
version=_generate_kernel_version()
changed_files,file_hash_list=_load_changed_files(KERNEL_HASH_FILE_PATH,KERNEL_FILE_DIRECTORY)
object_files=[]
rebuild_data_partition=False
error=False
kernel_symbols=[]
for root,_,files in os.walk(KERNEL_FILE_DIRECTORY):
	for file_name in files:
		suffix=file_name[file_name.rindex("."):]
		if (suffix not in SOURCE_FILE_SUFFIXES):
			continue
		file=os.path.join(root,file_name)
		object_file=KERNEL_OBJECT_FILE_DIRECTORY+file.replace("/","#")+".o"
		object_files.append(object_file)
		if (_file_not_changed(changed_files,object_file+".deps")):
			_read_extracted_object_file_symbols(object_file,kernel_symbols)
			continue
		command=None
		rebuild_data_partition=True
		if (suffix==".c"):
			command=["gcc-12","-mcmodel=kernel","-mno-red-zone","-mno-mmx","-mno-sse","-mno-sse2","-mbmi","-mbmi2","-fno-lto","-fno-pie","-fno-common","-fno-builtin","-fno-stack-protector","-fno-asynchronous-unwind-tables","-nostdinc","-nostdlib","-ffreestanding","-m64","-Wall","-Werror","-c","-ftree-loop-distribute-patterns","-O3","-g0","-fno-omit-frame-pointer","-DNULL=((void*)0)","-o",object_file,"-c",file,f"-I{KERNEL_FILE_DIRECTORY}/include"]+KERNEL_EXTRA_COMPILER_OPTIONS
		else:
			command=["nasm","-f","elf64","-O3","-Wall","-Werror","-o",object_file,file]
		print(file)
		if (subprocess.run(command+["-MD","-MT",object_file,"-MF",object_file+".deps"]).returncode!=0):
			del file_hash_list[file]
			error=True
		else:
			_extract_object_file_symbol_names(object_file,kernel_symbols)
_save_file_hash_list(file_hash_list,KERNEL_HASH_FILE_PATH)
object_files.append(_generate_symbol_file(kernel_symbols,KERNEL_SYMBOL_FILE_PATH))
linker_file=KERNEL_OBJECT_FILE_DIRECTORY+"linker.ld"
if (error or subprocess.run(["gcc-12","-E","-o",linker_file,"-x","none"]+KERNEL_EXTRA_LINKER_PREPROCESSING_OPTIONS+["-"],input=_read_file("src/kernel/linker.ld")).returncode!=0 or subprocess.run(["ld","-znoexecstack","-melf_x86_64","-o","build/kernel.elf","-O3","-T",linker_file]+KERNEL_EXTRA_LINKER_OPTIONS+object_files).returncode!=0 or subprocess.run(["objcopy","-S","-O","binary","build/kernel.elf","build/kernel.bin"]).returncode!=0):
	sys.exit(1)
kernel_symbols=_read_kernel_symbols("build/kernel.elf")
_patch_kernel("build/kernel.bin",kernel_symbols)
#####################################################################################################################################
for module in os.listdir(MODULE_FILE_DIRECTORY):
	_compile_module(module)
#####################################################################################################################################
runtime_object_files=_compile_user_files("runtime")
for program in os.listdir(USER_FILE_DIRECTORY):
	if (program=="runtime"):
		continue
	object_files=runtime_object_files+_compile_user_files(program)
	if (subprocess.run(["ld","-znoexecstack","-melf_x86_64","-o",f"build/user/{program}.elf"]+object_files+USER_EXTRA_LINKER_OPTIONS).returncode!=0):
		sys.exit(1)
#####################################################################################################################################
if (not os.path.exists("build/install_disk.img")):
	rebuild_uefi_partition=True
	rebuild_data_partition=True
	subprocess.run(["dd","if=/dev/zero","of=build/install_disk.img",f"bs={INSTALL_DISK_BLOCK_SIZE}",f"count={INSTALL_DISK_SIZE}"])
	subprocess.run(["parted","build/install_disk.img","-s","-a","minimal","mklabel","gpt"])
	subprocess.run(["parted","build/install_disk.img","-s","-a","minimal","mkpart","EFI","FAT16","34s","93719s"])
	subprocess.run(["parted","build/install_disk.img","-s","-a","minimal","mkpart","DATA","93720s",f"{INSTALL_DISK_SIZE-34}s"])
	subprocess.run(["parted","build/install_disk.img","-s","-a","minimal","toggle","1","boot"])
if (not os.path.exists("build/partitions/efi.img")):
	rebuild_uefi_partition=True
	subprocess.run(["dd","if=/dev/zero","of=build/partitions/efi.img",f"bs={INSTALL_DISK_BLOCK_SIZE}","count=93686"])
	subprocess.run(["mformat","-i","build/partitions/efi.img","-h","32","-t","32","-n","64","-c","1","-l","LABEL"])
	subprocess.run(["mmd","-i","build/partitions/efi.img","::/EFI","::/EFI/BOOT"])
if (rebuild_uefi_partition):
	subprocess.run(["mcopy","-i","build/partitions/efi.img","-D","o","build/uefi/loader.efi","::/EFI/BOOT/BOOTX64.EFI"])
	subprocess.run(["dd","if=build/partitions/efi.img","of=build/install_disk.img",f"bs={INSTALL_DISK_BLOCK_SIZE}","count=93686","seek=34","conv=notrunc"])
if (rebuild_data_partition):
	_copy_file("build/user/shell.elf","build/initramfs/boot/boot.elf")
	for module in os.listdir(MODULE_FILE_DIRECTORY):
		_copy_file(f"build/module/{module}.mod",f"build/initramfs/module/{module}.mod")
	initramfs.create("build/initramfs","build/partitions/initramfs.img")
	data_fs=kfs2.KFS2FileBackend("build/install_disk.img",INSTALL_DISK_BLOCK_SIZE,93720,INSTALL_DISK_SIZE-34)
	kfs2.format_partition(data_fs)
	with open("build/kernel.bin","rb") as kernel_rf,open("build/partitions/initramfs.img","rb") as initramfs_rf:
		kernel_inode=kfs2.get_inode(data_fs,"/boot/kernel.bin")
		initramfs_inode=kfs2.get_inode(data_fs,"/boot/initramfs")
		kfs2.set_file_content(data_fs,kernel_inode,kernel_rf.read()+b"\x00"*(kernel_symbols["__KERNEL_SECTION_kernel_bss_END__"]-kernel_symbols["__KERNEL_SECTION_kernel_bss_START__"]))
		kfs2.set_file_content(data_fs,initramfs_inode,initramfs_rf.read())
		kfs2.set_kernel_inode(data_fs,kernel_inode)
		kfs2.set_initramfs_inode(data_fs,initramfs_inode)
	data_fs.close()
#####################################################################################################################################
if ("--run" in sys.argv):
	if (not os.path.exists("build/vm/hdd.qcow2")):
		if (subprocess.run(["qemu-img","create","-q","-f","qcow2","build/vm/hdd.qcow2","16G"]).returncode!=0):
			sys.exit(1)
	if (not os.path.exists("build/vm/ssd.qcow2")):
		if (subprocess.run(["qemu-img","create","-q","-f","qcow2","build/vm/ssd.qcow2","8G"]).returncode!=0):
			sys.exit(1)
	if (not os.path.exists("build/vm/OVMF_CODE.fd")):
		if (subprocess.run(["cp","/usr/share/OVMF/OVMF_CODE.fd","build/vm/OVMF_CODE.fd"]).returncode!=0):
			sys.exit(1)
	if (not os.path.exists("build/vm/OVMF_VARS.fd")):
		if (subprocess.run(["cp","/usr/share/OVMF/OVMF_VARS.fd","build/vm/OVMF_VARS.fd"]).returncode!=0):
			sys.exit(1)
	_start_l2tpv3_thread()
	subprocess.run([
		"qemu-system-x86_64",
		# "-d","int,cpu_reset","--no-reboot",
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
		"-device","usb-storage,bus=xhci.0,drive=bootusb",
		# Network
		"-netdev","l2tpv3,id=network,src=127.0.0.1,dst=127.0.0.1,udp=on,srcport=7555,dstport=7556,rxsession=0xffffffff,txsession=0xffffffff,counter=off",
		"-device","e1000,netdev=network",
		# Memory
		"-m","2G,slots=2,maxmem=4G",
		# "-mem-prealloc",
		"-object","memory-backend-ram,size=1G,id=mem0",
		"-object","memory-backend-ram,size=1G,id=mem1",
		# CPU
		"-cpu","Skylake-Client-v1,tsc,invtsc,avx,avx2,bmi1,bmi2,pdpe1gb",
		"-smp","8,sockets=2,cores=2,threads=2,maxcpus=8",
		# "-device","intel-iommu","-machine","q35", ### Required for 256-288 cores
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
		"-machine","hmat=on",
		"-uuid","00112233-4455-6677-8899-aabbccddeeff",
		"-smbios","type=2,serial=SERIAL_NUMBER"
	])
	if (mode==MODE_COVERAGE):
		_generate_coverage_report("build/raw_coverage","build/coverage.lcov")
		os.remove("build/raw_coverage")
