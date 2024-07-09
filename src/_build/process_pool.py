import os
import subprocess
import sys
import threading



__all__=["ProcessPool"]



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

	def fail(self):
		self.pool.fail(self.file)

	def _trigger(self):
		thr=threading.Thread(target=self._thread)
		thr.start()
		self.pool._threads.append(thr)

	def _thread(self):
		error=False
		output=b""
		if (isinstance(self.command[0],str)):
			process=subprocess.run(self.command,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
			output=process.stdout
			error=(process.returncode!=0)
		else:
			self.command[0](*self.command[1:])
		sys.stdout.buffer.write(b"\x1b[1;94m"+bytes(self.name,"utf-8")+b"\x1b[0m\n"+output)
		sys.stdout.buffer.flush()
		if (error):
			if (self.name.split(" ")[-1] in self.pool._file_hash_list):
				del self.pool._file_hash_list[self.name.split(" ")[-1]]
			elif (self.file.startswith("build") and os.path.exists(self.file)):
				os.remove(self.file)
			self.pool._error=True
			self.pool.fail(self.file)
		else:
			self.pool.dispatch(self.file)
		self.pool._process_count-=1



class ProcessPool(object):
	def __init__(self,file_hash_list):
		self._file_hash_list=file_hash_list
		self._error=None
		self._dependency_map={}
		self._threads=[]
		self._lock=threading.Lock()
		self._process_count=0

	def add(self,dependencies,file,name,command):
		pool_command=ProcessPoolCommand(self,file,dependencies,command,name)
		fail=False
		self._lock.acquire()
		for dep in dependencies:
			dep_state=self._dependency_map.get(dep,[])
			if (dep_state is True):
				pool_command.drop_dependency(dep)
			elif (dep_state is False):
				fail=True
				break
			else:
				dep_state.append(pool_command)
				self._dependency_map[dep]=dep_state
		self._process_count+=1
		self._lock.release()
		if (fail):
			self.fail(file)

	def fail(self,file):
		self._lock.acquire()
		fail_list=[file]
		while (fail_list):
			file=fail_list.pop()
			if (file in self._dependency_map and self._dependency_map[file] is not False):
				for pool_command in self._dependency_map[file]:
					fail_list.append(pool_command.file)
			self._dependency_map[file]=False
		self._lock.release()

	def dispatch(self,file):
		self._lock.acquire()
		if (file in self._dependency_map):
			if (type(self._dependency_map[file])!=bool):
				for pool_command in self._dependency_map[file]:
					pool_command.drop_dependency(file)
				self._dependency_map[file]=True
		else:
			self._dependency_map[file]=True
		self._lock.release()

	def wait(self):
		self._error=False
		while (self._threads):
			self._threads.pop().join()
		for name in self._dependency_map.keys():
			if (type(self._dependency_map[name])!=bool):
				sys.stdout.buffer.write(b"\x1b[1;91mUnresolved condition: "+bytes(name,"utf-8")+b"\x1b[0m\n")
				sys.stdout.buffer.flush()
				self._error=True
		return self._error or bool(self._process_count)
