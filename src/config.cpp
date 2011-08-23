/*******************************************************************************
	ClusterGL - config.cpp
*******************************************************************************/
#include "main.h"

Config::Config(string filename, string id){

    LOG("Loading configuration for '%s' from '%s'\n", 
    	id.c_str(), filename.c_str());
    	
	//TODO: Get rid of these!
	scaleX = 1.0f;
	scaleY = 1.0f;
	
	this->id = id;
    
	//Output options
	static cfg_opt_t output_opts[] = {
        CFG_INT(	(char *)("sizeX"), 		0, CFGF_NONE),
		CFG_INT(	(char *)("sizeY"), 		0, CFGF_NONE),
		CFG_INT(	(char *)("offsetX"), 	0, CFGF_NONE),
		CFG_INT(	(char *)("offsetY"), 	0, CFGF_NONE),
		CFG_INT(	(char *)("port"), 		0, CFGF_NONE),
		CFG_INT(	(char *)("angle"), 		0, CFGF_NONE),
		CFG_STR(	(char *)("address"), 	0, CFGF_NONE),
		CFG_STR(	(char *)("viewmode"),	0, CFGF_NONE),
        CFG_END()
    };
    
    //Top level options
	static cfg_opt_t opts[] = {
		CFG_SIMPLE_INT(		(char *)("totalWidth"), 	&totalWidth),
		CFG_SIMPLE_INT(		(char *)("totalHeight"), 	&totalHeight),
		CFG_SIMPLE_INT(		(char *)("fakeWindowX"), 	&fakeWindowX),
		CFG_SIMPLE_INT(		(char *)("fakeWindowY"), 	&fakeWindowY),
		CFG_SIMPLE_INT(		(char *)("syncRate"),		&syncRate),
		CFG_SIMPLE_BOOL(	(char *)("enableStats"),	&enableStats),
		CFG_SIMPLE_BOOL(	(char *)("enableWindowPositioning"),	&enableWindowPositioning),
		CFG_STR_LIST((char *)"capturePipeline", (char *)"{}", CFGF_NONE),
		CFG_STR_LIST((char *)"outputPipeline", (char *)"{}", CFGF_NONE),
		CFG_SEC(			(char *)"output", 	output_opts, CFGF_MULTI | CFGF_TITLE),
		CFG_STR(	(char *)("interceptMode"), 0, CFGF_NONE),
		CFG_END()
	};
	
	
	cfg_t *cfg;

	cfg = cfg_init(opts, CFGF_NONE);
	
	if(cfg_parse(cfg, filename.c_str()) == CFG_PARSE_ERROR){
		LOG("Couldn't parse config file\n");
		exit(1);
	}
	
	interceptMode = string(cfg_getstr(cfg, "interceptMode"));
	
	numOutputs = 0;
	
	int n = cfg_size(cfg, "output");
    
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
        
        sizeX = cfg_getint(o, "sizeX");
        sizeY = cfg_getint(o, "sizeY");
        angle = cfg_getint(o, "angle");
        offsetX = cfg_getint(o, "offsetX");
        offsetY = cfg_getint(o, "offsetY");
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
	
	cfg_free(cfg);
	
}
