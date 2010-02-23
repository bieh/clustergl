/********************************************************
	Headers
********************************************************/

#include "main.h"

#include <zconf.h>
#include <zlib.h>
#include <lzo/lzo1b.h>
#include <lzo/lzo1x.h>

/********************************************************
	Main Globals (Loaded from config file)
********************************************************/

/* 
  Compression methods:
  1 = ZLib, best compression, but much slower
  2 = LZO1b, best for large blocks, with redundant data
  3 = LZO1x, best for most applications
*/

extern int compressingMethod;

/*********************************************************
	Module Stuff
*********************************************************/

NetCompressModule::NetCompressModule()
{
	if(compressingMethod == 2 || compressingMethod == 3) {
		if (lzo_init() != LZO_E_OK) {
			printf("LZO init failed!\n");
		}
	}
}


bool NetCompressModule::process(list<Instruction> &i)
{
	LOG("NetCompressModule::process: Shouldn't happen!\n");
}


void NetCompressModule::reply(Instruction *instr, int i)
{
	LOG("NetCompressModule::reply: Shouldn't happen!\n");
}


bool NetCompressModule::sync()
{
	LOG("NetCompressModule::sync: Shouldn't happen!\n");
}


/*********************************************************
	Compress Method
*********************************************************/

int NetCompressModule::myCompress(void *input, int nByte, void *output)
{
	uLongf CompBuffSize = 0;
	if(compressingMethod == 1)
		CompBuffSize = (uLongf)(nByte + (nByte * 0.1) + 12);
	else
		CompBuffSize = (uLongf)(nByte + (nByte/1024 * 16 ) + 16);

	unsigned char * workingMemory = NULL;
	if(compressingMethod == 2)
		workingMemory = (unsigned char*)malloc(LZO1B_MEM_COMPRESS);
	else if(compressingMethod == 3)
		workingMemory = (unsigned char*)malloc(LZO1X_1_15_MEM_COMPRESS);
	int ret = 0;
	if(nByte > 4) {
		int compressLevel = 1;
		if(compressingMethod == 1)
			ret = compress2((Bytef *) output, &CompBuffSize, (Bytef *) input, nByte, compressLevel);
		else if(compressingMethod == 2)
			ret = lzo1b_compress((Bytef *) input, nByte, (Bytef *) output, &CompBuffSize, workingMemory, compressLevel);
		else if(compressingMethod == 3)
			ret = lzo1x_1_15_compress((Bytef *) input, nByte, (Bytef *) output, &CompBuffSize, workingMemory);

		if(compressingMethod == 1) {
			if(ret != Z_OK) {
				if(ret == Z_MEM_ERROR)
					LOG("ERROR compressing: memory error\n");
				else if(ret == Z_BUF_ERROR)
					LOG("ERROR compressing: buffer error\n");
				else if(ret == Z_STREAM_ERROR)
					LOG("ERROR compressing: compressLevel not (1-9), %d\n", compressLevel);
			}
		}
	}
	else {
		memcpy(output, input, nByte);
		CompBuffSize = nByte;
	}
	if(compressingMethod == 2 || compressingMethod == 3)
		free(workingMemory);
	return CompBuffSize;
}


/*********************************************************
	Decompress Method
*********************************************************/

int NetCompressModule::myDecompress(void *dest, int destLen, void *source, int sourceLen)
{
	uLongf newSource = sourceLen;
	uLongf newDest = destLen;
	int ret = 0;
	if(sourceLen > 4) {
		if(compressingMethod == 1)
			ret = uncompress((Bytef*) dest, (uLongf*) &newDest, (const Bytef*)source, newSource);
		else if(compressingMethod == 2)
			ret = lzo1b_decompress((const Bytef*)source, newSource, (Bytef*) dest, &newDest, NULL);
		else if(compressingMethod == 3)
			ret = lzo1x_decompress((const Bytef*)source, newSource, (Bytef*) dest, &newDest, NULL);

		if(compressingMethod == 1) {
			if(ret != Z_OK) {
				if(ret == Z_MEM_ERROR)
					LOG("ERROR decompressing: memory error\n");
				else if(ret == Z_BUF_ERROR)
					LOG("ERROR decompressing: buffer error\n");
				else if(ret == Z_DATA_ERROR)
					LOG("ERROR decompressing: data error,\n");
			}
		}
	}
	else {
		memcpy(dest, source, sourceLen);
	}
	return ret;
}
