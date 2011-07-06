/*******************************************************************************
	ClusterGL - config.cpp
*******************************************************************************/
#include "main.h"

Config *gConfig = NULL;

Config::Config(string filename){
    LOG("Loading configuration from '%s'\n", filename.c_str());
    
    //load values in from config file
	cfg_opt_t opts[] = {
		CFG_SIMPLE_INT((char *)("sizeX"), &sizeX),
		CFG_SIMPLE_INT((char *)("sizeY"), &sizeY),
		CFG_SIMPLE_INT((char *)("offsetX"), &offsetX),
		CFG_SIMPLE_INT((char *)("offsetY"), &offsetY),
		CFG_SIMPLE_INT((char *)("fakeWindowX"), &fakeWindowX),
		CFG_SIMPLE_INT((char *)("fakeWindowY"), &fakeWindowY),
		CFG_SIMPLE_BOOL((char *)("bezelCompensation"), &bezelCompensation),
		CFG_SIMPLE_BOOL((char *)("glFrustumUsage"), &glFrustumUsage),
		CFG_SIMPLE_INT((char *)("port"), &port),
		CFG_SIMPLE_BOOL((char *)("multicast"), &multicast),
		CFG_SIMPLE_STR((char *)("multicastServer"), &multicastServer),
		CFG_FLOAT((char *)("scaleX"), 1.0f, CFGF_NONE),
		CFG_FLOAT((char *)("scaleY"), 1.0f, CFGF_NONE),
		CFG_SIMPLE_INT((char *)("syncRate"), &syncRate),
		CFG_SIMPLE_INT((char *)("compressingMethod"), &compressingMethod),
		CFG_SIMPLE_BOOL((char *)("usingSendCompression"), &usingSendCompression),
		CFG_SIMPLE_BOOL((char *)("usingReplyCompression"), &usingReplyCompression),
		CFG_SIMPLE_BOOL((char *)("useCGLRepeat"), &useRepeat),
		CFG_END()
	};
	cfg_t *cfg;

	cfg = cfg_init(opts, CFGF_NONE);
	cfg_parse(cfg, "config.conf");

	//process floats in the config file
	scaleX = cfg_getfloat(cfg,(char *) "scaleX");
	scaleY = cfg_getfloat(cfg,(char *) "scaleY");
	cfg_free(cfg);
}
