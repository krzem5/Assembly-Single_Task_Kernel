import os
import struct



ET_REL=1
ET_EXEC=2
ET_DYN=3

PT_INTERP=3



__all__=["generate_test_resource_files"]



def _generate_header(signature,word_size,endianess,header_version,abi,type,machine,version,phoff,phnum):
	return struct.pack("<IBBBB8xHHI8xQ14xHH6x",signature,word_size,endianess,header_version,abi,type,machine,version,phoff,56,phnum)



def _generate_interpreter_program_header(offset,size):
	return struct.pack("<I4xQ16xQ16x",PT_INTERP,offset,size)



def _generate_header_and_interpreter(interpreter):
	return _generate_header(0x464c457f,2,1,1,0,ET_EXEC,0x3e,1,64,1)+_generate_interpreter_program_header(64+56,len(interpreter)+1)+bytes(interpreter,"utf-8")+b"\x00"



def generate_test_resource_files():
	if (not os.path.exists("build/share/test/elf")):
		os.mkdir("build/share/test/elf")
	if (os.path.exists("build/share/test/elf/no_read_and_execute_access_file")):
		os.remove("build/share/test/elf/no_read_and_execute_access_file")
	with open("build/share/test/elf/no_read_and_execute_access_file","wb") as wf:
		wf.write(b"")
	os.chmod("build/share/test/elf/no_read_and_execute_access_file",0o000)
	if (os.path.exists("build/share/test/elf/no_read_access_file")):
		os.remove("build/share/test/elf/no_read_access_file")
	with open("build/share/test/elf/no_read_access_file","wb") as wf:
		wf.write(b"")
	os.chmod("build/share/test/elf/no_read_access_file",0o111)
	if (os.path.exists("build/share/test/elf/no_execute_access_file")):
		os.remove("build/share/test/elf/no_execute_access_file")
	with open("build/share/test/elf/no_execute_access_file","wb") as wf:
		wf.write(b"")
	os.chmod("build/share/test/elf/no_execute_access_file",0o444)
	with open("build/share/test/elf/invalid_header_signature","wb") as wf:
		wf.write(_generate_header(0xaabbccdd,2,1,1,0,ET_EXEC,0x3e,1,0,0))
	os.chmod("build/share/test/elf/invalid_header_signature",0o755)
	with open("build/share/test/elf/invalid_header_word_size","wb") as wf:
		wf.write(_generate_header(0x464c457f,4,1,1,0,ET_EXEC,0x3e,1,0,0))
	os.chmod("build/share/test/elf/invalid_header_word_size",0o755)
	with open("build/share/test/elf/invalid_header_endianess","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,2,1,0,ET_EXEC,0x3e,1,0,0))
	os.chmod("build/share/test/elf/invalid_header_endianess",0o755)
	with open("build/share/test/elf/invalid_header_header_version","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,1,2,0,ET_EXEC,0x3e,1,0,0))
	os.chmod("build/share/test/elf/invalid_header_header_version",0o755)
	with open("build/share/test/elf/invalid_header_abi","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,1,1,3,ET_EXEC,0x3e,1,0,0))
	os.chmod("build/share/test/elf/invalid_header_abi",0o755)
	with open("build/share/test/elf/invalid_header_type","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,1,1,0,ET_DYN,0x3e,1,0,0))
	os.chmod("build/share/test/elf/invalid_header_type",0o755)
	with open("build/share/test/elf/invalid_header_type_interpreter","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,1,1,0,ET_EXEC,0x3e,1,0,0))
	os.chmod("build/share/test/elf/invalid_header_type_interpreter",0o755)
	with open("build/share/test/elf/invalid_header_machine","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,1,1,0,ET_EXEC,0x3f,1,0,0))
	os.chmod("build/share/test/elf/invalid_header_machine",0o755)
	with open("build/share/test/elf/invalid_header_version","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,1,1,0,ET_EXEC,0x3e,2,0,0))
	os.chmod("build/share/test/elf/invalid_header_version",0o755)
	with open("build/share/test/elf/multiple_interpreters","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,1,1,0,ET_EXEC,0x3e,1,64,2))
		wf.write(_generate_interpreter_program_header(64+56*2,2))
		wf.write(_generate_interpreter_program_header(64+56*2,2))
		wf.write(b"A\x00")
	os.chmod("build/share/test/elf/multiple_interpreters",0o755)
	with open("build/share/test/elf/unterminated_interpreter","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,1,1,0,ET_EXEC,0x3e,1,64,1))
		wf.write(_generate_interpreter_program_header(64+56,1))
		wf.write(b"A")
	os.chmod("build/share/test/elf/unterminated_interpreter",0o755)
	with open("build/share/test/elf/interpreter_invalid_path","wb") as wf:
		wf.write(_generate_header_and_interpreter("/invalid/path"))
	os.chmod("build/share/test/elf/interpreter_invalid_path",0o755)
	with open("build/share/test/elf/interpreter_no_read_and_execute_permissions","wb") as wf:
		wf.write(_generate_header_and_interpreter("/share/test/elf/no_read_and_execute_access_file"))
	os.chmod("build/share/test/elf/interpreter_no_read_and_execute_permissions",0o755)
	with open("build/share/test/elf/interpreter_no_read_permissions","wb") as wf:
		wf.write(_generate_header_and_interpreter("/share/test/elf/no_read_access_file"))
	os.chmod("build/share/test/elf/interpreter_no_read_permissions",0o755)
	with open("build/share/test/elf/interpreter_no_execute_permissions","wb") as wf:
		wf.write(_generate_header_and_interpreter("/share/test/elf/no_execute_access_file"))
	os.chmod("build/share/test/elf/interpreter_no_execute_permissions",0o755)
	with open("build/share/test/elf/interpreter_invalid_header_signature","wb") as wf:
		wf.write(_generate_header_and_interpreter("/share/test/elf/invalid_header_signature"))
	os.chmod("build/share/test/elf/interpreter_invalid_header_signature",0o755)
	with open("build/share/test/elf/interpreter_invalid_header_word_size","wb") as wf:
		wf.write(_generate_header_and_interpreter("/share/test/elf/invalid_header_word_size"))
	os.chmod("build/share/test/elf/interpreter_invalid_header_word_size",0o755)
	with open("build/share/test/elf/interpreter_invalid_header_endianess","wb") as wf:
		wf.write(_generate_header_and_interpreter("/share/test/elf/invalid_header_endianess"))
	os.chmod("build/share/test/elf/interpreter_invalid_header_endianess",0o755)
	with open("build/share/test/elf/interpreter_invalid_header_header_version","wb") as wf:
		wf.write(_generate_header_and_interpreter("/share/test/elf/invalid_header_header_version"))
	os.chmod("build/share/test/elf/interpreter_invalid_header_header_version",0o755)
	with open("build/share/test/elf/interpreter_invalid_header_abi","wb") as wf:
		wf.write(_generate_header_and_interpreter("/share/test/elf/invalid_header_abi"))
	os.chmod("build/share/test/elf/interpreter_invalid_header_abi",0o755)
	with open("build/share/test/elf/interpreter_invalid_header_type","wb") as wf:
		wf.write(_generate_header_and_interpreter("/share/test/elf/invalid_header_type_interpreter"))
	os.chmod("build/share/test/elf/interpreter_invalid_header_type",0o755)
	with open("build/share/test/elf/interpreter_invalid_header_machine","wb") as wf:
		wf.write(_generate_header_and_interpreter("/share/test/elf/invalid_header_machine"))
	os.chmod("build/share/test/elf/interpreter_invalid_header_machine",0o755)
	with open("build/share/test/elf/interpreter_invalid_header_version","wb") as wf:
		wf.write(_generate_header_and_interpreter("/share/test/elf/invalid_header_version"))
	os.chmod("build/share/test/elf/interpreter_invalid_header_version",0o755)
	if (not os.path.exists("build/share/test/fd")):
		os.mkdir("build/share/test/fd")
	if (os.path.exists("build/share/test/fd/no_read_access_file")):
		os.remove("build/share/test/fd/no_read_access_file")
	with open("build/share/test/fd/no_read_access_file","wb") as wf:
		wf.write(b"")
	os.chmod("build/share/test/fd/no_read_access_file",0o222)
	if (os.path.exists("build/share/test/fd/no_write_access_file")):
		os.remove("build/share/test/fd/no_write_access_file")
	with open("build/share/test/fd/no_write_access_file","wb") as wf:
		wf.write(b"")
	os.chmod("build/share/test/fd/no_write_access_file",0o444)
	with open("build/share/test/fd/length_6_file","wb") as wf:
		wf.write(b"abcdef")
	if (not os.path.exists("build/share/test/fd/empty_directory")):
		os.mkdir("build/share/test/fd/empty_directory")
	if (not os.path.exists("build/share/test/fd/directory_with_abc_child")):
		os.mkdir("build/share/test/fd/directory_with_abc_child")
	if (not os.path.exists("build/share/test/fd/directory_with_abc_child")):
		os.mkdir("build/share/test/fd/directory_with_abc_child")
	with open("build/share/test/fd/directory_with_abc_child/abc","wb") as wf:
		wf.write(b"abcdef")
	if (not os.path.exists("build/share/test/module")):
		os.mkdir("build/share/test/module")
	with open("build/share/test/module/invalid_header_signature","wb") as wf:
		wf.write(_generate_header(0xaabbccdd,2,1,1,0,ET_REL,0x3e,1,0,0))
	with open("build/share/test/module/invalid_header_word_size","wb") as wf:
		wf.write(_generate_header(0x464c457f,4,1,1,0,ET_REL,0x3e,1,0,0))
	with open("build/share/test/module/invalid_header_endianess","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,2,1,0,ET_REL,0x3e,1,0,0))
	with open("build/share/test/module/invalid_header_header_version","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,1,2,0,ET_REL,0x3e,1,0,0))
	with open("build/share/test/module/invalid_header_abi","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,1,1,3,ET_REL,0x3e,1,0,0))
	with open("build/share/test/module/invalid_header_type","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,1,1,0,ET_EXEC,0x3e,1,0,0))
	with open("build/share/test/module/invalid_header_machine","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,1,1,0,ET_REL,0x3f,1,0,0))
	with open("build/share/test/module/invalid_header_version","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,1,1,0,ET_REL,0x3e,2,0,0))
	if (not os.path.exists("build/share/test/sys_lib")):
		os.mkdir("build/share/test/sys_lib")
	with open("build/share/test/sys_lib/invalid_header_signature","wb") as wf:
		wf.write(_generate_header(0xaabbccdd,2,1,1,0,ET_DYN,0x3e,1,0,0))
	os.chmod("build/share/test/sys_lib/invalid_header_signature",0o755)
	with open("build/share/test/sys_lib/invalid_header_word_size","wb") as wf:
		wf.write(_generate_header(0x464c457f,4,1,1,0,ET_DYN,0x3e,1,0,0))
	os.chmod("build/share/test/sys_lib/invalid_header_word_size",0o755)
	with open("build/share/test/sys_lib/invalid_header_endianess","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,2,1,0,ET_DYN,0x3e,1,0,0))
	os.chmod("build/share/test/sys_lib/invalid_header_endianess",0o755)
	with open("build/share/test/sys_lib/invalid_header_header_version","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,1,2,0,ET_DYN,0x3e,1,0,0))
	os.chmod("build/share/test/sys_lib/invalid_header_header_version",0o755)
	with open("build/share/test/sys_lib/invalid_header_abi","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,1,1,3,ET_DYN,0x3e,1,0,0))
	os.chmod("build/share/test/sys_lib/invalid_header_abi",0o755)
	with open("build/share/test/sys_lib/invalid_header_type","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,1,1,0,ET_EXEC,0x3e,1,0,0))
	os.chmod("build/share/test/sys_lib/invalid_header_type",0o755)
	with open("build/share/test/sys_lib/invalid_header_machine","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,1,1,0,ET_DYN,0x3f,1,0,0))
	os.chmod("build/share/test/sys_lib/invalid_header_machine",0o755)
	with open("build/share/test/sys_lib/invalid_header_version","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,1,1,0,ET_DYN,0x3e,2,0,0))
	os.chmod("build/share/test/sys_lib/invalid_header_version",0o755)
