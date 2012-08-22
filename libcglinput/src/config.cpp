#include "main.h"
#include <confuse.h>

Config::Config(string filename){

	LOG("Loading configuration from '%s'\n", filename.c_str());

	spacenav_device = new char[128];
	
	//Top level options
	static cfg_opt_t opts[] = {
		CFG_SIMPLE_INT(	(char *)("spacenav_0"), &spacenav_codes[0]),
		CFG_SIMPLE_INT(	(char *)("spacenav_1"), &spacenav_codes[1]),
		CFG_SIMPLE_INT(	(char *)("spacenav_2"), &spacenav_codes[2]),
		CFG_SIMPLE_INT(	(char *)("spacenav_3"), &spacenav_codes[3]),
		CFG_SIMPLE_INT(	(char *)("spacenav_4"), &spacenav_codes[4]),
		CFG_SIMPLE_INT(	(char *)("spacenav_5"), &spacenav_codes[5]),
		CFG_SIMPLE_STR(	(char *)("spacenav_device"), &spacenav_device),
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
	
	cfg_free(cfg);
	
}
