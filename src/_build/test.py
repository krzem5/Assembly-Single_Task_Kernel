import os
import struct



ET_EXEC=2
ET_DYN=3



__all__=["generate_test_resource_files"]



def _generate_header(signature,word_size,endianess,header_version,abi,type,machine,version):
	return struct.pack("<IBBBB8xHHI40x",signature,word_size,endianess,header_version,abi,type,machine,version)



def generate_test_resource_files():
	if (not os.path.exists("build/share/test/elf")):
		os.mkdir("build/share/test/elf")
	with open("build/share/test/elf/invalid_header_signature","wb") as wf:
		wf.write(_generate_header(0xaabbccdd,2,1,1,0,ET_EXEC,0x3e,1))
	with open("build/share/test/elf/invalid_header_word_size","wb") as wf:
		wf.write(_generate_header(0x464c457f,4,1,1,0,ET_EXEC,0x3e,1))
	with open("build/share/test/elf/invalid_header_endianess","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,2,1,0,ET_EXEC,0x3e,1))
	with open("build/share/test/elf/invalid_header_header_version","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,1,2,0,ET_EXEC,0x3e,1))
	with open("build/share/test/elf/invalid_header_abi","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,1,1,3,ET_EXEC,0x3e,1))
	with open("build/share/test/elf/invalid_header_type","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,1,1,0,ET_DYN,0x3e,1))
	with open("build/share/test/elf/invalid_header_machine","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,1,1,0,ET_EXEC,0x3f,1))
	with open("build/share/test/elf/invalid_header_version","wb") as wf:
		wf.write(_generate_header(0x464c457f,2,1,1,0,ET_EXEC,0x3e,2))
	os.chmod("build/share/test/elf/invalid_header_signature",0o755)
	os.chmod("build/share/test/elf/invalid_header_word_size",0o755)
	os.chmod("build/share/test/elf/invalid_header_endianess",0o755)
	os.chmod("build/share/test/elf/invalid_header_header_version",0o755)
	os.chmod("build/share/test/elf/invalid_header_abi",0o755)
	os.chmod("build/share/test/elf/invalid_header_type",0o755)
	os.chmod("build/share/test/elf/invalid_header_machine",0o755)
	os.chmod("build/share/test/elf/invalid_header_version",0o755)
