import array
import binascii
import hashlib
import initramfs
import kernel_linker
import os
import process_pool
import signature
import struct
import subprocess
import sys
import test
import time



BYPASS_KVM_LOCK=True
NO_DISPLAY=True
NO_FILE_SERVER=False
COMPRESSION_LEVEL="fast"



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
	"build/initramfs",
	"build/initramfs/boot",
	"build/initramfs/boot/module",
	"build/initramfs/etc",
	"build/kernel",
	"build/keys",
	"build/lib",
	"build/module",
	"build/objects",
	"build/objects/kernel",
	"build/objects/kernel_coverage",
	"build/objects/kernel_debug",
	"build/objects/lib",
	"build/objects/lib_coverage",
	"build/objects/lib_debug",
	"build/objects/module",
	"build/objects/module_coverage",
	"build/objects/module_debug",
	"build/objects/tool",
	"build/objects/tool_debug",
	"build/objects/uefi",
	"build/objects/user",
	"build/objects/user_coverage",
	"build/objects/user_debug",
	"build/partitions",
	"build/share",
	"build/share/bin",
	"build/share/lib",
	"build/share/test",
	"build/tool",
	"build/uefi",
	"build/user",
	"build/vm",
	"build/vm/tpm",
	"src/kernel/_generated"
]
CLEAR_BUILD_DIRECTORIES=[
	"build/share/bin",
	"build/share/lib"
]
UEFI_HASH_FILE_PATH="build/hashes/uefi.release.txt"
UEFI_FILE_DIRECTORY="src/uefi/"
UEFI_OBJECT_FILE_DIRECTORY="build/objects/uefi/"
KERNEL_HASH_FILE_PATH={
	MODE_NORMAL: "build/hashes/kernel.debug.txt",
	MODE_COVERAGE: "build/hashes/kernel.coverage.txt",
	MODE_RELEASE: "build/hashes/kernel.release.txt"
}[mode]
KERNEL_OBJECT_FILE_DIRECTORY={
	MODE_NORMAL: "build/objects/kernel_debug/",
	MODE_COVERAGE: "build/objects/kernel_coverage/",
	MODE_RELEASE: "build/objects/kernel/"
}[mode]
KERNEL_EXTRA_COMPILER_OPTIONS={
	MODE_NORMAL: ["-ggdb","-O1","-DKERNEL_DEBUG=1"],
	MODE_COVERAGE: ["-ggdb","--coverage","-fprofile-arcs","-ftest-coverage","-fprofile-info-section","-fprofile-update=atomic","-O1","-DKERNEL_COVERAGE=1"],
	MODE_RELEASE: ["-O3","-g0","-DKERNEL_RELEASE=1"]
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
	MODE_NORMAL: "build/objects/module_debug/",
	MODE_COVERAGE: "build/objects/module_coverage/",
	MODE_RELEASE: "build/objects/module/"
}[mode]
MODULE_EXTRA_COMPILER_OPTIONS={
	MODE_NORMAL: ["-ggdb","-O1","-DKERNEL_DEBUG=1"],
	MODE_COVERAGE: ["-ggdb","--coverage","-fprofile-arcs","-ftest-coverage","-fprofile-info-section","-fprofile-update=atomic","-O1","-DKERNEL_COVERAGE=1"],
	MODE_RELEASE: ["-O3","-g0","-DKERNEL_RELEASE=1"]
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
	MODE_COVERAGE: "build/objects/lib_coverage/",
	MODE_RELEASE: "build/objects/lib/"
}[mode]
LIBRARY_EXTRA_COMPILER_OPTIONS={
	MODE_NORMAL: ["-O1","-ggdb","-fno-omit-frame-pointer"],
	MODE_COVERAGE: ["-O1","-ggdb","-fno-omit-frame-pointer","--coverage","-fprofile-arcs","-ftest-coverage","-fprofile-info-section","-fprofile-update=atomic","-DKERNEL_COVERAGE=1"],
	MODE_RELEASE: ["-O3","-g0","-fdata-sections","-ffunction-sections","-fomit-frame-pointer"]
}[mode]
LIBRARY_EXTRA_ASSEMBLY_COMPILER_OPTIONS={
	MODE_NORMAL: ["-O1","-g"],
	MODE_COVERAGE: ["-O1","-g"],
	MODE_RELEASE: ["-O3"]
}[mode]
LIBRARY_EXTRA_LINKER_OPTIONS={
	MODE_NORMAL: ["-O1","-g"],
	MODE_COVERAGE: ["-O1","-g"],
	MODE_RELEASE: ["-O3","--gc-sections","-s"]
}[mode]
USER_HASH_FILE_SUFFIX={
	MODE_NORMAL: ".txt",
	MODE_COVERAGE: ".txt",
	MODE_RELEASE: ".release.txt"
}[mode]
USER_OBJECT_FILE_DIRECTORY={
	MODE_NORMAL: "build/objects/user_debug/",
	MODE_COVERAGE: "build/objects/user_coverage/",
	MODE_RELEASE: "build/objects/user/"
}[mode]
USER_EXTRA_COMPILER_OPTIONS={
	MODE_NORMAL: ["-O0","-ggdb","-fno-omit-frame-pointer"],
	MODE_COVERAGE: ["-O0","-ggdb","-fno-omit-frame-pointer","--coverage","-fprofile-arcs","-ftest-coverage","-fprofile-info-section","-fprofile-update=atomic","-DKERNEL_COVERAGE=1"],
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
TOOL_HASH_FILE_SUFFIX={
	MODE_NORMAL: ".txt",
	MODE_COVERAGE: ".txt",
	MODE_RELEASE: ".release.txt"
}[mode]
TOOL_OBJECT_FILE_DIRECTORY={
	MODE_NORMAL: "build/objects/tool_debug/",
	MODE_COVERAGE: "build/objects/tool_debug/",
	MODE_RELEASE: "build/objects/tool/"
}[mode]
TOOL_EXTRA_COMPILER_OPTIONS={
	MODE_NORMAL: ["-O0","-ggdb","-fno-omit-frame-pointer"],
	MODE_COVERAGE: ["-O0","-ggdb","-fno-omit-frame-pointer"],
	MODE_RELEASE: ["-O3","-g0","-fdata-sections","-ffunction-sections","-fomit-frame-pointer"]
}[mode]
TOOL_EXTRA_ASSEMBLY_COMPILER_OPTIONS={
	MODE_NORMAL: ["-O0","-g"],
	MODE_COVERAGE: ["-O0","-g"],
	MODE_RELEASE: ["-O3"]
}[mode]
TOOL_EXTRA_LINKER_OPTIONS={
	MODE_NORMAL: ["-O0","-g"],
	MODE_COVERAGE: ["-O0","-g"],
	MODE_RELEASE: ["-O3","-Wl,--gc-sections","-Wl,-s"]
}[mode]
SOURCE_FILE_SUFFIXES=[".asm",".c"]
KERNEL_FILE_DIRECTORY="src/kernel"
KERNEL_SYMBOL_FILE_PATH="build/kernel_symbols.c"
COMMON_FILE_DIRECTORY="src/common"
MODULE_FILE_DIRECTORY="src/module"
LIBRARY_FILE_DIRECTORY="src/lib"
USER_FILE_DIRECTORY="src/user"
TOOL_FILE_DIRECTORY="src/tool"
MODULE_ORDER_FILE_PATH="src/module/module_order.config"
FS_LIST_FILE_PATH="src/module/fs_list.config"
INSTALL_DISK_SIZE=262144
INSTALL_DISK_BLOCK_SIZE=512
INITRAMFS_SIZE=512
COVERAGE_FILE_REPORT_MARKER=0xb8bcbbbe41444347
COVERAGE_FILE_FAILURE_MARKER=0xb9beb6b34c494146
KERNEL_SYMBOL_VISIBILITY=("hidden" if mode!=MODE_COVERAGE else "default")



def _clear_if_obsolete(path,src_file_path,prefix):
	for file in os.listdir(path):
		if (not file.startswith(prefix) or os.path.exists(os.path.join(src_file_path,file[len(prefix):].split(".")[0]))):
			continue
		os.remove(os.path.join(path,file))



def _generate_syscalls(table_name,table_index,src_file_path,kernel_file_path,user_header_file_path):
	syscalls={}
	with open(src_file_path,"r") as rf:
		for line in rf.read().split("\n"):
			line=line.strip()
			if (not line):
				continue
			index,line=int(line[:line.index(",")]),line[line.index(",")+1:]
			name=line.split("(")[0].strip()
			args=tuple(line.split("(")[1].split(")")[0].strip().split(","))
			if (args==("void",)):
				args=tuple()
			ret=line.split("->")[1].strip()
			syscalls[index]=(name,args,ret)
	if (user_header_file_path is not None):
		with open(user_header_file_path,"w") as wf:
			wf.write(f"#ifndef _SYS_SYSCALL_KERNEL_SYSCALLS_H_\n#define _SYS_SYSCALL_KERNEL_SYSCALLS_H_ 1\n#include <sys/syscall/syscall.h>\n#include <sys/types.h>\n\n\n\n")
			for index,(name,args,ret) in syscalls.items():
				wf.write(f"static inline {ret} _sys_syscall_{name}({','.join(args) if args else 'void'}){{\n\t{('return ' if ret!='void' else '')}{('(void*)' if '*' in ret else '')}_sys_syscall{len(args)}({hex(index|(table_index<<32))}{''.join([','+('(u64)' if '*' in arg else '')+arg.split(' ')[-1] for arg in args])});\n}}\n\n\n\n")
			wf.write("#endif\n")
	with open(kernel_file_path,"w") as wf:
		wf.write("#include <kernel/types.h>\n\n\n\n")
		for name,_,_ in syscalls.values():
			wf.write(f"extern u64 syscall_{name}();\n")
		size=max(syscalls.keys())+1
		wf.write(f"\n\n\nconst u64 _syscalls_{table_name}_count={size};\n\n\n\nconst void*const _syscalls_{table_name}_functions[{size}]={{\n")
		for index,(name,_,_) in syscalls.items():
			wf.write(f"\t[{index}]=syscall_{name},\n")
		wf.write("};\n")



def _generate_kernel_build_info():
	with open("src/kernel/_generated/build_info.c","w") as wf:
		wf.write(f"#include <kernel/types.h>\n\n\n\nKERNEL_PUBLIC const u64 _version=0x{time.time_ns():016x};\nKERNEL_PUBLIC const char* _build_name=\"x86_64 { {MODE_NORMAL:'debug',MODE_COVERAGE:'coverage',MODE_RELEASE:'release'}[mode]}\";\n")



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



def _get_files(directories):
	for directory in directories:
		for root,_,files in os.walk(directory):
			for file_name in files:
				if (file_name[file_name.rindex("."):] not in SOURCE_FILE_SUFFIXES):
					continue
				yield os.path.join(root,file_name)



def _compile_uefi():
	changed_files,file_hash_list=_load_changed_files(UEFI_HASH_FILE_PATH,UEFI_FILE_DIRECTORY)
	object_files=[]
	pool=process_pool.ProcessPool(file_hash_list)
	rebuild_uefi_partition=False
	for file in _get_files([UEFI_FILE_DIRECTORY]):
		object_file=UEFI_OBJECT_FILE_DIRECTORY+file.replace("/","#")+".o"
		object_files.append(object_file)
		if (_file_not_changed(changed_files,object_file+".deps")):
			pool.dispatch(object_file)
			continue
		rebuild_uefi_partition=True
		command=None
		if (file.endswith(".c")):
			command=["gcc-12","-I/usr/include/efi","-I/usr/include/efi/x86_64","-fno-stack-protector","-ffreestanding","-O3","-fpic","-fshort-wchar","-mno-red-zone","-maccumulate-outgoing-args","-fdiagnostics-color=always","-DGNU_EFI_USE_MS_ABI","-Dx86_64","-m64","-Wall","-Werror","-Wno-trigraphs","-DNULL=((void*)0)","-o",object_file,"-c",file,f"-I{UEFI_FILE_DIRECTORY}/include"]
		else:
			command=["nasm","-f","elf64","-O3","-Wall","-Werror","-o",object_file,file]
		pool.add([],object_file,"C "+file,command+["-MD","-MT",object_file,"-MF",object_file+".deps"])
	if (not rebuild_uefi_partition):
		return False
	pool.add(object_files,"build/uefi/loader.efi","L build/uefi/loader.efi",["ld","-nostdlib","-znocombreloc","-znoexecstack","-fshort-wchar","-T","/usr/lib/elf_x86_64_efi.lds","-shared","-Bsymbolic","-o","build/uefi/loader.efi","/usr/lib/gcc/x86_64-linux-gnu/12/libgcc.a"]+object_files)
	pool.add(["build/uefi/loader.efi"],"build/uefi/loader.efi","P build/uefi/loader.efi",["objcopy","-j",".text","-j",".sdata","-j",".data","-j",".dynamic","-j",".dynsym","-j",".rel","-j",".rela","-j",".reloc","-S","--target=efi-app-x86_64","build/uefi/loader.efi","build/uefi/loader.efi"])
	error=pool.wait()
	_save_file_hash_list(file_hash_list,UEFI_HASH_FILE_PATH)
	if (error):
		sys.exit(1)
	return True



def _compile_kernel(force_patch_kernel):
	changed_files,file_hash_list=_load_changed_files(KERNEL_HASH_FILE_PATH,KERNEL_FILE_DIRECTORY)
	if ("src/kernel/_generated/build_info.c" in changed_files):
		changed_files.remove("src/kernel/_generated/build_info.c")
	if (changed_files):
		_generate_kernel_build_info()
		changed_files.append("src/kernel/_generated/build_info.c")
	object_files=[]
	pool=process_pool.ProcessPool(file_hash_list)
	for file in _get_files([KERNEL_FILE_DIRECTORY]):
		object_file=KERNEL_OBJECT_FILE_DIRECTORY+file.replace("/","#")+".o"
		object_files.append(object_file)
		if (_file_not_changed(changed_files,object_file+".deps")):
			pool.dispatch(object_file)
			continue
		command=None
		if (file.endswith(".c")):
			command=["gcc-12","-mcmodel=kernel","-mno-red-zone","-mno-mmx","-mno-sse","-mno-sse2","-mbmi","-mbmi2","-fno-lto","-fplt","-fno-pie","-fno-pic","-fno-common","-fno-builtin","-fno-stack-protector","-fno-asynchronous-unwind-tables","-nostdinc","-nostdlib","-ffreestanding",f"-fvisibility={KERNEL_SYMBOL_VISIBILITY}","-m64","-Wall","-Werror","-Wno-trigraphs","-Wno-address-of-packed-member","-c","-fdiagnostics-color=always","-ftree-loop-distribute-patterns","-O3","-g0","-fno-omit-frame-pointer","-DNULL=((void*)0)","-DBUILD_KERNEL=1","-o",object_file,file,f"-I{KERNEL_FILE_DIRECTORY}/include"]+KERNEL_EXTRA_COMPILER_OPTIONS
		else:
			command=["nasm","-f","elf64","-O3","-Wall","-Werror","-o",object_file,file]
		if (os.path.exists(object_file+".gcno")):
			os.remove(os.path.exists(object_file+".gcno"))
		pool.add([],object_file,"C "+file,command+["-MD","-MT",object_file,"-MF",object_file+".deps"])
	if (not changed_files and not force_patch_kernel):
		return False
	pool.add(object_files,"build/kernel/kernel.elf","L build/kernel/kernel.elf",["ld","-znoexecstack","-melf_x86_64","-Bsymbolic","-r","-o","build/kernel/kernel.elf","-O3","-T","src/kernel/linker.ld"]+KERNEL_EXTRA_LINKER_OPTIONS+object_files)
	pool.add(["build/kernel/kernel.elf"],"build/kernel/kernel.elf","P build/kernel/kernel.elf",[kernel_linker.link_kernel,"build/kernel/kernel.elf","build/kernel/kernel.bin"])
	error=pool.wait()
	_save_file_hash_list(file_hash_list,KERNEL_HASH_FILE_PATH)
	if (error):
		sys.exit(1)
	return True



def _compile_module(module,dependencies,changed_files,pool):
	if (mode!=MODE_COVERAGE and module.startswith("test")):
		return False
	object_files=[]
	included_directories=[f"-I{MODULE_FILE_DIRECTORY}/{module}/include",f"-I{KERNEL_FILE_DIRECTORY}/include"]+[f"-I{(MODULE_FILE_DIRECTORY if 'common/' not in dep else COMMON_FILE_DIRECTORY)}/{dep.split('/')[-1]}/include" for dep in dependencies]
	has_updates=False
	for file in _get_files([MODULE_FILE_DIRECTORY+"/"+module]+[COMMON_FILE_DIRECTORY+"/"+dep.split("/")[-1] for dep in dependencies if "common/" in dep]):
		object_file=MODULE_OBJECT_FILE_DIRECTORY+file.replace("/","#")+".o"
		object_files.append(object_file)
		if (_file_not_changed(changed_files,object_file+".deps")):
			pool.dispatch(object_file)
			continue
		command=None
		if (file.endswith(".c")):
			command=["gcc-12","-mcmodel=kernel","-mno-red-zone","-mno-mmx","-mno-sse","-mno-sse2","-mbmi","-mbmi2","-fno-common","-fno-builtin","-nostdlib","-fno-omit-frame-pointer","-fno-asynchronous-unwind-tables","-ffreestanding",f"-fvisibility={KERNEL_SYMBOL_VISIBILITY}","-fplt","-fno-pie","-fno-pic","-m64","-Wall","-Werror","-Wno-trigraphs","-Wno-address-of-packed-member","-c","-o",object_file,"-fdiagnostics-color=always",file,"-DNULL=((void*)0)","-DBUILD_MODULE=1"]+included_directories+MODULE_EXTRA_COMPILER_OPTIONS
		else:
			command=["nasm","-f","elf64","-Wall","-Werror","-O3","-o",object_file,file]
		if (os.path.exists(object_file+".gcno")):
			os.remove(os.path.exists(object_file+".gcno"))
		pool.add([],object_file,"C "+file,command+["-MD","-MT",object_file,"-MF",object_file+".deps"])
		has_updates=True
	if (os.path.exists(f"build/module/{module}.mod") and not has_updates):
		return False
	pool.add(object_files,f"build/module/{module}.mod",f"L build/module/{module}.mod",["ld","-znoexecstack","-melf_x86_64","-Bsymbolic","-r","-T","src/module/linker.ld","-o",f"build/module/{module}.mod"]+object_files+MODULE_EXTRA_LINKER_OPTIONS)
	pool.add([f"build/module/{module}.mod"],f"build/module/{module}.mod",f"P build/module/{module}.mod",[kernel_linker.link_module_or_library,f"build/module/{module}.mod","module"])
	return True



def _compile_all_modules():
	hash_file_path=f"build/hashes/module"+MODULE_HASH_FILE_SUFFIX
	changed_files,file_hash_list=_load_changed_files(hash_file_path,MODULE_FILE_DIRECTORY,COMMON_FILE_DIRECTORY,KERNEL_FILE_DIRECTORY+"/include")
	pool=process_pool.ProcessPool(file_hash_list)
	out=False
	with open("src/module/dependencies.txt","r") as rf:
		for line in rf.read().split("\n"):
			line=line.strip()
			if (not line):
				continue
			name,dependencies=line.split(":")
			out|=_compile_module(name,[dep.strip() for dep in dependencies.split(",") if dep.strip()],changed_files,pool)
	error=pool.wait()
	_save_file_hash_list(file_hash_list,hash_file_path)
	if (error):
		sys.exit(1)
	return out



def _compile_library(library,flags,dependencies,changed_files,pool):
	if (mode!=MODE_COVERAGE and library.startswith("test")):
		return False
	for root,_,files in os.walk(f"{LIBRARY_FILE_DIRECTORY}/{library}/rsrc"):
		for file_name in files:
			if (not os.path.exists(f"{LIBRARY_FILE_DIRECTORY}/{library}/_generated")):
				os.mkdir(f"{LIBRARY_FILE_DIRECTORY}/{library}/_generated")
				os.mkdir(f"{LIBRARY_FILE_DIRECTORY}/{library}/_generated/include")
			name=os.path.join(root,file_name).replace("/","_").replace(".","_")
			with open(os.path.join(root,file_name),"rb") as rf,open(f"{LIBRARY_FILE_DIRECTORY}/{library}/_generated/include/{name}.h","w") as wf:
				wf.write(f"#include <sys/types.h>\n\n\n\nstatic const u8 {name}[]={{")
				size=0
				while (True):
					line=rf.read(16)
					if (not line):
						break
					size+=len(line)
					wf.write("\n\t"+"".join([f"0x{e:02x}," for e in line]))
				wf.write(f"\n\t0x00,\n}};\n\n\n\nstatic const u32 {name}_length={size};\n")
	object_files=[]
	included_directories=[f"-I{LIBRARY_FILE_DIRECTORY}/{library}/include",f"-I{LIBRARY_FILE_DIRECTORY}/{library}/_generated/include"]+[f"-I{LIBRARY_FILE_DIRECTORY}/{dep.split('@')[0]}/include" for dep in dependencies]
	has_updates=False
	for file in _get_files([LIBRARY_FILE_DIRECTORY+"/"+library]):
		object_file=LIBRARY_OBJECT_FILE_DIRECTORY+file.replace("/","#")+".o"
		object_files.append(object_file)
		if (_file_not_changed(changed_files,object_file+".deps")):
			pool.dispatch(object_file)
			continue
		command=None
		if (file.endswith(".c")):
			command=["gcc-12","-fno-common","-fno-builtin","-nostdlib","-fvisibility=hidden","-ffreestanding","-shared","-fpic","-m64","-Wall","-Werror","-Wno-trigraphs","-c","-o",object_file,"-fdiagnostics-color=always",file,"-DNULL=((void*)0)"]+included_directories+LIBRARY_EXTRA_COMPILER_OPTIONS
		else:
			command=["nasm","-f","elf64","-Wall","-Werror","-DBUILD_SHARED=1","-O3","-o",object_file,file]+included_directories+LIBRARY_EXTRA_ASSEMBLY_COMPILER_OPTIONS
		if (os.path.exists(object_file+".gcno")):
			os.remove(os.path.exists(object_file+".gcno"))
		pool.add([],object_file,"C "+file,command+["-MD","-MT",object_file,"-MF",object_file+".deps"])
		has_updates=True
	if ("nodynamic" not in flags):
		if (has_updates or not os.path.exists(f"build/lib/lib{library}.so")):
			pool.add(object_files+[f"build/lib/lib{dep.split('@')[0]}.{('a' if '@static' in dep else 'so')}" for dep in dependencies],f"build/lib/lib{library}.so",f"L build/lib/lib{library}.so",["ld","-znoexecstack","-melf_x86_64","-T","src/lib/linker.ld","--exclude-libs","ALL","-shared","-o",f"build/lib/lib{library}.so"]+object_files+[(f"build/lib/lib{dep.split('@')[0]}.a" if "@static" in dep else f"-l{dep.split('@')[0]}") for dep in dependencies]+LIBRARY_EXTRA_LINKER_OPTIONS)
			pool.add([f"build/lib/lib{library}.so"],f"build/lib/lib{library}.so",f"P build/lib/lib{library}.so",[kernel_linker.link_module_or_library,f"build/lib/lib{library}.so","user"])
		else:
			pool.dispatch(f"build/lib/lib{library}.so")
	if ("nostatic" not in flags):
		if (has_updates or not os.path.exists(f"build/lib/lib{library}.a")):
			if (os.path.exists(f"build/lib/lib{library}.a")):
				os.remove(f"build/lib/lib{library}.a")
			pool.add(object_files,f"build/lib/lib{library}.a",f"L build/lib/lib{library}.a",["ar","rcs",f"build/lib/lib{library}.a"]+object_files)
		else:
			pool.dispatch(f"build/lib/lib{library}.a")
	return has_updates



def _compile_all_libraries():
	hash_file_path=f"build/hashes/lib"+MODULE_HASH_FILE_SUFFIX
	changed_files,file_hash_list=_load_changed_files(hash_file_path,LIBRARY_FILE_DIRECTORY)
	pool=process_pool.ProcessPool(file_hash_list)
	out=False
	with open("src/lib/dependencies.txt","r") as rf:
		for line in rf.read().split("\n"):
			line=line.strip()
			if (not line):
				continue
			name,dependencies=line.split(":")
			flags=([] if "@" not in name else [flag.strip() for flag in name.split("@")[1].split(",") if flag.strip()])
			out|=_compile_library(name.split("@")[0],flags,[dep.strip() for dep in dependencies.split(",") if dep.strip()],changed_files,pool)
	error=pool.wait()
	_save_file_hash_list(file_hash_list,hash_file_path)
	if (error):
		sys.exit(1)
	return out



def _compile_user_program(program,dependencies,changed_files,pool):
	if (mode!=MODE_COVERAGE and program.startswith("test")):
		return False
	dependencies.append(["runtime","static"])
	object_files=[]
	included_directories=[f"-I{USER_FILE_DIRECTORY}/{program}/include"]+[f"-I{LIBRARY_FILE_DIRECTORY}/{dep[0]}/include" for dep in dependencies]
	has_updates=False
	for file in _get_files([USER_FILE_DIRECTORY+"/"+program]):
		object_file=USER_OBJECT_FILE_DIRECTORY+file.replace("/","#")+".o"
		object_files.append(object_file)
		if (_file_not_changed(changed_files,object_file+".deps")):
			pool.dispatch(object_file)
			continue
		command=None
		if (file.endswith(".c")):
			command=["gcc-12","-fno-common","-fno-builtin","-nostdlib","-ffreestanding","-fno-pie","-fno-pic","-m64","-Wall","-Werror","-Wno-trigraphs","-c","-o",object_file,"-fdiagnostics-color=always",file,"-DNULL=((void*)0)"]+included_directories+USER_EXTRA_COMPILER_OPTIONS
		else:
			command=["nasm","-f","elf64","-Wall","-Werror","-O3","-o",object_file,file]+USER_EXTRA_ASSEMBLY_COMPILER_OPTIONS
		if (os.path.exists(object_file+".gcno")):
			os.remove(os.path.exists(object_file+".gcno"))
		pool.add([],object_file,"C "+file,command+["-MD","-MT",object_file,"-MF",object_file+".deps"])
		has_updates=True
	if (os.path.exists(f"build/user/{program}") and not has_updates):
		return False
	pool.add(object_files,f"build/user/{program}",f"L build/user/{program}",["ld","-znoexecstack","-melf_x86_64","-I/lib/ld.so","-T","src/user/linker.ld","--exclude-libs","ALL","-o",f"build/user/{program}"]+[(f"-l{dep[0]}" if len(dep)==1 or dep[1]!="static" else f"build/lib/lib{dep[0]}.a") for dep in dependencies]+object_files+USER_EXTRA_LINKER_OPTIONS)
	pool.add([f"build/user/{program}"],f"build/user/{program}",f"P build/user/{program}",[kernel_linker.link_module_or_library,f"build/user/{program}","user"])
	return True



def _compile_all_user_programs():
	hash_file_path=f"build/hashes/user"+USER_HASH_FILE_SUFFIX
	changed_files,file_hash_list=_load_changed_files(hash_file_path,USER_FILE_DIRECTORY,LIBRARY_FILE_DIRECTORY)
	pool=process_pool.ProcessPool(file_hash_list)
	out=False
	with open("src/user/dependencies.txt","r") as rf:
		for line in rf.read().split("\n"):
			line=line.strip()
			if (not line):
				continue
			name,dependencies=line.split(":")
			out|=_compile_user_program(name,[dep.strip().split("@") for dep in dependencies.split(",") if dep.strip()],changed_files,pool)
	error=pool.wait()
	_save_file_hash_list(file_hash_list,hash_file_path)
	if (error):
		sys.exit(1)
	return out



def _compile_tool(tool,dependencies,changed_files,pool):
	object_files=[]
	included_directories=[f"-I{TOOL_FILE_DIRECTORY}/{tool}/include"]+[f"-I{COMMON_FILE_DIRECTORY}/{dep}/include" for dep in dependencies]
	has_updates=False
	for file in _get_files([TOOL_FILE_DIRECTORY+"/"+tool]+[COMMON_FILE_DIRECTORY+"/"+dep for dep in dependencies]):
		object_file=TOOL_OBJECT_FILE_DIRECTORY+file.replace("/","#")+".o"
		object_files.append(object_file)
		if (_file_not_changed(changed_files,object_file+".deps")):
			pool.dispatch(object_file)
			continue
		command=None
		if (file.endswith(".c")):
			command=["gcc-12","-m64","-Wall","-Werror","-Wno-trigraphs","-c","-o",object_file,"-fdiagnostics-color=always",file,"-DNULL=((void*)0)","-Wno-address-of-packed-member"]+included_directories+TOOL_EXTRA_COMPILER_OPTIONS
		else:
			command=["nasm","-f","elf64","-Wall","-Werror","-O3","-o",object_file,file]+TOOL_EXTRA_ASSEMBLY_COMPILER_OPTIONS
		pool.add([],object_file,"C "+file,command+["-MD","-MT",object_file,"-MF",object_file+".deps"])
		has_updates=True
	if (os.path.exists(f"build/tool/{tool}") and not has_updates):
		return False
	pool.add(object_files,f"build/tool/{tool}",f"L build/tool/{tool}",["gcc-12","-Wl,-znoexecstack","-o",f"build/tool/{tool}"]+object_files+TOOL_EXTRA_LINKER_OPTIONS)



def _compile_all_tools():
	hash_file_path=f"build/hashes/tool"+TOOL_HASH_FILE_SUFFIX
	changed_files,file_hash_list=_load_changed_files(hash_file_path,TOOL_FILE_DIRECTORY,COMMON_FILE_DIRECTORY)
	pool=process_pool.ProcessPool(file_hash_list)
	out=False
	with open("src/tool/dependencies.txt","r") as rf:
		for line in rf.read().split("\n"):
			line=line.strip()
			if (not line):
				continue
			name,dependencies=line.split(":")
			_compile_tool(name,[dep for dep in dependencies.split(",") if dep.strip()],changed_files,pool)
	error=pool.wait()
	_save_file_hash_list(file_hash_list,hash_file_path)
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



def _get_early_modules(file_path):
	out=["module_loader"]
	with open(file_path,"r") as rf:
		for module in rf.read().split("\n"):
			module=module.strip()
			if (module and module[0]!="#" and module.count("=")==1):
				name,type_=module.split("=")
				if (type_=="early"):
					out.append(name)
	return out



def _execute_compressor_command(file_path):
	if (subprocess.run(["build/tool/compressor",file_path,COMPRESSION_LEVEL,file_path+".compressed"]).returncode!=0):
		sys.exit(1)



def _execute_kfs2_command(command):
	if (subprocess.run(["build/tool/kfs2","build/install_disk.img",f"{INSTALL_DISK_BLOCK_SIZE}:93720:{INSTALL_DISK_SIZE-34}"]+command).returncode!=0):
		sys.exit(1)



def _generate_install_disk(rebuild_uefi_partition,rebuild_data_partition):
	if (not os.path.exists("build/install_disk.img")):
		rebuild_uefi_partition=True
		rebuild_data_partition=True
		subprocess.run(["dd","if=/dev/zero","of=build/install_disk.img",f"bs={INSTALL_DISK_BLOCK_SIZE}",f"count={INSTALL_DISK_SIZE}"])
		subprocess.run(["parted","build/install_disk.img","-s","-a","minimal","mklabel","gpt"])
		subprocess.run(["parted","build/install_disk.img","-s","-a","minimal","mkpart","EFI","FAT16","34s","93719s"])
		subprocess.run(["parted","build/install_disk.img","-s","-a","minimal","mkpart","DATA","93720s",f"{INSTALL_DISK_SIZE-34}s"])
		subprocess.run(["parted","build/install_disk.img","-s","-a","minimal","toggle","1","boot"])
		_execute_kfs2_command(["format"])
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
		_copy_file(FS_LIST_FILE_PATH,"build/initramfs/etc/fs_list.config")
		initramfs.create("build/initramfs","build/partitions/initramfs.img")
		_execute_compressor_command("build/kernel/kernel.bin")
		_execute_compressor_command("build/partitions/initramfs.img")
		_execute_kfs2_command(["mkdir","/bin","0755"])
		_execute_kfs2_command(["mkdir","/boot","0500"])
		_execute_kfs2_command(["mkdir","/boot/module","0700"])
		_execute_kfs2_command(["mkdir","/etc","0755"])
		_execute_kfs2_command(["mkdir","/lib","0755"])
		_execute_kfs2_command(["copy","/boot/kernel.compressed","0400","build/kernel/kernel.bin.compressed"])
		_execute_kfs2_command(["copy","/boot/initramfs.compressed","0400","build/partitions/initramfs.img.compressed"])
		_execute_kfs2_command(["setboot","kernel","/boot/kernel.compressed"])
		_execute_kfs2_command(["setboot","initramfs","/boot/initramfs.compressed"])
		_execute_kfs2_command(["copy","/boot/module/module_order.config","0600",MODULE_ORDER_FILE_PATH])
		_execute_kfs2_command(["copy","/etc/fs_list.config","0600",FS_LIST_FILE_PATH])
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
		if ("tsc" in rf.read() and not BYPASS_KVM_LOCK):
			print("\x1b[1m\x1b[38;2;231;72;86mKVM support disabled due to kernel TSC clock source\x1b[0m")
			return []
	return ["-accel","kvm"]



def _generate_coverage_report(vm_output_file_path,output_file_path):
	for file in os.listdir(KERNEL_OBJECT_FILE_DIRECTORY):
		if (file.endswith(".gcda")):
			os.remove(os.path.join(KERNEL_OBJECT_FILE_DIRECTORY,file))
	for file in os.listdir(MODULE_OBJECT_FILE_DIRECTORY):
		if (file.endswith(".gcda")):
			os.remove(os.path.join(MODULE_OBJECT_FILE_DIRECTORY,file))
	for file in os.listdir(LIBRARY_OBJECT_FILE_DIRECTORY):
		if (file.endswith(".gcda")):
			os.remove(os.path.join(LIBRARY_OBJECT_FILE_DIRECTORY,file))
	for file in os.listdir(USER_OBJECT_FILE_DIRECTORY):
		if (file.endswith(".gcda")):
			os.remove(os.path.join(USER_OBJECT_FILE_DIRECTORY,file))
	file_list=set()
	with open(vm_output_file_path,"rb") as rf:
		while (True):
			buffer=rf.read(8)
			if (len(buffer)<8):
				break
			marker=struct.unpack("<Q",buffer)[0]
			if (marker==COVERAGE_FILE_FAILURE_MARKER):
				sys.exit(1)
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
	if (not NO_FILE_SERVER):
		subprocess.Popen((["/usr/libexec/virtiofsd",f"--socket-group={os.getlogin()}"] if not os.getenv("GITHUB_ACTIONS","") else ["sudo","build/external/virtiofsd"])+["--socket-path=build/vm/virtiofsd.sock","--shared-dir","build/share","--inode-file-handles=mandatory"])
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
	subprocess.run(([] if not os.getenv("GITHUB_ACTIONS","") else ["sudo"])+[
		"qemu-system-x86_64",
		# "-d","trace:virtio*,trace:virtio_blk*",
		# "-d","trace:virtio*,trace:virtio_gpu*",
		# "-d","trace:virtio*,trace:vhost*,trace:virtqueue*",
		# "-d","trace:usb*",
		# "-d","trace:nvme*,trace:pci_nvme*",
		# "-d","trace:tpm*",
		# "-d","int,cpu_reset",
		# "--no-reboot",
		# "-d","guest_errors",
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
		"-device","e1000,netdev=network", ### 'Real' network card
		# "-device","virtio-net,netdev=network",
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
		*(["-display","none"] if NO_DISPLAY or os.getenv("GITHUB_ACTIONS","") else ["-device","virtio-vga-gl,xres=1280,yres=960","-display","sdl,gl=on"]),
		# Shared directory
		*(["-chardev","socket,id=virtio-fs-sock,path=build/vm/virtiofsd.sock","-device","vhost-user-fs-pci,queue-size=1024,chardev=virtio-fs-sock,tag=build-fs"] if not NO_FILE_SERVER else []),
		# Serial
		"-chardev","stdio,id=console,mux=on",
		"-mon","chardev=console,mode=readline",
		"-serial","chardev:console",
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
		*([] if mode!=MODE_NORMAL else ["-gdb","tcp::9000"])
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



if (mode==MODE_COVERAGE):
	NO_FILE_SERVER=False
	NO_DISPLAY=True
force_patch_kernel=False
if (os.path.exists("build/last_mode")):
	with open("build/last_mode","r") as rf:
		if (int(rf.read())!=mode):
			CLEAR_BUILD_DIRECTORIES.extend(["build/lib","build/module","build/tool","build/user"])
			force_patch_kernel=True
for dir_ in BUILD_DIRECTORIES:
	if (not os.path.exists(dir_)):
		os.mkdir(dir_)
for dir_ in CLEAR_BUILD_DIRECTORIES:
	for file in os.listdir(dir_):
		os.remove(os.path.join(dir_,file))
_clear_if_obsolete("build/lib","src/lib","lib")
_clear_if_obsolete("build/module","src/module","")
_clear_if_obsolete("build/user","src/user","")
with open("build/last_mode","w") as wf:
	wf.write(f"{mode}\n")
_generate_syscalls("kernel",1,"src/kernel/syscalls-kernel.txt","src/kernel/_generated/syscalls_kernel.c","src/lib/sys/include/sys/syscall/kernel_syscalls.h")
signature.load_key("module",mode==MODE_RELEASE)
signature.load_key("user",mode==MODE_RELEASE)
if (mode==MODE_COVERAGE):
	test.generate_test_resource_files()
rebuild_uefi_partition=_compile_uefi()
rebuild_data_partition=_compile_kernel(force_patch_kernel)
rebuild_data_partition|=_compile_all_modules()
rebuild_data_partition|=_compile_all_libraries()
rebuild_data_partition|=_compile_all_user_programs()
_compile_all_tools()
_generate_shared_directory()
if ("--share" in sys.argv):
	sys.exit(0)
_generate_install_disk(rebuild_uefi_partition,rebuild_data_partition)
if ("--run" not in sys.argv):
	sys.exit(0)
_execute_vm()
for root,_,files in os.walk("build/share",followlinks=False):
	for file in files:
		if (os.path.islink(os.path.join(root,file))):
			continue
		os.chmod(os.path.join(root,file),0o664)
