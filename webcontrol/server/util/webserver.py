#!/usr/bin/python
################################################################################
# Simple HTTP server
################################################################################
import string, cgi, time
import json as simplejson
from os import curdir, sep
from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer
#import pri

_get_handler = None

def extract_args(path):
	if path.find("?") < 0:
		return {"path":path}
	base = path[:path.find("?")]
	ret = {"path":base}
	for kv in path[path.find("?")+1:].split("&"):
		s = kv.split("=")
		ret[s[0].strip().lower()] = s[1].strip()
	return ret
		

class WebRequestHandler(BaseHTTPRequestHandler):
	def do_GET(self):
		try:		
			ret = _get_handler(extract_args(self.path))
			
			if ret == None:
				self.send_error(404,'Not found: %s' % self.path)
				return
			
			self.send_response(200)
			self.send_header('Content-type', 'text/plain')
			self.end_headers()				
			self.wfile.write(simplejson.dumps(ret, sort_keys=True, indent=4))
		except IOError:
			self.send_error(500,'Error handling request: %s' % self.path)
	 

def run(get_handler):

	global _get_handler
	_get_handler = get_handler

	try:
		server = HTTPServer(('', 8080), WebRequestHandler)
		print 'Waiting for requests'
		server.serve_forever()
	except KeyboardInterrupt:
		print 'Shutting down server'
		server.socket.close()
