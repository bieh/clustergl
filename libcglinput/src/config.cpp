#include "main.h"
#include <confuse.h>

Config::Config(string filename){

	LOG("Loading configuration from '%s'\n", filename.c_str());
	
	cfg_t *cfg;

	cfg = cfg_init(NULL, CFGF_NONE);
	
	int parse_result = cfg_parse(cfg, filename.c_str());
	
	if(parse_result == CFG_FILE_ERROR){
		LOG("Error with the file (does it exist?)\n");
		exit(1);
	}
	
	if(parse_result == CFG_PARSE_ERROR){
		LOG("Couldn't parse config file\n");
		exit(1);
	}

	device = string(cfg_getstr(cfg, "device"));
	thresh = cfg_getint(cfg, "thresh");

	int n = cfg_size(cfg, "axis");
	num_axis = 0;
	
	for(int i=0;i<n;i++){
		cfg_t *o = cfg_getnsec(cfg, "axis", i);
	
		num_axis++;

		int id = cfg_getint(o, "id");
		int pos = cfg_getint(o, "positive");
		int neg = cfg_getint(o, "negative");
	
		if(id >= MAX_AXIS){
			LOG("Axis id %d outside of range %d\n", id, MAX_AXIS);
			continue;
		}

		axis_actions[id][0] = pos;
		axis_actions[id][1] = neg;
	}
	

	
	cfg_free(cfg);
	
}
