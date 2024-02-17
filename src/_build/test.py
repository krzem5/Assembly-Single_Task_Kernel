import os
import struct



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
	with open("build/share/test/elf/invalid_header_signature","wb") as wf:
		wf.write(_generate_header(0xaabbccdd,2,1,1,0,ET_EXEC,0x3e,1,0,0))
	with open("build/share/test/elf/invalid_header_word_size","wb") as wf:
		wf.write(_generate_header(0x464c457f,4,1,1,0,ET_EXEC,0x3e,1,0,0))
	with open("build/share/test/elf/invalid_header_endianess","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,2,1,0,ET_EXEC,0x3e,1,0,0))
	with open("build/share/test/elf/invalid_header_header_version","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,1,2,0,ET_EXEC,0x3e,1,0,0))
	with open("build/share/test/elf/invalid_header_abi","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,1,1,3,ET_EXEC,0x3e,1,0,0))
	with open("build/share/test/elf/invalid_header_type","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,1,1,0,ET_DYN,0x3e,1,0,0))
	with open("build/share/test/elf/invalid_header_type_interpreter","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,1,1,0,ET_EXEC,0x3e,1,0,0))
	with open("build/share/test/elf/invalid_header_machine","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,1,1,0,ET_EXEC,0x3f,1,0,0))
	with open("build/share/test/elf/invalid_header_version","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,1,1,0,ET_EXEC,0x3e,2,0,0))
	with open("build/share/test/elf/multiple_interpreters","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,1,1,0,ET_EXEC,0x3e,1,64,2))
		wf.write(_generate_interpreter_program_header(64+56*2,2))
		wf.write(_generate_interpreter_program_header(64+56*2,2))
		wf.write(b"A\x00")
	with open("build/share/test/elf/unterminated_interpreter","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,1,1,0,ET_EXEC,0x3e,1,64,1))
		wf.write(_generate_interpreter_program_header(64+56,1))
		wf.write(b"A")
	with open("build/share/test/elf/interpreter_invalid_path","wb") as wf:
		wf.write(_generate_header_and_interpreter("/invalid/path"))
	with open("build/share/test/elf/interpreter_no_permissions","wb") as wf:
		wf.write(_generate_header_and_interpreter("/no-permission-node"))
	with open("build/share/test/elf/interpreter_invalid_header_signature","wb") as wf:
		wf.write(_generate_header_and_interpreter("/share/test/elf/invalid_header_signature"))
	with open("build/share/test/elf/interpreter_invalid_header_word_size","wb") as wf:
		wf.write(_generate_header_and_interpreter("/share/test/elf/invalid_header_word_size"))
	with open("build/share/test/elf/interpreter_invalid_header_endianess","wb") as wf:
		wf.write(_generate_header_and_interpreter("/share/test/elf/invalid_header_endianess"))
	with open("build/share/test/elf/interpreter_invalid_header_header_version","wb") as wf:
		wf.write(_generate_header_and_interpreter("/share/test/elf/invalid_header_header_version"))
	with open("build/share/test/elf/interpreter_invalid_header_abi","wb") as wf:
		wf.write(_generate_header_and_interpreter("/share/test/elf/invalid_header_abi"))
	with open("build/share/test/elf/interpreter_invalid_header_type","wb") as wf:
		wf.write(_generate_header_and_interpreter("/share/test/elf/invalid_header_type_interpreter"))
	with open("build/share/test/elf/interpreter_invalid_header_machine","wb") as wf:
		wf.write(_generate_header_and_interpreter("/share/test/elf/invalid_header_machine"))
	with open("build/share/test/elf/interpreter_invalid_header_version","wb") as wf:
		wf.write(_generate_header_and_interpreter("/share/test/elf/invalid_header_version"))
	os.chmod("build/share/test/elf/invalid_header_signature",0o755)
	os.chmod("build/share/test/elf/invalid_header_word_size",0o755)
	os.chmod("build/share/test/elf/invalid_header_endianess",0o755)
	os.chmod("build/share/test/elf/invalid_header_header_version",0o755)
	os.chmod("build/share/test/elf/invalid_header_abi",0o755)
	os.chmod("build/share/test/elf/invalid_header_type",0o755)
	os.chmod("build/share/test/elf/invalid_header_type_interpreter",0o755)
	os.chmod("build/share/test/elf/invalid_header_machine",0o755)
	os.chmod("build/share/test/elf/invalid_header_version",0o755)
	os.chmod("build/share/test/elf/multiple_interpreters",0o755)
	os.chmod("build/share/test/elf/unterminated_interpreter",0o755)
	os.chmod("build/share/test/elf/interpreter_invalid_path",0o755)
	os.chmod("build/share/test/elf/interpreter_no_permissions",0o755)
	os.chmod("build/share/test/elf/interpreter_invalid_header_signature",0o755)
	os.chmod("build/share/test/elf/interpreter_invalid_header_word_size",0o755)
	os.chmod("build/share/test/elf/interpreter_invalid_header_endianess",0o755)
	os.chmod("build/share/test/elf/interpreter_invalid_header_header_version",0o755)
	os.chmod("build/share/test/elf/interpreter_invalid_header_abi",0o755)
	os.chmod("build/share/test/elf/interpreter_invalid_header_type",0o755)
	os.chmod("build/share/test/elf/interpreter_invalid_header_machine",0o755)
	os.chmod("build/share/test/elf/interpreter_invalid_header_version",0o755)
