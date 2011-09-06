#!/usr/bin/python
################################################################################ 
# ClusterGL webcontrol - server component
# (This runs on the master CGL machine - in Symphony, DN1)
#
# It provides a JSON interface for configuring, launching and managing apps
# running under CGL. Intended to be used by the php frontend. 
################################################################################

import util.webserver
import util.cglrunner
import glob, imp
from os.path import join, basename, splitext
   
################################################################################ 
# Some hackery to locate and load all app plugins dynamically
################################################################################
def load_apps(dir):
	return dict( _load(path) for path in glob.glob(join(dir,'[!_]*.py')) )

def _load(path):
	name, ext = splitext(basename(path))
	return name, imp.load_source(name, path)
	
app_modules = load_apps("apps")

################################################################################ 
# List configured apps
################################################################################
def on_list_apps(args):	
	ret = []
	for name,app in app_modules.iteritems():
		a = {"name":app.name(), "descr":app.descr(), "options":app.options()}
		ret.append(a)
	return ret
	
################################################################################ 
# Details of one app
################################################################################
def on_app_details(args):	
	for name,app in app_modules.iteritems():
		if args["name"] == app.name():
			a = {"name":app.name(), "descr":app.descr(), "options":app.options()}
			return a
	return {}
	
################################################################################ 
# Run CGL with a selected configuration
################################################################################
def on_run(args):	

	#TODO: What if someone else is already running?
	#TODO: Validate user

	#First get the command line from the application and validate that
	app = app_modules[args["app"]]
	appcmd = app.on_run(args)
	
	if appcmd == None:
		raise Exception("Failed to configure app")
		
	args["appcmd"] = appcmd
	
	return util.cglrunner.run(args)
	
################################################################################ 
# Return the status of the running CGL (if any)
################################################################################
def on_status(args):	
	return util.cglrunner.status(args)
	
################################################################################ 
# Kill the current CGL instance
################################################################################
def on_stop(args):
	return util.cglrunner.stop(args)


################################################################################ 
# Main web request dispatcher
################################################################################

#these are the handlers for various operations
dispatcher_lookup = {	
						"list_apps" : on_list_apps,
						"run": on_run,
						"status": on_status,
						"stop": on_stop,
						"app_details" : on_app_details,
					}

def on_request(args):

	if args["op"] in dispatcher_lookup:
		return dispatcher_lookup[args["op"]](args)
			
	return None #will 404	
	
################################################################################ 
# Entry
################################################################################
def main():
    util.webserver.run(on_request)

if __name__ == '__main__':
    main()

