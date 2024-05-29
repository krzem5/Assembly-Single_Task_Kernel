import gdb
import json



gdb.execute(f"add-symbol-file build/kernel/kernel.elf 0xffffffffc0100000",to_string=True)



char_ptr_t=gdb.lookup_type("char").pointer()
handle_descriptor_t=gdb.lookup_type("handle_descriptor_t")
rb_tree_node_t=gdb.lookup_type("rb_tree_node_t")
string_t=gdb.lookup_type("string_t")
thread_t=gdb.lookup_type("thread_t")



def offsetof(type,field):
	return int(gdb.Value(0).cast(type.pointer())[field].address)



def rb_tree_lookup_node(tree,key):
	x=tree["root"].cast(rb_tree_node_t.pointer())[0]
	while (x.address):
		x_key=int(x["key"])
		if (x_key==key):
			return x
		x=x["rb_nodes"][x_key<key].cast(rb_tree_node_t.pointer())[0]
	return None



def rb_tree_iter_start(tree):
	x=tree["root"].cast(rb_tree_node_t.pointer())[0]
	if (not x.address):
		return None
	while (True):
		left=x["rb_left"].cast(rb_tree_node_t.pointer())[0]
		if (not left.address):
			break
		x=left
	return x



def rb_tree_iter_next(tree,x):
	right=x["rb_right"].cast(rb_tree_node_t.pointer())[0]
	if (right.address):
		x=right
		while (True):
			left=x["rb_left"].cast(rb_tree_node_t.pointer())[0]
			if (not left.address):
				return x
			x=left
	while (True):
		y=x
		x=(x["rb_parent_and_color"]&(~1)).cast(rb_tree_node_t.pointer())[0]
		if (not x.address):
			return None
		if (y.address!=x["rb_right"]):
			return x



def rb_tree_iter(tree):
	node=rb_tree_iter_start(tree)
	while (node is not None):
		yield node
		node=rb_tree_iter_next(tree,node)



class KernelListThreads(gdb.Command):
	def __init__(self):
		super(KernelListThreads,self).__init__("kernel-list-threads",gdb.COMMAND_USER)

	def invoke(self,arg,from_tty):
		_handle_type_tree=gdb.lookup_symbol("_handle_type_tree")[0].value()
		thread_handle_type=int(gdb.lookup_symbol("thread_handle_type")[0].value())
		thread_handle_descriptor=gdb.Value(int(rb_tree_lookup_node(_handle_type_tree,thread_handle_type).address)-offsetof(handle_descriptor_t,"rb_node")).cast(handle_descriptor_t.pointer())[0]
		for k in rb_tree_iter(thread_handle_descriptor["tree"]):
			thread=gdb.Value(int(k.address)-offsetof(thread_t,"handle")).cast(thread_t.pointer())[0]
			print(thread["name"].cast(string_t.pointer())[0]["data"].cast(char_ptr_t).string())



KernelListThreads()

