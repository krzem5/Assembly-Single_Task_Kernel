import array
import binascii
import compression
import hashlib
import initramfs
import kfs2
import os
import struct
import subprocess
import sys
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
	"build/hashes/lib",
	"build/hashes/modules",
	"build/hashes/uefi",
	"build/hashes/user",
	"build/initramfs",
	"build/initramfs/boot",
	"build/initramfs/boot/module",
	"build/lib",
	"build/module",
	"build/objects",
	"build/objects/kernel",
	"build/objects/kernel_coverage",
	"build/objects/kernel_debug",
	"build/objects/lib",
	"build/objects/lib_debug",
	"build/objects/modules",
	"build/objects/modules_coverage",
	"build/objects/modules_debug",
	"build/objects/uefi",
	"build/objects/user",
	"build/objects/user_debug",
	"build/partitions",
	"build/uefi",
	"build/user",
	"build/vm",
	"src/kernel/_generated"
]
SYSCALL_SOURCE_FILE_PATH="src/kernel/syscalls.txt"
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
	MODE_RELEASE: ["-O3","-g0","-DKERNEL_DISABLE_ASSERT=1","-DKERNEL_DISABLE_WATCHDOG=1"]
}[mode]
KERNEL_EXTRA_LINKER_PREPROCESSING_OPTIONS={
	MODE_NORMAL: ["-D_KERNEL_DEBUG_BUILD_=1"],
	MODE_COVERAGE: ["-D_KERNEL_COVERAGE_BUILD_=1"],
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
	MODE_COVERAGE: "build/objects/modules_coverage/",
	MODE_RELEASE: "build/objects/modules/"
}[mode]
MODULE_EXTRA_COMPILER_OPTIONS={
	MODE_NORMAL: ["-ggdb","-O1"],
	MODE_COVERAGE: ["--coverage","-fprofile-arcs","-ftest-coverage","-fprofile-info-section","-fprofile-update=atomic","-DKERNEL_COVERAGE_ENABLED=1","-O1"],
	MODE_RELEASE: ["-O3","-g0","-DKERNEL_DISABLE_ASSERT=1"]
}[mode]
MODULE_EXTRA_LINKER_OPTIONS={
	MODE_NORMAL: ["-g"],
	MODE_COVERAGE: ["-g"],
	MODE_RELEASE: []
}[mode]
LIBRARY_HASH_FILE_SUFFIX={
	MODE_NORMAL: ".txt",
	MODE_COVERAGE: ".txt",
	MODE_RELEASE: ".release.txt"
}[mode]
LIBRARY_OBJECT_FILE_DIRECTORY={
	MODE_NORMAL: "build/objects/lib_debug/",
	MODE_COVERAGE: "build/objects/lib_debug/",
	MODE_RELEASE: "build/objects/lib/"
}[mode]
LIBRARY_EXTRA_COMPILER_OPTIONS={
	MODE_NORMAL: ["-O0","-ggdb","-fno-omit-frame-pointer"],
	MODE_COVERAGE: ["-O0","-ggdb","-fno-omit-frame-pointer"],
	MODE_RELEASE: ["-O3","-g0","-fdata-sections","-ffunction-sections","-fomit-frame-pointer"]
}[mode]
LIBRARY_EXTRA_ASSEMBLY_COMPILER_OPTIONS={
	MODE_NORMAL: ["-O0","-g"],
	MODE_COVERAGE: ["-O0","-g"],
	MODE_RELEASE: ["-O3"]
}[mode]
LIBRARY_EXTRA_LINKER_OPTIONS={
	MODE_NORMAL: ["-O0","-g"],
	MODE_COVERAGE: ["-O0","-g"],
	MODE_RELEASE: ["-O3","--gc-sections","-s"]
}[mode]
USER_HASH_FILE_SUFFIX={
	MODE_NORMAL: ".txt",
	MODE_COVERAGE: ".txt",
	MODE_RELEASE: ".release.txt"
}[mode]
USER_OBJECT_FILE_DIRECTORY={
	MODE_NORMAL: "build/objects/user_debug/",
	MODE_COVERAGE: "build/objects/user_debug/",
	MODE_RELEASE: "build/objects/user/"
}[mode]
USER_EXTRA_COMPILER_OPTIONS={
	MODE_NORMAL: ["-O0","-ggdb","-fno-omit-frame-pointer"],
	MODE_COVERAGE: ["-O0","-ggdb","-fno-omit-frame-pointer"],
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
	MODE_RELEASE: ["-O3","--gc-sections","-s"]
}[mode]
SOURCE_FILE_SUFFIXES=[".asm",".c"]
KERNEL_FILE_DIRECTORY="src/kernel"
KERNEL_SYMBOL_FILE_PATH="build/kernel_symbols.c"
MODULE_FILE_DIRECTORY="src/module"
LIBRARY_FILE_DIRECTORY="src/lib"
USER_FILE_DIRECTORY="src/user"
MODULE_ORDER_FILE_PATH="src/module/module_order.config"
COMPRESSION_LEVEL=compression.COMPRESSION_LEVEL_FAST
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
	with open("src/lib/sys/sys/syscall.asm","w") as wf:
		wf.write("[bits 64]\n")
		for i,(name,args,_,ret) in enumerate(syscalls):
			wf.write(f"\n\n\nsection .text._syscall_{name} exec nowrite\nglobal _syscall_{name}:function _syscall_{name}_size\n_syscall_{name}:\n\tmov rax, {i+1}\n")
			if (len(args)>4):
				wf.write("\tmov r9, r8\n")
			if (len(args)>3):
				wf.write("\tmov r8, rcx\n")
			wf.write(f"\tsyscall\n\tret\n_syscall_{name}_size equ $-$$\n")
	with open("src/lib/sys/include/sys/syscall.h","w") as wf:
		wf.write("#ifndef _SYS_SYSCALL_H_\n#define _SYS_SYSCALL_H_ 1\n#include <sys/types.h>\n\n\n\n")
		for name,args,attrs,ret in syscalls:
			wf.write(f"{ret} {'__attribute__(('+attrs+')) ' if attrs else ''}_syscall_{name}({','.join(args) if args else 'void'});\n\n\n\n")
		wf.write("#endif\n")
	with open("src/kernel/_generated/syscalls.c","w") as wf:
		wf.write("#include <kernel/isr/isr.h>\n#include <kernel/types.h>\n\n\n\n")
		for name,_,_,_ in syscalls:
			wf.write(f"extern void syscall_{name}(isr_state_t* regs);\n")
		wf.write(f"\n\n\nconst u64 _syscall_count={len(syscalls)+1};\n\n\n\nconst void*const _syscall_handlers[{len(syscalls)+1}]={{\n\tNULL,\n")
		for name,_,_,_ in syscalls:
			wf.write(f"\tsyscall_{name},\n")
		wf.write("};\n")



def _generate_kernel_build_info():
	version=time.time_ns()
	with open("src/kernel/_generated/build_info.c","w") as wf:
		wf.write(f"#include <kernel/types.h>\n\n\n\nKERNEL_PUBLIC const u64 __version=0x{version:016x};\nKERNEL_PUBLIC const char* __build_name=\"x86_64 { {MODE_NORMAL:'debug',MODE_COVERAGE:'coverage',MODE_RELEASE:'release'}[mode]}\";\n")
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
	for line in subprocess.run(["objdump","-t",file_path],stdout=subprocess.PIPE).stdout.decode("utf-8").split("\n"):
		line=line.strip().split("\t")
		if (len(line)!=2 or line[1].startswith("__func__") or "." in line[1].split(" ")[-1]):
			continue
		out[line[1].split(" ")[-1]]=(int(line[0][:16],16),".hidden" not in line[1] and line[0][17]=="g")
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
	kernel_symbols=tuple(sorted(set(kernel_symbols)))
	with open(file_path,"w") as wf:
		wf.write("typedef unsigned long long int u64;\n")
		for i,symbol in enumerate(kernel_symbols):
			wf.write(f"static const char __attribute__((section(\".erdata\"))) _sym_{i}[]=\"{symbol}\";\n")
		wf.write("const u64 __attribute__((section(\".erdata\"))) _raw_kernel_symbols[]={\n")
		for i in range(0,len(kernel_symbols)):
			wf.write(f"\t0,(u64)_sym_{i},\n")
		wf.write("\t0,0\n};\n")
	object_file=KERNEL_OBJECT_FILE_DIRECTORY+file_path.replace("/","#")+".o"
	if (subprocess.run(["gcc-12","-mcmodel=large","-fno-lto","-fno-pie","-fno-common","-fno-builtin","-nostdinc","-nostdlib","-ffreestanding","-m64","-Wall","-Werror","-O3","-g0","-o",object_file,"-c",file_path]).returncode!=0):
		sys.exit(1)
	os.remove(file_path)
	return object_file



def _patch_kernel(file_path,kernel_symbols):
	address_offset=kernel_symbols["__KERNEL_SECTION_kernel_START__"][0]
	with open(file_path,"r+b") as wf:
		wf.seek(kernel_symbols["_idt_data"][0]-address_offset)
		for i in range(0,256):
			address=kernel_symbols[f"_isr_entry_{i}"][0]
			ist=0
			if (i==14):
				ist=1
			elif (i==32):
				ist=2
			wf.write(struct.pack("<HIHQ",address&0xffff,0x8e000008|(ist<<16),(address>>16)&0xffff,address>>32))
		offset=kernel_symbols["_raw_kernel_symbols"][0]-address_offset
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
			wf.write(struct.pack("<Q",kernel_symbols[name][0]^((not kernel_symbols[name][1])<<63)))
			offset+=16



def _compress(file_path):
	with open(file_path,"rb") as rf,open(file_path+".compressed","wb") as wf:
		compression.compress(rf.read(),COMPRESSION_LEVEL,wf)



def _compile_module(module,dependencies):
	hash_file_path=f"build/hashes/modules/"+module+MODULE_HASH_FILE_SUFFIX
	changed_files,file_hash_list=_load_changed_files(hash_file_path,MODULE_FILE_DIRECTORY+"/"+module,KERNEL_FILE_DIRECTORY+"/include",*[MODULE_FILE_DIRECTORY+"/"+dep for dep in dependencies])
	object_files=[]
	included_directories=[f"-I{MODULE_FILE_DIRECTORY}/{module}/include",f"-I{KERNEL_FILE_DIRECTORY}/include"]+[f"-I{MODULE_FILE_DIRECTORY}/{dep}/include" for dep in dependencies]
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
				command=["gcc-12","-mcmodel=kernel","-mno-red-zone","-mno-mmx","-mno-sse","-mno-sse2","-mbmi","-mbmi2","-fno-common","-fno-builtin","-nostdlib","-fno-omit-frame-pointer","-fno-asynchronous-unwind-tables","-ffreestanding","-fvisibility=hidden","-fplt","-fno-pie","-fno-pic","-m64","-Wall","-Werror","-Wno-address-of-packed-member","-c","-o",object_file,"-c",file,"-DNULL=((void*)0)"]+included_directories+MODULE_EXTRA_COMPILER_OPTIONS
			else:
				command=["nasm","-f","elf64","-Wall","-Werror","-O3","-o",object_file,file]+MODULE_EXTRA_ASSEMBLY_COMPILER_OPTIONS
			print(file)
			if (subprocess.run(command+["-MD","-MT",object_file,"-MF",object_file+".deps"]).returncode!=0):
				del file_hash_list[file]
				error=True
	_save_file_hash_list(file_hash_list,hash_file_path)
	if (error or subprocess.run(["ld","-znoexecstack","-melf_x86_64","-Bsymbolic","-r","-o",f"build/module/{module}.mod"]+object_files+MODULE_EXTRA_LINKER_OPTIONS).returncode!=0):
		sys.exit(1)



def _compile_library(library,flags,dependencies):
	hash_file_path=f"build/hashes/lib/"+library+LIBRARY_HASH_FILE_SUFFIX
	changed_files,file_hash_list=_load_changed_files(hash_file_path,LIBRARY_FILE_DIRECTORY+"/"+library,*[LIBRARY_FILE_DIRECTORY+"/"+dep for dep in dependencies])
	object_files=[]
	included_directories=[f"-I{LIBRARY_FILE_DIRECTORY}/{library}/include"]+[f"-I{LIBRARY_FILE_DIRECTORY}/{dep}/include" for dep in dependencies]
	error=False
	for root,_,files in os.walk(LIBRARY_FILE_DIRECTORY+"/"+library):
		for file_name in files:
			suffix=file_name[file_name.rindex("."):]
			if (suffix not in SOURCE_FILE_SUFFIXES):
				continue
			file=os.path.join(root,file_name)
			object_file=LIBRARY_OBJECT_FILE_DIRECTORY+file.replace("/","#")+".pic.o"
			object_files.append(object_file)
			if (_file_not_changed(changed_files,object_file+".deps")):
				continue
			command=None
			command=None
			if (suffix==".c"):
				command=["gcc-12","-fno-common","-fno-builtin","-nostdlib","-fvisibility=hidden","-ffreestanding","-shared","-fno-plt","-fpic","-m64","-Wall","-Werror","-c","-o",object_file,"-c",file,"-DNULL=((void*)0)"]+included_directories+LIBRARY_EXTRA_COMPILER_OPTIONS
			else:
				command=["nasm","-f","elf64","-Wall","-Werror","-DBUILD_SHARED=1","-O3","-o",object_file,file]+included_directories+LIBRARY_EXTRA_ASSEMBLY_COMPILER_OPTIONS
			print(file)
			if (subprocess.run(command+["-MD","-MT",object_file,"-MF",object_file+".deps"]).returncode!=0):
				del file_hash_list[file]
				error=True
	_save_file_hash_list(file_hash_list,hash_file_path)
	if (error):
		sys.exit(1)
	if ("nodynamic" not in flags and subprocess.run(["ld","-znoexecstack","-melf_x86_64","--exclude-libs","ALL","-shared","-o",f"build/lib/lib{library}.so"]+object_files+[f"build/lib/lib{dep}.a" for dep in dependencies]+LIBRARY_EXTRA_LINKER_OPTIONS).returncode!=0):
		sys.exit(1)
	if ("nostatic" in flags):
		return
	static_library_file=f"build/lib/lib{library}.a"
	if (os.path.exists(static_library_file)):
		os.remove(static_library_file)
	if (subprocess.run(["ar","rcs",static_library_file]+object_files).returncode!=0):
		sys.exit(1)



def _compile_user_files(program,dependencies):
	hash_file_path=f"build/hashes/user/"+program+USER_HASH_FILE_SUFFIX
	changed_files,file_hash_list=_load_changed_files(hash_file_path,USER_FILE_DIRECTORY+"/"+program,*[LIBRARY_FILE_DIRECTORY+"/"+dep[0] for dep in dependencies])
	object_files=[]
	included_directories=[f"-I{USER_FILE_DIRECTORY}/{program}/include"]+[f"-I{LIBRARY_FILE_DIRECTORY}/{dep[0]}/include" for dep in dependencies]
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
				command=["gcc-12","-fno-common","-fno-builtin","-nostdlib","-ffreestanding","-fno-pie","-fno-pic","-m64","-Wall","-Werror","-c","-o",object_file,"-c",file,"-DNULL=((void*)0)"]+included_directories+USER_EXTRA_COMPILER_OPTIONS
			else:
				command=["nasm","-f","elf64","-Wall","-Werror","-O3","-o",object_file,file]+USER_EXTRA_ASSEMBLY_COMPILER_OPTIONS
			print(file)
			if (subprocess.run(command+["-MD","-MT",object_file,"-MF",object_file+".deps"]).returncode!=0):
				del file_hash_list[file]
				error=True
	_save_file_hash_list(file_hash_list,hash_file_path)
	if (error or subprocess.run(["ld","-znoexecstack","-melf_x86_64","-L","build/lib","-I/lib/ld.so","--exclude-libs","ALL","-o",f"build/user/{name}"]+[(f"-l{dep[0]}" if len(dep)==1 or dep[1]!="static" else f"build/lib/lib{dep[0]}.a") for dep in dependencies]+object_files+USER_EXTRA_LINKER_OPTIONS).returncode!=0):
		sys.exit(1)



def _get_early_modules(file_path):
	out=["os_loader"]
	with open(file_path,"r") as rf:
		for module in rf.read().split("\n"):
			module=module.strip()
			if (module and module[0]!="#" and module.count("=")==1):
				name,type_=module.split("=")
				if (type_=="early"):
					out.append(name)
	return out



def _generate_coverage_report(vm_output_file_path,output_file_path):
	for file in os.listdir(KERNEL_OBJECT_FILE_DIRECTORY):
		if (file.endswith(".gcda")):
			os.remove(os.path.join(KERNEL_OBJECT_FILE_DIRECTORY,file))
	for file in os.listdir(MODULE_OBJECT_FILE_DIRECTORY):
		if (file.endswith(".gcda")):
			os.remove(os.path.join(MODULE_OBJECT_FILE_DIRECTORY,file))
	file_list=[]
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
			file_list.append(file_name)
			with open(file_name[:-5]+".gcno","rb") as gcno_rf:
				stamp=struct.unpack("III",gcno_rf.read(12))[2]
			with open(file_name,"wb") as wf:
				wf.write(b"adcg")
				wf.write(struct.pack("III",version,stamp,checksum))
				for i in range(0,struct.unpack("I",rf.read(4))[0]):
					id_,lineno_checksum,cfg_checksum,counter_count=struct.unpack("IIII",rf.read(16))
					wf.write(struct.pack("IIIIIII",0x01000000,12,id_,lineno_checksum,cfg_checksum,0x01a10000,counter_count<<3))
					wf.write(rf.read(counter_count<<3))
	with open(output_file_path,"w") as wf:
		wf.write("TN:\n")
		function_stats=None
		for line in subprocess.run(["gcov-12","-b","-t"]+file_list,stdout=subprocess.PIPE).stdout.decode("utf-8").split("\n"):
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
version=_generate_kernel_build_info()
changed_files,file_hash_list=_load_changed_files(KERNEL_HASH_FILE_PATH,KERNEL_FILE_DIRECTORY)
object_files=[]
rebuild_data_partition=False
error=False
kernel_symbol_names=[]
for root,_,files in os.walk(KERNEL_FILE_DIRECTORY):
	for file_name in files:
		suffix=file_name[file_name.rindex("."):]
		if (suffix not in SOURCE_FILE_SUFFIXES):
			continue
		file=os.path.join(root,file_name)
		object_file=KERNEL_OBJECT_FILE_DIRECTORY+file.replace("/","#")+".o"
		object_files.append(object_file)
		if (_file_not_changed(changed_files,object_file+".deps")):
			_read_extracted_object_file_symbols(object_file,kernel_symbol_names)
			continue
		command=None
		rebuild_data_partition=True
		if (suffix==".c"):
			command=["gcc-12","-mcmodel=kernel","-mno-red-zone","-mno-mmx","-mno-sse","-mno-sse2","-mbmi","-mbmi2","-fno-lto","-fno-pie","-fno-common","-fno-builtin","-fno-stack-protector","-fno-asynchronous-unwind-tables","-nostdinc","-nostdlib","-ffreestanding","-fvisibility=hidden","-m64","-Wall","-Werror","-Wno-address-of-packed-member","-c","-ftree-loop-distribute-patterns","-O3","-g0","-fno-omit-frame-pointer","-DNULL=((void*)0)","-o",object_file,"-c",file,f"-I{KERNEL_FILE_DIRECTORY}/include"]+KERNEL_EXTRA_COMPILER_OPTIONS
		else:
			command=["nasm","-f","elf64","-O3","-Wall","-Werror","-o",object_file,file]
		print(file)
		if (subprocess.run(command+["-MD","-MT",object_file,"-MF",object_file+".deps"]).returncode!=0):
			del file_hash_list[file]
			error=True
		else:
			_extract_object_file_symbol_names(object_file,kernel_symbol_names)
_save_file_hash_list(file_hash_list,KERNEL_HASH_FILE_PATH)
object_files.append(_generate_symbol_file(kernel_symbol_names,KERNEL_SYMBOL_FILE_PATH))
linker_file=KERNEL_OBJECT_FILE_DIRECTORY+"linker.ld"
if (error or subprocess.run(["gcc-12","-E","-o",linker_file,"-x","none"]+KERNEL_EXTRA_LINKER_PREPROCESSING_OPTIONS+["-"],input=_read_file("src/kernel/linker.ld")).returncode!=0 or subprocess.run(["ld","-znoexecstack","-melf_x86_64","-o","build/kernel.elf","-O3","-T",linker_file]+KERNEL_EXTRA_LINKER_OPTIONS+object_files).returncode!=0 or subprocess.run(["objcopy","-S","-O","binary","build/kernel.elf","build/kernel.bin"]).returncode!=0):
	sys.exit(1)
kernel_symbols=_read_kernel_symbols("build/kernel.elf")
_patch_kernel("build/kernel.bin",kernel_symbols)
with open("build/kernel.bin","ab+") as wf:
	wf.write(b"\x00"*(kernel_symbols["__KERNEL_SECTION_kernel_zw_END__"][0]-kernel_symbols["__KERNEL_SECTION_kernel_zw_START__"][0]))
_compress("build/kernel.bin")
#####################################################################################################################################
with open("src/module/dependencies.txt","r") as rf:
	for line in rf.read().split("\n"):
		line=line.strip()
		if (not line):
			continue
		name,dependencies=line.split(":")
		_compile_module(name,[dep.strip() for dep in dependencies.split(",") if dep.strip()])
#####################################################################################################################################
with open("src/lib/dependencies.txt","r") as rf:
	for line in rf.read().split("\n"):
		line=line.strip()
		if (not line):
			continue
		name,dependencies=line.split(":")
		flags=([] if "@" not in name else [flag.strip() for flag in name.split("@")[1].split(",") if flag.strip()])
		_compile_library(name.split("@")[0],flags,[dep.strip() for dep in dependencies.split(",") if dep.strip()])
#####################################################################################################################################
with open("src/user/dependencies.txt","r") as rf:
	for line in rf.read().split("\n"):
		line=line.strip()
		if (not line):
			continue
		name,dependencies=line.split(":")
		_compile_user_files(name,[dep.strip().split("@") for dep in dependencies.split(",") if dep.strip()])
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
	for module in _get_early_modules(MODULE_ORDER_FILE_PATH):
		_copy_file(f"build/module/{module}.mod",f"build/initramfs/boot/module/{module}.mod")
	_copy_file(MODULE_ORDER_FILE_PATH,"build/initramfs/boot/module/module_order.config")
	initramfs.create("build/initramfs","build/partitions/initramfs.img")
	_compress("build/partitions/initramfs.img")
	data_fs=kfs2.KFS2FileBackend("build/install_disk.img",INSTALL_DISK_BLOCK_SIZE,93720,INSTALL_DISK_SIZE-34)
	kfs2.format_partition(data_fs)
	kfs2.get_inode(data_fs,"/boot",0o000,True)
	kfs2.get_inode(data_fs,"/boot/module",0o000,True)
	kfs2.get_inode(data_fs,"/lib",0o644,True)
	kfs2.get_inode(data_fs,"/bin",0o644,True)
	with open("build/kernel.bin.compressed","rb") as rf:
		kernel_inode=kfs2.get_inode(data_fs,"/boot/kernel.bin",0o000)
		kfs2.set_file_content(data_fs,kernel_inode,rf.read())
		kfs2.set_kernel_inode(data_fs,kernel_inode)
	with open("build/partitions/initramfs.img.compressed","rb") as rf:
		initramfs_inode=kfs2.get_inode(data_fs,"/boot/initramfs",0o000)
		kfs2.set_file_content(data_fs,initramfs_inode,rf.read())
		kfs2.set_initramfs_inode(data_fs,initramfs_inode)
	for library in os.listdir("build/lib"):
		if (not library.endswith(".so")):
			continue
		with open(f"build/lib/{library}","rb") as rf:
			kfs2.set_file_content(data_fs,kfs2.get_inode(data_fs,f"/lib/{library}",0o755),rf.read())
	dynamic_linker_inode=kfs2.get_inode(data_fs,"/lib/ld.so",0o644)
	kfs2.convert_to_link(data_fs,dynamic_linker_inode)
	kfs2.set_file_content(data_fs,dynamic_linker_inode,b"/lib/liblinker.so")
	for program in os.listdir("build/user"):
		with open(f"build/user/{program}","rb") as rf:
			kfs2.set_file_content(data_fs,kfs2.get_inode(data_fs,f"/bin/{program}",0o755),rf.read())
	with open(MODULE_ORDER_FILE_PATH,"rb") as rf:
		kfs2.set_file_content(data_fs,kfs2.get_inode(data_fs,"/boot/module/module_order.config",0o000),rf.read())
	for module in os.listdir("build/module"):
		with open(f"build/module/{module}","rb") as rf:
			kfs2.set_file_content(data_fs,kfs2.get_inode(data_fs,f"/boot/module/{module}",0o000),rf.read())
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
	if (b"br0" not in subprocess.run(["brctl","show"],stdout=subprocess.PIPE).stdout):
		if (subprocess.run(["sudo","bash","./src/_build/create_network_bridge.sh"]).returncode!=0):
			sys.exit(1)
	subprocess.run([
		"qemu-system-x86_64",
		# "-d","trace:usb*",
		# "-d","trace:nvme*,trace:pci_nvme*",
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
		"-netdev","bridge,br=br0,id=network",
		"-device","e1000,netdev=network",
		"-object","filter-dump,id=network-filter,netdev=network,file=build/network.dat",
		# Memory
		"-m","2G,slots=2,maxmem=4G",
		"-object","memory-backend-ram,size=1G,id=mem0",
		"-object","memory-backend-ram,size=1G,id=mem1",
		# CPU
		"-cpu","Skylake-Client-v1,tsc,invtsc,avx,avx2,bmi1,bmi2,pdpe1gb",
		"-smp","4,sockets=2,cores=1,threads=2,maxcpus=4",
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
