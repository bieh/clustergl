#include "main.h"
#include <confuse.h>

Config::Config(string filename){

	LOG("Loading configuration from '%s'\n", filename.c_str());
	
	cfg_t *cfg;
	
	//Output options
	static cfg_opt_t axis_opts[] = {
		CFG_INT(	 (char *)("id"), 0, CFGF_NONE),
		CFG_INT(	 (char *)("positive"), 0, CFGF_NONE),
		CFG_INT(	 (char *)("negative"), 0, CFGF_NONE),
		CFG_END()
	};


	//Output options
	static cfg_opt_t button_opts[] = {
		CFG_INT(	 (char *)("id"), 0, CFGF_NONE),
		CFG_INT(	 (char *)("keycode"), 0, CFGF_NONE),
		CFG_END()
	};
	
	//Top level options
	static cfg_opt_t opts[] = {
		CFG_SIMPLE_INT(	(char *)("thresh"), 	&thresh),
		CFG_STR(	 (char *)("device"), 0, CFGF_NONE),
		CFG_SEC(	 (char *)"axis", 	axis_opts, CFGF_MULTI | CFGF_TITLE),
		CFG_END()
	};

	

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

	device = string(cfg_getstr(cfg, "device"));
	thresh = cfg_getint(cfg, "thresh");

	int n = cfg_size(cfg, "axis");
	num_axis = 0;

	for(int i=0;i<MAX_AXIS;i++){
		axis_actions[i][0] = 0;
		axis_actions[i][1] = 0;
	}
	
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
