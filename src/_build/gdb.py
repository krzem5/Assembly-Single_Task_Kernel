import gdb
import json
import struct



gdb.execute("source src/_build/linker.py")
gdb.execute("add-symbol-file build/kernel/kernel.elf 0xffffffffc0100000",to_string=True)



MODULE_STATE_LOADING=1
MODULE_STATE_LOADED=2
MODULE_STATE_UNLOADING=3
MODULE_STATE_UNLOADED=4

THREAD_STATE_TYPE_NONE=0
THREAD_STATE_TYPE_QUEUED=1
THREAD_STATE_TYPE_RUNNING=2
THREAD_STATE_TYPE_AWAITING_EVENT=3
THREAD_STATE_TYPE_TERMINATED=255

event_t=gdb.lookup_type("event_t")
event_thread_container_t=gdb.lookup_type("event_thread_container_t")
handle_descriptor_t=gdb.lookup_type("handle_descriptor_t")
mmap_region_t=gdb.lookup_type("mmap_region_t")
module_t=gdb.lookup_type("module_t")
process_t=gdb.lookup_type("process_t")
rb_tree_node_t=gdb.lookup_type("rb_tree_node_t")
string_t=gdb.lookup_type("string_t")
thread_t=gdb.lookup_type("thread_t")

_handle_type_tree=gdb.lookup_symbol("_handle_type_tree")[0].value()
event_handle_type=int(gdb.lookup_symbol("event_handle_type")[0].value())
module_handle_type=int(gdb.lookup_symbol("module_handle_type")[0].value())
thread_handle_type=int(gdb.lookup_symbol("thread_handle_type")[0].value())



def offsetof(type,field):
	return int(gdb.Value(0).cast(type.pointer())[field].address)



def containerof(value,type,field):
	return gdb.Value(int(value.address)-offsetof(type,field)).cast(type.pointer())[0]



def string(value):
	string=value.cast(string_t.pointer())[0]
	return gdb.inferiors()[0].read_memory(int(string["data"].address),int(string["length"])).tobytes().decode("utf-8")



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



def iter_linked_list(object,type):
	entry=object["head"].cast(type.pointer())[0]
	while (entry.address):
		yield entry
		entry=entry["next"].cast(type.pointer())[0]



def handle_get_descriptor(type):
	return gdb.Value(int(rb_tree_lookup_node(_handle_type_tree,type).address)-offsetof(handle_descriptor_t,"rb_node")).cast(handle_descriptor_t.pointer())[0]



def handle_lookup(handle_id,type):
	thread_handle_descriptor=handle_get_descriptor(type)
	return rb_tree_lookup_node(thread_handle_descriptor["tree"],handle_id)



class KernelListThreads(gdb.Command):
	def __init__(self):
		super(KernelListThreads,self).__init__("kernel-list-threads",gdb.COMMAND_USER)

	def invoke(self,arg,from_tty):
		thread_to_event={}
		event_handle_descriptor=handle_get_descriptor(event_handle_type)
		for k in rb_tree_iter(event_handle_descriptor["tree"]):
			event=containerof(k,event_t,"handle")
			for e in iter_linked_list(event,event_thread_container_t):
				thread_handle=handle_lookup(e["thread"],thread_handle_type)
				if (thread_handle is None):
					continue
				thread=containerof(thread_handle,thread_t,"handle")
				if (e["sequence_id"]!=thread["event_sequence_id"]):
					continue
				key=int(thread.address)
				if (key not in thread_to_event):
					thread_to_event[key]=[]
				thread_to_event[key].append(event["name"].string())
		thread_handle_descriptor=handle_get_descriptor(thread_handle_type)
		for k in rb_tree_iter(thread_handle_descriptor["tree"]):
			thread=containerof(k,thread_t,"handle")
			process=thread["process"].cast(process_t.pointer())[0]
			line=string(process["name"])+": "+string(thread["name"])+": "+{
				THREAD_STATE_TYPE_NONE: "<none>",
				THREAD_STATE_TYPE_QUEUED: "queued",
				THREAD_STATE_TYPE_RUNNING: "running",
				THREAD_STATE_TYPE_AWAITING_EVENT: "awaiting event",
				THREAD_STATE_TYPE_TERMINATED: "terminated"
			}[int(thread["state"])]
			if (thread["state"]==THREAD_STATE_TYPE_QUEUED):
				line+=f" ({thread['scheduler_load_balancer_queue_index']})"
			elif (thread["state"]==THREAD_STATE_TYPE_AWAITING_EVENT):
				line+=" ("+", ".join(thread_to_event[int(thread.address)])+")"
			print(line)



class KernelListModules(gdb.Command):
	def __init__(self):
		super(KernelListModules,self).__init__("kernel-list-modules",gdb.COMMAND_USER)

	def invoke(self,arg,from_tty):
		module_handle_descriptor=handle_get_descriptor(module_handle_type)
		for k in rb_tree_iter(module_handle_descriptor["tree"]):
			module=containerof(k,module_t,"handle")
			name=string(module["name"])
			state=None
			if (module["state"]==MODULE_STATE_LOADING):
				state="loading"
			elif (module["state"]==MODULE_STATE_LOADED):
				state="loaded @ "+hex(int(module["region"].cast(mmap_region_t.pointer())[0]["rb_node"]["key"]))
			elif (module["state"]==MODULE_STATE_UNLOADING):
				state="unloading"
			elif (module["state"]==MODULE_STATE_UNLOADED):
				state="unloaded"
			print(f"{name}: {state}")



KernelListThreads()
KernelListModules()



for k in rb_tree_iter(handle_get_descriptor(module_handle_type)["tree"]):
	module=containerof(k,module_t,"handle")
	if (module["state"]!=MODULE_STATE_LOADED):
		continue
	name=string(module["name"])
	base_address=int(module["region"].cast(mmap_region_t.pointer())[0]["rb_node"]["key"])
	print(name,hex(base_address))
	patch_module_for_gdb(f"build/module/{name}.mod",f"build/gdb/{name}.mod",base_address)
	gdb.execute(f"add-symbol-file build/gdb/{name}.mod -readnow {base_address}",to_string=True)
