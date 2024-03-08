import os
import subprocess
import sys



PROCESS_POOL_COMMAND_COMPILE=0
PROCESS_POOL_COMMAND_LINK=1
PROCESS_POOL_COMMAND_PATCH=2



__all__=["PROCESS_POOL_COMMAND_COMPILE","PROCESS_POOL_COMMAND_LINK","PROCESS_POOL_COMMAND_PATCH","ProcessPool"]



class ProcessPool(object):
	def __init__(self):
		self.commands={PROCESS_POOL_COMMAND_COMPILE:[],PROCESS_POOL_COMMAND_LINK:[],PROCESS_POOL_COMMAND_PATCH:[]}

	def add(self,type_,file,command):
		self.commands[type_].append((file,command))

	def wait(self,file_hash_list):
		processes=[]
		for file,command in self.commands[PROCESS_POOL_COMMAND_COMPILE]:
			processes.append((file,subprocess.Popen(command,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)))
		error=False
		for file,process in processes:
			print("C "+file)
			process.wait()
			sys.stdout.buffer.write(process.stdout.read())
			if (process.returncode):
				del file_hash_list[file]
				error=True
		if (error):
			return True
		for file,command in self.commands[PROCESS_POOL_COMMAND_LINK]:
			print("L "+file)
			if (not subprocess.run(command).returncode):
				continue
			if (file.startswith("build/") and os.path.exists(file)):
				os.remove(file)
			error=True
		if (error):
			return True
		for file,command in self.commands[PROCESS_POOL_COMMAND_PATCH]:
			print("P "+file)
			command[0](*command[1:])
		return False
