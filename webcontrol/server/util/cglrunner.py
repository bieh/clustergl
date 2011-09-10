################################################################################ 
# Manages the horrible details of actually invoking and killing ClusterGL
################################################################################

import config, os, time, sys

running_pid = 0


################################################################################ 
# Invoke CGL
################################################################################
def run(args):
	
	pid = os.fork()
	
	global running_pid
	
	if pid != 0:
		running_pid = pid
			
		#Parent process. Return a result to the waiting web client
		return status(args)

	#At this point we're in the child. Start up CGL.
	#TODO: This is just asking for bash injection attacks!
	
	#First start up the renderers
	for hostname in config.CGL_RENDERERS:
		pid = os.fork()
		cmdline = "ssh \"" + hostname + "\" \"" +\
			"killall cglrender -9 2> /dev/null; DISPLAY=:0 CGL_CONFIG_FILE='" +\
			config.CGL_CONFIG_FILE + "' '" +\
			config.CGL_LOCATION + "/runtime/cgl-render' " + hostname + "\" > /dev/null" 
		if pid != 0:
			continue
		os.system(cmdline)
		
		#All done, bail out!
		os._exit(0)

	#let them start up...
	time.sleep(2)
	
	#okay, now start up the capture
	cmdline = "LD_PRELOAD=\"" + config.CGL_LOCATION + "/runtime/libcgl-capture.so\" " +\
	"CGL_CONFIG_FILE=\"" + config.CGL_CONFIG_FILE + "\" " + args["appcmd"] + " > /dev/null"
	
	os.system(cmdline)
	
	#All done, bail out!
	os._exit(0)


################################################################################ 
# Kill all running renderers and (if possible) the last host process
################################################################################
def stop(args):

	global running_pid
	
	#First start up the renderers
	for hostname in config.CGL_RENDERERS:		
		cmdline = "ssh \"" + hostname + "\" \"" +\
			"killall cgl-render -9 2> /dev/null\""	
		os.system(cmdline)
	
	#Now go look for the CGL capture PID
	if os.path.exists(config.CGL_PID_FILE):
		pid = open(config.CGL_PID_FILE).read()
		cmdline = "kill -9 " + pid + " "
		print cmdline
		os.system(cmdline)
	else:
		print "No CGL_PID_FILE"
	
	running_pid = 0
	
	return True
	
################################################################################ 
# Return some information about the current CGL process
################################################################################
def status(args):
	if running_pid <= 0:
		return {"status":"stopped"}
		
	return {"status":"running",
			"pid":running_pid}
	
