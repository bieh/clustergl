#!/usr/bin/python
params = {}


for line in open("/usr/include/GL/gl.h").readlines():
	if line.find("#define GL_") >= 0:
		line = line.replace("\t", " ").strip()
		parts = line.split(" ")
		name = parts[1].strip()
		val = int(parts[-1].strip(), 16)
		
		if val in params:
			params[val] = params[val] + " OR " + name
		else:
			params[val] = name

print "#include \"main.h\""
print
print "const char *getGLParamName(unsigned int param){"
print
print "\tswitch (param){"

for k,v in params.iteritems():
	print "\t\tcase " + str(hex(k)) + ": return \"" + v + "\";"
	
print "\t\tdefault: return \"Unknown\";"
print "\t}"
print "}"
