/*******************************************************************************
 Duplicate buffer module
*******************************************************************************/

#include "main.h"

#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

#define MAX_LRU 8192
#define MIN_BUF_SIZE 128

uint32_t hash(byte *data, int len);

/*******************************************************************************
 Encode stage
*******************************************************************************/
DuplicateBufferEncodeModule::DuplicateBufferEncodeModule(){
	mLRU = new lru_cache(MAX_LRU);
}


bool DuplicateBufferEncodeModule::process(vector<Instruction *> *list){
	
	for(int n=0;n<(int)list->size();n++){		
		Instruction *instr = (*list)[n];
		
		for(int i=0;i<3;i++){
			InstructionBuffer *buf = &instr->buffers[i];
			
			if(!buf->buffer || buf->needReply || buf->len < MIN_BUF_SIZE){
				buf->hash = 0;
				continue;
			}
			
			int h = hash(buf->buffer, buf->len);
						
			buf->hash = h;
			buf->hashlen = buf->len;
			
			if(mLRU->exists(h)){
			
				//LOG("Encoding: already had hash %d, freeing buffer (%d)\n", h, buf->len);
			
				if(buf->needClear){
					free(buf->buffer);
				}
			
				buf->buffer = NULL;
				buf->len = 0;
				buf->needClear = false;
			}else{
				//LOG("Encoding: adding hash %d!\n", h);
				mLRU->insert(h,0); //we don't need to copy the actual bytes
			}	
		}
	}
		
	mListResult = list;
	
	//LOG("Delta: %d from %d\n", list->size() - totalSkip, list->size());		
			
	return true;
}

//output
vector<Instruction *> *DuplicateBufferEncodeModule::resultAsList(){
	return mListResult;
}





/*******************************************************************************
 Decode stage
*******************************************************************************/
DuplicateBufferDecodeModule::DuplicateBufferDecodeModule(){
	mLRU = new lru_cache(MAX_LRU);
} 

bool DuplicateBufferDecodeModule::process(vector<Instruction *> *list){

	int hitCount = 0;
	int missCount = 0; 
				
	for(int n=0;n<(int)list->size();n++){		
		Instruction *instr = (*list)[n];
		
		for(int i=0;i<3;i++){
			InstructionBuffer *buf = &instr->buffers[i];
			
			if(buf->buffer && buf->hash){
				//we've got data /and/ a hash. Enter it into the LRU
				byte *copy = (byte *)malloc(buf->len);
				memcpy(copy, buf->buffer, buf->len);
				
				mLRU->insert(buf->hash,copy);
				
				missCount++;
				
				//LOG("Added %d to LRU (%d)\n", buf->hash, buf->len);
				continue;
			}
			
			else if(!buf->buffer && buf->hash){
				
				//No data, but we have a hash. Pull the data from the LRU
				if(!mLRU->exists(buf->hash)){
					LOG("ERROR: decode side didn't have hash %d in LRU\n", buf->hash);
					return false;
				}
			
				buf->buffer = mLRU->fetch(buf->hash);
				buf->len = buf->hashlen;
				buf->needClear = false; //we'll clean up our own mess
				
				//LOG("Read %d from LRU (%d)\n", buf->hash, buf->hashlen);
				hitCount++;
			}
		}
	}
	
	//LOG("Hit: %d, Miss: %d\n", hitCount, missCount);
	
	Stats::count("mod_duplicatebuffer hits", hitCount);
	Stats::count("mod_duplicatebuffer misses", missCount);

	mListResult = list;
		
	return true;			
}

//output
vector<Instruction *> *DuplicateBufferDecodeModule::resultAsList(){
	return mListResult;
}



//http://www.azillionmonkeys.com/qed/hash.html
uint32_t hash(byte *data, int len){
	
	//Makes it rather a lot faster...if you don't mind inaccuracies
	//if(len > 64){
	//	return hash(data, 32) + hash(data, len-32);
	//}

	uint32_t hash = len, tmp;
	int rem;

    if (len <= 0 || data == NULL) return 0;

    rem = len & 3;
    len >>= 2;

    /* Main loop */
    for (;len > 0; len--) {
        hash  += get16bits (data);
        tmp    = (get16bits (data+2) << 11) ^ hash;
        hash   = (hash << 16) ^ tmp;
        data  += 2*sizeof (uint16_t);
        hash  += hash >> 11;
    }

    /* Handle end cases */
    switch (rem) {
        case 3: hash += get16bits (data);
                hash ^= hash << 16;
                hash ^= data[sizeof (uint16_t)] << 18;
                hash += hash >> 11;
                break;
        case 2: hash += get16bits (data);
                hash ^= hash << 11;
                hash += hash >> 17;
                break;
        case 1: hash += *data;
                hash ^= hash << 10;
                hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}



