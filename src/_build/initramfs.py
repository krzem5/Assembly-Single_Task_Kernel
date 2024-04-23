import os
import struct
import uuid



HEADER_SIGNATURE=0x007f534654494e49

FLAG_DIRECTORY=1



__all__=["create"]



class Node(object):
	def __init__(self,name,flags,file_path):
		name=bytes(name,"utf-8")
		self.name=name
		self.flags=flags
		self.children=[]
		self.data_size=(os.stat(file_path).st_size if file_path is not None else 0)
		self.size=((19+len(name))&0xfffffff8)+((self.data_size+7)&0xfffffff8)
		self.file_path=file_path

	def add_child(self,child):
		self.children.append(child)



def _create_node(node,path,file):
	if (path[0]!="/"):
		raise RuntimeError
	path=path[1:].split("/")
	for name in path[:-1]:
		for child in node.children:
			if (child.name.decode("utf-8")==name):
				node=child
				break
		else:
			child=Node(name,FLAG_DIRECTORY,None)
			node.add_child(child)
			node=child
	node.add_child(Node(path[-1],0,file))



def _calculate_node_size(node):
	for child in node.children:
		node.size+=_calculate_node_size(child)
	return node.size



def _write_node(node,wf):
	wf.write(struct.pack(f"<IIHBB{len(node.name)}s{(-12-len(node.name))&7}x",
		node.size,
		node.data_size,
		len(node.children),
		node.flags,
		len(node.name),
		node.name
	))
	if (node.data_size):
		with open(node.file_path,"rb") as rf:
			wf.write(rf.read())
		wf.write(b"\x00"*((-node.data_size)&7))
	for child in node.children:
		_write_node(child,wf)



def create(file_path,files):
	root=Node("",FLAG_DIRECTORY,None)
	for path,file in files.items():
		_create_node(root,path,file)
	_calculate_node_size(root)
	with open(file_path,"wb") as wf:
		wf.write(struct.pack("<Q16B",HEADER_SIGNATURE,*uuid.uuid4().bytes))
		_write_node(root,wf)
		wf.write(b"\x00"*((-root.size)&4095))
