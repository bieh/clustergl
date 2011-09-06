################################################################################ 
# Simple rotating cube program
################################################################################

#Name of the application. 
def name():
	return "lesson05"
	
#Free-form text description of the application
def descr():
	return "This is a rotating cube. It's pretty exciting"
	
#Return a list of configurable options that will be rendered in html
#Each individual option is a [name, description, type]
def options():
	return [ ["arg1", "The first argument", "text"],
			 ["arg2", "The second argument", "text"],
			 ["arg3", "The third and most awesome argument", "text"] ]

#Called when the app is being launched. A dict with the arguments is provided
#We return the command line to launch the app. Use the full path!
#We could also create config files or something here
#If you want to validate arguments in here, return None if you want to abort
#the run
def on_run(args):
	return "/home/paul/Projects/clustergl2/runtime/tests/lesson05/lesson05 " + args["arg1"]
	
#Called when we are being shut down. 
def on_shutdown():
	return
	
