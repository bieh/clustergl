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
    
    //Top level options
	static cfg_opt_t opts[] = {
		CFG_SIMPLE_INT(		(char *)("totalWidth"), 	&offsetX),
		CFG_SIMPLE_INT(		(char *)("totalHeight"), 	&offsetY),
		CFG_SIMPLE_INT(		(char *)("fakeWindowX"), 	&fakeWindowX),
		CFG_SIMPLE_INT(		(char *)("fakeWindowY"), 	&fakeWindowY),
		CFG_SIMPLE_INT(		(char *)("port"), 			&serverPort),
		CFG_SIMPLE_INT(		(char *)("syncRate"),		&syncRate),
		CFG_SIMPLE_BOOL(	(char *)("enableStats"),	&enableStats),
		CFG_END()
	};
	
	//Output options
	static cfg_opt_t bookmark_opts[] = {
        CFG_INT(	(char *)("sizeX"), 		0, CFGF_NONE),
		CFG_INT(	(char *)("sizeY"), 		0, CFGF_NONE),
		CFG_INT(	(char *)("offsetX"), 	0, CFGF_NONE),
		CFG_INT(	(char *)("offsetY"), 	0, CFGF_NONE),
		CFG_INT(	(char *)("port"), 		0, CFGF_NONE),
		CFG_STR(	(char *)("hostname"), 	0, CFGF_NONE),
        CFG_END()
    };
	
	cfg_t *cfg;

	cfg = cfg_init(opts, CFGF_NONE);
	cfg_parse(cfg, filename.c_str());
	
	int n = cfg_size(cfg, "output");
    printf("%d configured outputs\n", n);
	
	cfg_free(cfg);
}
