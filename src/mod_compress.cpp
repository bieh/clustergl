#include "main.h"
#include <zconf.h>
#include <zlib.h>

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


int NetCompressModule::myCompress(void *input, int nByte, void *output){
	uLongf CompBuffSize = (uLongf)(nByte + (nByte * 0.1) + 12);
	if(nByte > 4)
	{
		int ret = compress((Bytef *) output, &CompBuffSize, (Bytef *) input, nByte);
		if(ret != Z_OK)
		{
			if(ret == Z_MEM_ERROR)
				LOG("\t\tERROR compressing: memory\n");
			else if(ret == Z_BUF_ERROR)
				LOG("\t\tERROR compressing: buffer\n");
		}
	}
	else
	{
	memcpy(output, input, 4);
	CompBuffSize = nByte;
	}
	//LOG("incoming: %d bytes, outgoing %d bytes\n", nByte, CompBuffSize);
	return CompBuffSize;
}

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
				LOG("error decompressing: memory\n");
			else if(ret == Z_BUF_ERROR)
				LOG("error decompressing: buffer\n");
			else if(ret == Z_DATA_ERROR)
				LOG("error decompressing: data\n");
		}
	}
	else
	{
		memcpy(dest, source, 4);
	}
	return ret;
}
