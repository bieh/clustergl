#include "main.h"
#include <zconf.h>
#include <zlib.h>
#include <lzo/lzo1b.h>

#define lzo true

/*********************************************************
	Module Stuff
*********************************************************/

NetCompressModule::NetCompressModule(int level) {
	compressLevel = level;
	if (lzo_init() != LZO_E_OK)
	{
	    printf("Uh oh! LZO init failed!\n");
	}

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
	uLongf CompBuffSize = (uLongf)(nByte + (nByte/1024 * 16 ) + 16);
	unsigned char * workingMemory = (unsigned char*)malloc(LZO1B_MEM_COMPRESS);
	int ret = 0;
	if(nByte > 4)
	{	
		
		#ifndef lzo
			ret = compress2((Bytef *) output, &CompBuffSize, (Bytef *) input, nByte, compressLevel);
		#else
			ret = lzo1b_compress((Bytef *) input, nByte, (Bytef *) output, &CompBuffSize, workingMemory, compressLevel);
		#endif
		if(ret != Z_OK)
		{
			if(ret == Z_MEM_ERROR)
				LOG("ERROR compressing: memory error\n");
			else if(ret == Z_BUF_ERROR)
				LOG("ERROR compressing: buffer error\n");
			else if(ret == Z_STREAM_ERROR)
				LOG("ERROR compressing: compressLevel not (1-9), %d\n", compressLevel);
		}
	}
	else
	{
	memcpy(output, input, nByte);
	CompBuffSize = nByte;
	}
	free(workingMemory);
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
		#ifndef lzo
		ret = uncompress((Bytef*) dest, (uLongf*) &newDest, (const Bytef*)source, newSource);
		#else
		ret = lzo1b_decompress((const Bytef*)source, newSource, (Bytef*) dest, &newDest, NULL);
		#endif
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
