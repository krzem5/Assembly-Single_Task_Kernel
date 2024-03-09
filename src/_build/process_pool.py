import os
import subprocess
import sys
import threading



__all__=["ProcessPool"]



def _process_thread(pool_command):
	error=False
	output=b""
	if (isinstance(pool_command.command[0],str)):
		process=subprocess.run(pool_command.command,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
		output=process.stdout
		error=(process.returncode!=0)
	else:
		pool_command.command[0](*pool_command.command[1:])
	sys.stdout.buffer.write(b"\x1b[1;94m"+bytes(pool_command.name,"utf-8")+b"\x1b[0m\n"+output)
	if (error):
		del pool_command.pool._file_hash_list[pool_command.name]
		pool_command.pool._error=True
	else:
		pool_command.pool.dispatch(pool_command.file)



class ProcessPoolCommand(object):
	def __init__(self,pool,file,dependencies,command,name):
		self.pool=pool
		self.file=file
		self.dependencies=set(dependencies)
		self.command=command
		self.name=(name if name else file)
		if (not self.dependencies):
			self._trigger()

	def drop_dependency(self,name):
		self.dependencies.remove(name)
		if (not self.dependencies):
			self._trigger()

	def _trigger(self):
		thr=threading.Thread(target=_process_thread,args=(self,))
		thr.start()
		self.pool._threads.append(thr)



class ProcessPool(object):
	def __init__(self):
		self._file_hash_list=None
		self._error=None
		self._dependency_map={}
		self._threads=[]
		self._lock=threading.Lock()

	def add(self,dependencies,file,name,command):
		pool_command=ProcessPoolCommand(self,file,dependencies,command,name)
		self._lock.acquire()
		for dep in dependencies:
			dep_state=self._dependency_map.get(dep,[])
			if (dep_state is True):
				pool_command.drop_dependency(dep)
			else:
				dep_state.append(pool_command)
				self._dependency_map[dep]=dep_state
		self._lock.release()

	def dispatch(self,file):
		self._lock.acquire()
		if (file in self._dependency_map and self._dependency_map[file] is not True):
			for pool_command in self._dependency_map[file]:
				pool_command.drop_dependency(file)
		self._dependency_map[file]=True
		self._lock.release()

	def wait(self,file_hash_list):
		self._file_hash_list=file_hash_list
		self._error=False
		while (self._threads):
			self._threads.pop().join()
		return self._error
