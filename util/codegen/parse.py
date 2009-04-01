#!/usr/bin/python
import xml.dom.minidom
import re

#clientsource = open("glapi.cpp", "w")
lastOffset = 0

def getchildren(node, name):
	ret = []
	for e2 in node.childNodes:
		if e2.localName == name:
			ret.append(e2)
	return ret
	
def getparams(node):
	
	ret = "("

	params = getchildren(node, "param")
	
	for e in params:
		t = e.attributes["type"].value;
		v = e.attributes["name"].value
		
		if v != None and t != None:
			ret += t + " " + v
			
		if e != params[len(params)-1]:
			ret += ", "
						
	ret += ")"
	
	return ret
	
def isReturn(node):
	
	ret = getchildren(node, "return")			
	if len(ret) > 0:
		#print "//Ignored retval '" + ret[0].attributes["type"].value + "'" + " (" + str(count - good) + " ignored so far )"
		return True
	return False
	
def hasBuf(node):	
	params = getchildren(node, "param")
	
	for e in params:
		t = e.attributes["type"].value;
		v = e.attributes["name"].value
		
		if t.find("*") > 0:
		
			#Check to see if it's a nice simple buffer...
			if "count" in e.attributes.keys():
				return False
				
			if v == "v":
				return False
		
			#print "//Ignored buffer '" + t  + " " + v + "'" + " (" + str(count - good) + " ignored so far )"
			return True
					
	return False
	
def getCmdId(n):
	if "offset" in n.attributes.keys() and n.attributes["offset"].value != "assign":
		lastOffset = int(n.attributes["offset"].value)
		return n.attributes["offset"].value
	else:
		lastOffset = count
		return str(lastOffset)
	
def getType(letter):
	if letter == "d":
		return "GLdouble"
	elif letter == "f":
		return "GLfloat"
	elif letter == "s":
		return "GLshort"
	elif letter == "i":
		return "GLint"
	
	return "**ERROR " + letter + "**"

# Client side intercept function
def doFunction(n):

	cmdid = getCmdId(n)
	name = n.attributes["name"].value
	skip = False
	
	ret = getchildren(n, "return")	
	retType = "void"		
	if len(ret) > 0:
		retType = ret[0].attributes["type"].value
		
	
	#if retType != "void":
	#	skip = True
		
	if hasBuf(n):
		skip = True
	
	print "//" + cmdid
	print "extern \"C\" " + retType + " gl" + n.attributes["name"].value + getparams(n) + "{"
	
	if skip:
		print "\tLOG(\"Called unimplemted stub " + name + "!\\n\");"
	else:
		print "\tpushOp(" + cmdid + ");"
	
		params = getchildren(n, "param")	
		for e in params:
		
			v = e.attributes["name"].value
			name = n.attributes["name"].value;
			name = name.replace("MESA", "")
			name = name.replace("ARB", "")
			
			number_regex = re.compile('\d+').search(name)
			number = ""
			if number_regex != None:
				number = number_regex.group(0)
						
			l = "0"
			
			t = e.attributes["type"].value.replace(" *", "")
			
			if "count" in e.attributes.keys():				
				l = " sizeof(" + t + ") * " + e.attributes["count"].value
			else:
				end = " sizeof(" + t + ") * " + number
				
				l = end				
		
			if e.attributes["type"].value.find("*") > 0:
				print "\tpushBuf(" + v + "," + l + ");"
			else:				
				print "\tpushParam(" + v + ");"
				
		# Handle returns from this function
		if retType != "void":
			print "\n\t" + retType + " ret;"
			print "\tpushBuf(&ret, sizeof(" + retType + "), true);"
			print "\twaitForReturn();"
			print "\n\treturn ret;"
		
	print "}\n"
	
	return not skip
	
# Server side function that executes this given the CGL command object
def doExecute(n):
	
	cmdid = getCmdId(n)
	name = n.attributes["name"].value
	
	#if isReturn(n):
	#	print "//Skipped " + cmdid + " (" + name + ")\n"
	#	return False
		
	#if hasBuf(n):
	#	print "//Skipped " + cmdid + " (" + name + ")\n"
	#	return False
		
	print "//" + cmdid
	print "void EXEC_gl" + name + "(byte *commandbuf){";
	
	
	invoke = "\n\t";
	
	if isReturn(n):
		invoke += "pushRet("
	invoke += "gl" + name + "("
	
	params = getchildren(n, "param")
	for e in params:
		v = e.attributes["name"].value
		t = e.attributes["type"].value
	
		if t.find("*") <= 0:
			print "\t" + t + " *" + v + " = (" + t + "*)commandbuf;\t commandbuf += sizeof(" + t + ");"
		
		if t.find("*") > 0:
			invoke += "(" + t + ")popBuf()"
		else:		
			invoke += "*" + v
		
		if e != params[len(params)-1]:
			invoke += ", "
	
	if isReturn(n):
		invoke += ")"
	
	invoke += ");"
	
	
	print invoke
	
	print "}\n"
	
	return True
	
#Server side function that prints the command to stdout
def doText(n):
	
	cmdid = getCmdId(n)
	name = n.attributes["name"].value
	
	
	#if isReturn(n):
		#print "//Skipped " + cmdid + " (" + name + ")\n"
	#	return False
		
	#if hasBuf(n):
		#print "//Skipped " + cmdid + " (" + name + ")\n"
	#	return False
	
		
	#print "\tmFunctions[" + cmdid + "] = EXEC_gl" + name + ";";
	#return
	
		
	print "//" + cmdid
	print "void EXEC_gl" + name + "(byte *commandbuf){";			
		
	if not hasBuf(n):
	
		logcmd = "\n\tLOG(\"gl" + name + "("
		p = ""
	
		params = getchildren(n, "param")
		for e in params:
			v = e.attributes["name"].value
			t = e.attributes["type"].value
		
			print "\t" + t + " *" + v + " = (" + t + "*)commandbuf;\t commandbuf += sizeof(" + t + ");"
					
			logcmd += v + "=%0.1f"
			p += "(float)*" + v
		
			if e != params[len(params)-1]:
				logcmd += ", "
				p += ", "
		
		if len(p) > 0:
			logcmd += ")\\n\", " + p + ");"
		else:
			logcmd += ")\\n\");"
	
		print logcmd
	else:		
		print "\n\tLOG(\"gl" + name + "()\\n\");\n"
	
	print "}\n"
	
	return True
	
	

#entry
#print "Parsing!"

thefile = xml.dom.minidom.parse("gl_API.xml")

count = 0
good = 0
lastOffset = 0

# We want to find the 'function' bits
for e in thefile.childNodes[1].childNodes:
	for e2 in e.childNodes:
		if e2.nodeType == e2.ELEMENT_NODE and e2.localName == "function":
		
			#print e2.attributes.keys()
		
			#if doFunction(e2):
		#		good += 1
			
			if doExecute(e2):
				good+=1
			
			#if doText(e2):
			#	good += 1
				
			count+=1
			
			#if count > 7:
			#	break
	
	#if count > 7:
	#	break
		
#print
print "Total:\t " + str(count)
print "Good:\t " + str(good)
print "Bad:\t " + str(count - good)


#clientsource.close()
