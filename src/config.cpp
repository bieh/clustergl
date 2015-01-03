/*******************************************************************************
	ClusterGL - config.cpp
*******************************************************************************/
#include "main.h"

Config::Config(string filename, string id){

	LOG("Loading configuration for '%s' from '%s'\n", 
		id.c_str(), filename.c_str());
	
	this->id = id;
	
	//Output options
	static cfg_opt_t output_opts[] = {
		CFG_INT(	 (char *)("sizeX"), 0, CFGF_NONE),
		CFG_INT(	 (char *)("sizeY"), 0, CFGF_NONE),
		CFG_INT(	 (char *)("positionX"), 0, CFGF_NONE),
		CFG_INT(	 (char *)("positionY"), 0, CFGF_NONE),
		CFG_INT(	 (char *)("offsetX"), 0, CFGF_NONE),
		CFG_INT(	 (char *)("offsetY"), 0, CFGF_NONE),
		CFG_INT(	 (char *)("port"), 0, CFGF_NONE),
		CFG_INT(	 (char *)("angle"), 0, CFGF_NONE),
		CFG_STR(	 (char *)("address"), 0, CFGF_NONE),
		CFG_FLOAT(	 (char *)("scaleX"), 1.0, CFGF_NONE),
		CFG_FLOAT(	 (char *)("scaleY"), 1.0, CFGF_NONE),
		CFG_STR(	 (char *)("viewmode"), 0, CFGF_NONE),
		CFG_END()
	};
	
	//Top level options
	static cfg_opt_t opts[] = {
		CFG_SIMPLE_INT(	(char *)("totalWidth"), 	&totalWidth),
		CFG_SIMPLE_INT(	(char *)("totalHeight"), &totalHeight),
		CFG_SIMPLE_INT(	(char *)("fakeWindowX"), &fakeWindowX),
		CFG_SIMPLE_INT(	(char *)("fakeWindowY"), &fakeWindowY),
		CFG_SIMPLE_INT(	(char *)("syncRate"), &syncRate),
		CFG_SIMPLE_INT(	(char *)("networkCompression"), &networkCompression),
		CFG_SIMPLE_BOOL(	(char *)("enableStats"),	 &enableStats),
		CFG_STR_LIST( (char *)"capturePipeline", (char *)"{}", CFGF_NONE),
		CFG_STR_LIST( (char *)"outputPipeline", (char *)"{}", CFGF_NONE),
		CFG_SEC(	 (char *)"output", 	output_opts, CFGF_MULTI | CFGF_TITLE),
		CFG_STR(	 (char *)("interceptMode"), 0, CFGF_NONE),
		CFG_STR(	 (char *)("capturePidFile"), 0, CFGF_NONE),
		CFG_SIMPLE_BOOL(	(char *)("remoteConfigServerEnabled"),	 
			&remoteConfigServerEnabled),
		CFG_END()
	};
	
	cfg_t *cfg;

	cfg = cfg_init(opts, CFGF_NONE);
	
	int parse_result = cfg_parse(cfg, filename.c_str());
	
	if(parse_result == CFG_FILE_ERROR){
		LOG("Error with the file (does it exist?)\n");
		exit(1);
	}
	
	if(parse_result == CFG_PARSE_ERROR){
		LOG("Couldn't parse config file\n");
		exit(1);
	}
	
	if(!cfg_getstr(cfg, "interceptMode")){
		LOG("No interceptMode specified, assuming \"x11\"\n");
		interceptMode = "x11";
	}else{	
		interceptMode = string(cfg_getstr(cfg, "interceptMode"));
	}
	
	if(!cfg_getstr(cfg, "capturePidFile")){
		capturePidFile = "";
	}else{	
		capturePidFile = string(cfg_getstr(cfg, "capturePidFile"));
	}
	
	numOutputs = 0;
	
	int n = cfg_size(cfg, "output");
	
	if(n == 0){
		LOG("No outputs specified, aborting\n");
		exit(1);
	}
	
	bool found_section = false;
	
	for(int i=0;i<n;i++){
		cfg_t *o = cfg_getnsec(cfg, "output", i);
	
		numOutputs++;
	
		char *addr = cfg_getstr(o, "address");
		int port = cfg_getint(o, "port");
	
		outputAddresses.push_back(string(addr));
		outputPorts.push_back(port);
		
		if(id != cfg_title(o)){
			continue;
		}
		
		found_section = true;
	
		sizeX = cfg_getint(o, "sizeX");
		sizeY = cfg_getint(o, "sizeY");
		positionX = cfg_getint(o, "positionX");
		positionY = cfg_getint(o, "positionY");
		angle = cfg_getint(o, "angle");
		offsetX = cfg_getint(o, "offsetX");
		offsetY = cfg_getint(o, "offsetY");
		scaleX = cfg_getfloat(o, "scaleX");
		scaleY = cfg_getfloat(o, "scaleY");
		viewModeString = string(cfg_getstr(o, "viewmode"));
	
		if(viewModeString == "viewport"){
			viewMode = VIEWMODE_VIEWPORT;
		}else if(viewModeString == "curve"){
			viewMode = VIEWMODE_CURVE;
		}else{
			LOG("Unknown viewmode '%s' - using default viewmode 'viewport'\n", 
				 viewModeString.c_str());
			viewMode = VIEWMODE_VIEWPORT;
		}	
	
		serverPort = port;
	}
	
	if(!found_section && id != "capture"){
		LOG("Couldn't find output '%s' in config file\n", id.c_str());
		exit(1);
	}

	for(unsigned int i = 0; i < cfg_size(cfg, "capturePipeline"); i++){
		capturePipeline.push_back(string(cfg_getnstr(cfg, "capturePipeline", i)));
        }
	if (capturePipeline.empty()){
		LOG("Something has to be defined in capturePipeline\n");
		exit(1);
	}
	
	cfg_free(cfg);

	if(remoteConfigServerEnabled){
		startRemoteConfigServer();
	}
	
}

/*******************************************************************************
	Config over HTTP
*******************************************************************************/
static void *callback(enum mg_event event, struct mg_connection *conn);
static struct mg_context *ctx;

void Config::startRemoteConfigServer(){

	const char *options[] = {"listening_ports", "8081", NULL};

	ctx = mg_start(&callback, NULL, options);
	
	if(!ctx){
		LOG("Failed to start webserver!\n");
	}else{
		LOG("Web configuration server started on port 8081\n");
	}

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

string variable_from_uri(string uri){
	uri = uri.substr(1,uri.size()); //lose the front /
	return uri;
}


int value_from_variable(string var){
	int start = var.rfind("/")+1;
	string substr = var.substr(start, var.size());

	int result;
	std::stringstream(substr) >> result;

	return result;
}

bool handled = false;
int config_result = 0;

void handle_config_attrib(string attrib, string uri, int *ptr){
	if(uri.find(attrib) == string::npos){
		return;
	}

	handled = true;

	string var = variable_from_uri(uri);

	if(var.find("/") == string::npos){
		config_result = *ptr;
		LOG("GET %s = %d\n", var.c_str(), *ptr);
		return;
	}

	int value = value_from_variable(var);
	*ptr = value;
	config_result = 1;

	LOG("SET %s = %d\n", attrib.c_str(), value);
}

/*******************************************************************************
 Web server high-level callback
*******************************************************************************/
string request(string url){
	//LOG("WEB: %s\n", url.c_str());

	handled = false;
	config_result = 0;

	handle_config_attrib("sizeX", url, &gConfig->sizeX);
	handle_config_attrib("sizeY", url, &gConfig->sizeY);
	handle_config_attrib("positionX", url, &gConfig->positionX);
	handle_config_attrib("positionY", url, &gConfig->positionY);
	handle_config_attrib("offsetX", url, &gConfig->offsetX);
	handle_config_attrib("offsetY", url, &gConfig->offsetY);
	handle_config_attrib("angle", url, &gConfig->angle);
	handle_config_attrib("totalWidth", url, &gConfig->totalWidth);
	handle_config_attrib("totalHeight", url, &gConfig->totalHeight);
	handle_config_attrib("screenGap", url, &gConfig->screenGap);
	//handle_config_attrib("scaleX", url, &gConfig->sizeX);
	//handle_config_attrib("scaleY", url, &gConfig->sizeX);
	handle_config_attrib("fakeWindowX", url, &gConfig->fakeWindowX);
	handle_config_attrib("fakeWindowY", url, &gConfig->fakeWindowY);

	if(handled){
		std::stringstream out;
		out << config_result;
		return out.str() + "\n"; 
	}

	string html = read_file("web" + url); 

	if(url == "/"){
		html = read_file("web/index.html");
	}

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
