#!/usr/bin/python

def n(s):
	if s.find("one value") > 0:
		return 1
	elif s.find("single boolean value") > 0:
		return 1
	elif s.find("single enumerated value") > 0:
		return 1
	elif s.find("a single integer value") > 0:
		return 1
	elif s.find("a single floating-point value") > 0:
		return 1
	elif s.find("a single positive floating-point value") > 0:
		return 1
	elif s.find("a single value") > 0:
		return 1
	elif s.find("two values") > 0:
		return 2
	elif s.find("three values") > 0:
		return 3
	elif s.find("four values") > 0:
		return 4
	elif s.find("four boolean values") > 0:
		return 4
	elif s.find("16 values") > 0:
		return 16
	elif s.find("sixteen values") > 0:
		return 16
	print "*** " + s
	return -1

param = ""

for line in open("input.txt").readlines():
	line = line.strip()
	
	if line.find("GL_") == 0:
		param = line
		
	elif line.find("params returns ") >= 0:
		print "case " + param + ": return " + str(n(line)) + ";"
