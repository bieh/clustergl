/*******************************************************************************
 Web server stuff for the touchscreen
*******************************************************************************/
#include "main.h"
#include "mongoose.h"

static void *callback(enum mg_event event, struct mg_connection *conn);
static struct mg_context *ctx;

/*******************************************************************************
 Set up and start the embedded web server
*******************************************************************************/
bool begin_web(){

	//Set up the webserver for the touchscreen

	const char *options[] = {"listening_ports", "8080", NULL};

	ctx = mg_start(&callback, NULL, options);

	if(!ctx){
		return false;
	}

	LOG("Web server started on port 8080\n");
	return true;
}

void end_web(){
	mg_stop(ctx);

	LOG("Web server terminated");
}


/*******************************************************************************
 Utility functions
*******************************************************************************/
string read_file(string filepath){
	FILE *f = fopen(filepath.c_str(), "r");

	if(!f){
		return "";
	}

	const int len = 1024 * 100;
	
	char buf[len];
	memset(buf, 0, len);
	fread(buf, len, 1, f);
	fclose(f);

	return string(buf);
}

int keycode_from_uri(string uri){
	int start = uri.rfind("/")+1;
	string substr = uri.substr(start, uri.size());

	int result;
	std::stringstream(substr) >> result;

	return result;
}

/*******************************************************************************
 Web server high-level callback
*******************************************************************************/
string request(string url){
	LOG("WEB: %s\n", url.c_str());

	//See if it's a keyboard event request
	if(url.find("/kbdown/") != string::npos){
		mInject->keydown(keycode_from_uri(url));
		return "okay\n";
	}

	if(url.find("/kbup/") != string::npos){
		mInject->keyup(keycode_from_uri(url));
		return "okay\n";
	}

	//OK, if it's not a keyboard request, maybe it's a file request
	string html = read_file("web" + url); //turns into web/foo.html or whatever

	if(html.length() > 0){
		return html;
	}

	LOG("WEB: 404 NOT FOUND %s\n", url.c_str());

	//Give up!
	return string("404 not found: ") + url + "\n";
}

string guess_content_type(string url){
	if(url.find(".css") != string::npos){
		return "text/css";
	}

	if(url.find(".js") != string::npos){
		return "script/javascript";
	}

	return "text/html";
}

/*******************************************************************************
 Web server low-level callback
*******************************************************************************/
static void *callback(enum mg_event event, struct mg_connection *conn) {
	const struct mg_request_info *request_info = mg_get_request_info(conn);

	if (event == MG_NEW_REQUEST) {
		string content = request(string(request_info->uri));
		mg_printf(conn,
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: %s\r\n"
				"Content-Length: %d\r\n"   
				"\r\n"
				"%s",
				guess_content_type(string(request_info->uri)).c_str(),
				(int)content.length(), 
				content.c_str());
		// Mark as processed
		return (void *)"";
	} else if(event == MG_EVENT_LOG) {
		printf("Webserver: %s\n", request_info->log_message);
		return NULL;
	}

	return NULL;
}
