import os
import subprocess
import sys
import threading



__all__=["ProcessPool"]



class ProcessPoolCommand(object):
	def __init__(self,pool,file,command,name):
		self.pool=pool
		self.file=file
		self.dependencies=set()
		self.command=command
		self.name=(name if name else file)
		self.fail=False

	def update(self):
		if (self.fail):
			self.pool.fail(self.file)
			return
		if (not self.dependencies):
			self.pool._dispatch(self)



class ProcessPool(object):
	def __init__(self,file_hash_list):
		self._file_hash_list=file_hash_list
		self._lock=threading.Lock()
		self._dependency_map={}
		self._ready_queue=[]
		self._pool_threads=[]
		self._has_error=False

	def add(self,dependencies,file,name,command):
		cmd=ProcessPoolCommand(self,file,command,name)
		self._lock.acquire()
		for dep in set(dependencies):
			if (dep not in self._dependency_map):
				self._dependency_map[dep]=[]
			if (self._dependency_map[dep] is False):
				cmd.fail=True
				break
			if (self._dependency_map[dep] is not True):
				self._dependency_map[dep].append(cmd)
				cmd.dependencies.add(dep)
		self._lock.release()
		cmd.update()

	def fail(self,file):
		update_list=[]
		self._lock.acquire()
		if (file not in self._dependency_map):
			self._dependency_map[file]=False
		elif (type(self._dependency_map[file])!=bool):
			for cmd in self._dependency_map[file]:
				cmd.fail=True
			self._dependency_map[file]=False
		self._lock.release()
		for cmd in update_list:
			cmd.update()

	def success(self,file):
		update_list=[]
		self._lock.acquire()
		if (file not in self._dependency_map):
			self._dependency_map[file]=True
		elif (type(self._dependency_map[file])!=bool):
			for cmd in self._dependency_map[file]:
				if (file in cmd.dependencies):
					cmd.dependencies.remove(file)
					update_list.append(cmd)
			self._dependency_map[file]=True
		self._lock.release()
		for cmd in update_list:
			cmd.update()

	def wait(self):
		while (self._ready_queue or self._pool_threads):
			for thread in self._pool_threads[:]:
				thread.join()
		for file in self._dependency_map.keys():
			if (type(self._dependency_map[file])!=bool):
				sys.stdout.buffer.write(b"\x1b[1;91mUnresolved condition: "+bytes(file,"utf-8")+b"\x1b[0m\n")
				sys.stdout.buffer.flush()
				self._has_error=True
		return self._has_error

	def _dispatch(self,cmd):
		self._lock.acquire()
		if (len(self._pool_threads)<os.cpu_count()-1):
			thr=threading.Thread(target=self._pool_thread,args=(len(self._pool_threads),))
			self._pool_threads.append(thr)
			thr.start()
		self._ready_queue.append(cmd)
		self._lock.release()

	def _pool_thread(self,index):
		while (True):
			self._lock.acquire()
			if (not self._ready_queue):
				self._pool_threads.remove(threading.current_thread())
				self._lock.release()
				return
			cmd=self._ready_queue.pop(0)
			self._lock.release()
			process=subprocess.run(cmd.command,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
			sys.stdout.buffer.write(b"\x1b[1;94m"+bytes(f"[{index:02d}] {cmd.name}","utf-8")+b"\x1b[0m\n"+process.stdout)
			sys.stdout.buffer.flush()
			if (process.returncode!=0):
				if (cmd.name.split(" ")[-1] in self._file_hash_list):
					del self._file_hash_list[cmd.name.split(" ")[-1]]
				elif (cmd.file.startswith("build") and os.path.exists(cmd.file)):
					os.remove(cmd.file)
				self._has_error=True
				self.fail(cmd.file)
			else:
				self.success(cmd.file)
