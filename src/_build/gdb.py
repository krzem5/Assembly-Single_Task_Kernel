import gdb
import json



with open("build/kernel/kernel.json","r") as rf:
	sections=json.loads(rf.read())
gdb.execute(f"add-symbol-file build/kernel/kernel.elf 0x{min(sections.values()):x}"+"".join([f" -s {name} 0x{address:x}" for name,address in sections.items()]))
# thread_handle_type=int(gdb.lookup_symbol("thread_handle_type")[0].value())
# print(thread_handle_type)



class KernelListThreads(gdb.Command):
	def __init__(self):
		super(KernelListThreads, self).__init__("kernel-list-threads",gdb.COMMAND_USER)

	def invoke(self,arg,from_tty):
		print(gdb.lookup_symbol("handle_get_descriptor")[0].value())

KernelListThreads()

