#include "main.h"
#include <zconf.h>
#include <zlib.h>

/*********************************************************
	Module Stuff
*********************************************************/

NetCompressModule::NetCompressModule(){
}

bool NetCompressModule::process(list<Instruction> &i){
	LOG("NetCompressModule::process: Shouldn't happen!\n");
}

void NetCompressModule::reply(Instruction *instr, int i){
	LOG("NetCompressModule::reply: Shouldn't happen!\n");
}

bool NetCompressModule::sync(){
	LOG("NetCompressModule::sync: Shouldn't happen!\n");
}


/*********************************************************
	Compress Method
*********************************************************/

int NetCompressModule::myCompress(void *input, int nByte, void *output){
	uLongf CompBuffSize = (uLongf)(nByte + (nByte * 0.1) + 12);
	if(nByte > 4)
	{
		int ret = compress((Bytef *) output, &CompBuffSize, (Bytef *) input, nByte);
		if(ret != Z_OK)
		{
			if(ret == Z_MEM_ERROR)
				LOG("ERROR compressing: memory error\n");
			else if(ret == Z_BUF_ERROR)
				LOG("ERROR compressing: buffer error\n");
		}
	}
	else
	{
	memcpy(output, input, nByte);
	CompBuffSize = nByte;
	}
	return CompBuffSize;
}

/*********************************************************
	Decompress Method
*********************************************************/

int NetCompressModule::myDecompress(void *dest, int destLen, void *source, int sourceLen){
	uLongf newSource = sourceLen;
	uLongf newDest = destLen;
	int ret = 0;		
	if(sourceLen > 4)
	{
		ret = uncompress((Bytef*) dest, (uLongf*) &newDest, (const Bytef*)source, newSource);
		if(ret != Z_OK)
		{
			if(ret == Z_MEM_ERROR)
				LOG("ERROR decompressing: memory error\n");
			else if(ret == Z_BUF_ERROR)
				LOG("ERROR decompressing: buffer error\n");
			else if(ret == Z_DATA_ERROR)
				LOG("ERROR decompressing: data error,\n");
		}
	}
	else
	{
		memcpy(dest, source, sourceLen);
	}
	return ret;
}
