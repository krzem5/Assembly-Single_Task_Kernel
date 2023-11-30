import os
import struct



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
		self.size+=child.size
		self.children.append(child)



def _generate_tree(directory,node):
	for name in sorted(os.listdir(directory)):
		path=os.path.join(directory,name)
		if (os.path.isfile(path)):
			node.add_child(Node(name,0,path))
		else:
			child_node=Node(name,FLAG_DIRECTORY,None)
			_generate_tree(path,child_node)
			node.add_child(child_node)



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



def create(base_directory,file_path):
	root=Node("",FLAG_DIRECTORY,None)
	_generate_tree(base_directory,root)
	with open(file_path,"wb") as wf:
		wf.write(struct.pack("<Q",HEADER_SIGNATURE))
		_write_node(root,wf)
		wf.write(b"\x00"*((-root.size)&4095))
