CONFIG_WHITESPACE_CHARACTERS=b" \t\n\r"
CONFIG_DIGIT_CHARACTERS=b"0123456789"
CONFIG_IDENTIFIER_START_CHARACTERS=b"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_.$:/"
CONFIG_IDENTIFIER_CHARACTERS=CONFIG_IDENTIFIER_START_CHARACTERS+CONFIG_DIGIT_CHARACTERS+b"-"



CONFIG_TAG_TYPE_NONE=0
CONFIG_TAG_TYPE_INT=1
CONFIG_TAG_TYPE_STRING=2
CONFIG_TAG_TYPE_ARRAY=3



__all__=["parse","ConfigTag","CONFIG_TAG_TYPE_NONE","CONFIG_TAG_TYPE_INT","CONFIG_TAG_TYPE_STRING","CONFIG_TAG_TYPE_ARRAY"]



class ConfigTag(object):
	def __init__(self,parent,name,type,data):
		self.parent=parent
		self.name=name.decode("utf-8")
		self.type=type
		self.data=data

	def __repr__(self):
		out=self.name
		if (self.name and self.type!=CONFIG_TAG_TYPE_NONE):
			out+="="
		if (self.type==CONFIG_TAG_TYPE_INT):
			out+=str(self.data)
		elif (self.type==CONFIG_TAG_TYPE_STRING):
			out+=self.data
		elif (self.type==CONFIG_TAG_TYPE_ARRAY):
			out+="{"+",".join(map(str,self.data))+"}"
		return out

	def iter(self):
		if (self.type!=CONFIG_TAG_TYPE_ARRAY):
			return
		for tag in self.data:
			yield tag

	def find(self,name):
		for tag in self.data:
			if (tag.name==name):
				yield tag



def parse(file_path):
	with open(file_path,"rb") as rf:
		data=rf.read()
	out=ConfigTag(None,b"",CONFIG_TAG_TYPE_ARRAY,[])
	i=0
	while (i<len(data)):
		while (i<len(data) and data[i] in CONFIG_WHITESPACE_CHARACTERS):
			i+=1
		if (i==len(data)):
			break
		name=b""
		if (data[i] in CONFIG_IDENTIFIER_START_CHARACTERS):
			name_length=0
			while (i+name_length<len(data) and data[i+name_length] in CONFIG_IDENTIFIER_CHARACTERS):
				name_length+=1
			name=data[i:i+name_length]
			i+=name_length
		while (i<len(data) and data[i] not in b"\n" and data[i] in CONFIG_WHITESPACE_CHARACTERS):
			i+=1
		if (i==len(data) or data[i] in b",\n"):
			if (i<len(data)):
				i+=1
			if (not name):
				continue
			out.data.append(ConfigTag(out,name,CONFIG_TAG_TYPE_NONE,None))
			continue
		if (data[i] in b"}"):
			i+=1
			if (name):
				out.data.append(ConfigTag(out,name,CONFIG_TAG_TYPE_NONE,None))
			out=out.parent
			if (out is None):
				raise RuntimeError("Unbalanced brackets")
			continue
		if (name or data[i] in b"="):
			if (data[i] not in b"="):
				raise RuntimeError(f"Expected '=', got '{chr(data[i])}'")
			i+=1
			while (i<len(data) and data[i] not in b"\n" and data[i] in CONFIG_WHITESPACE_CHARACTERS):
				i+=1
		if (i==len(data)):
			out.data.append(ConfigTag(out,name,CONFIG_TAG_TYPE_NONE,None))
			continue
		if (data[i] in b"#"):
			while (i<len(data) and data[i] not in b"\n"):
				i+=1
			continue
		if (data[i] in b"{"):
			i+=1
			tag=ConfigTag(out,name,CONFIG_TAG_TYPE_ARRAY,[])
			out.data.append(tag)
			out=tag
			continue
		if (data[i] in CONFIG_DIGIT_CHARACTERS or (data[i] in b"-+" and data[i+1] in CONFIG_DIGIT_CHARACTERS)):
			start=i
			is_negative=False
			if (data[i] in b"-"):
				is_negative=True
				i+=1
			elif (data[i] in b"+"):
				i+=1
			value=0
			while (data[i] in CONFIG_DIGIT_CHARACTERS):
				value=value*10+data[i]-48
				i+=1
			if (data[i] in b"\n,#"):
				out.data.append(ConfigTag(out,name,CONFIG_TAG_TYPE_INT,(-value if is_negative else value)))
				continue
			i=start
		if (data[i] in b"\""):
			buffer=b""
			i+=1
			while (i<len(data) and data[i] not in b"\""):
				if (data[i] not in b"\\"):
					buffer+=data[i:i+1]
					i+=1
					continue
				i+=1
				raise RuntimeError(f"Escaped character: {chr(data[i])}")
			if (i==len(data)):
				raise RuntimeError("Unbalanced quotes")
			i+=1
			out.data.append(ConfigTag(out,name,CONFIG_TAG_TYPE_STRING,buffer.decode("utf-8")))
			continue
		string_length=0
		while (i+string_length<len(data) and data[i+string_length] not in b"\n,}"):
			string_length+=1
		out.data.append(ConfigTag(out,name,CONFIG_TAG_TYPE_STRING,data[i:i+string_length].decode("utf-8")))
		i+=string_length
	return out
