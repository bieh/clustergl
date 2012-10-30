/*******************************************************************************
 Delta/Framediff module
*******************************************************************************/

#include "main.h"


/*******************************************************************************
 Encode stage
*******************************************************************************/
DeltaEncodeModule::DeltaEncodeModule(){

}

Instruction *DeltaEncodeModule::makeSkip(uint32_t n){
	Instruction *skip = new Instruction();
	skip->id = CGL_REPEAT_INSTRUCTION;
	memcpy(skip->args, &n, sizeof(uint32_t));	
	
	//LOG("Skip %d\n", n);
			
	return skip;
}

bool DeltaEncodeModule::process(vector<Instruction *> *list){
	
	if(lastFrame.size() == 0){
		//this must be the first frame
		mListResult = list;
		lastFrame = *list;
		return true;
	}
		
	vector<Instruction *> *result = new vector<Instruction *>();
		
	int sameCount = 0;
	int totalSkip = 0;
		
	for(int n=0;n<(int)list->size();n++){		
		Instruction *instr = (*list)[n];
		
		if((int)lastFrame.size() > n){		
			Instruction *last = lastFrame[n];
			
			if(!instr->needReply() && instr->compare(last)){
				sameCount++;	
				totalSkip++;			
			}else{
			
				if(sameCount)
					result->push_back(makeSkip(sameCount));
					
				result->push_back(instr);	
				sameCount = 0;
			}
		}else{
			result->push_back(instr);	
		}
	}
	
	if(sameCount){
		result->push_back(makeSkip(sameCount));
	}
			
	lastFrame = *list;
	delete list;
		
	mListResult = result;
	
	//LOG("Delta: %d from %d\n", list->size() - totalSkip, list->size());		
	Stats::count("Delta replace", totalSkip);
		
	return true;
}

//output
vector<Instruction *> *DeltaEncodeModule::resultAsList(){
	return mListResult;
}






/*******************************************************************************
 Decode stage
*******************************************************************************/
DeltaDecodeModule::DeltaDecodeModule(){
	lastFrame.clear();
}

bool DeltaDecodeModule::process(vector<Instruction *> *list){
			
	vector<Instruction *> *result = new vector<Instruction *>();
		
	int instrCount = 0;
	
			
	for(int n=0;n<(int)list->size();n++){		
		Instruction *instr = (*list)[n];
		
		
		if((int)lastFrame.size() > n){
				
			if(instr->id == CGL_REPEAT_INSTRUCTION){
			
				uint32_t *num = (uint32_t *)instr->args;								
				for(int i=0;i<(int)*num;i++){
					Instruction *last = lastFrame[instrCount];
					// we have to copy here because otherwise lru-processing
					// would fail if we just saved another reference to
					// the same instruction ...
					result->push_back(last->copy());
					instrCount++;
				}
			}else{
				// ... but then we _have_ to copy here as well
				// (otherwise we would mess up dynamically created
				// with static referenced objects in one list)
				result->push_back(instr->copy());
				instrCount++;
			}
		}else{
			// ... and here (same reason)
			result->push_back(instr->copy());
			instrCount++;
		}
	}
	
	
	
	//copy into lastFrame
	//This is probably the slowest bit, but we don't have any other option, we
	//need a copy of the frame
	for(int n=0;n<(int)lastFrame.size();n++){
		lastFrame[n]->clear();
		delete lastFrame[n];
	}	
	
	lastFrame.clear();
	for(int n=0;n<(int)result->size();n++){
		Instruction *src = (*result)[n];
		Instruction *dst = src->copy();
						
		lastFrame.push_back(dst);
	}
	
	for(int n=0;n<(int)list->size();n++) {
		Instruction *iter = (*list)[n];
		iter->clear();
	}

	delete list;
			

	mListResult = result;
	
	return true;			
}

//output
vector<Instruction *> *DeltaDecodeModule::resultAsList(){
	return mListResult;
}



/*
//Now send the instructions
	int counter = 0;
	for(std::list<Instruction>::iterator iter = list.begin(), 
	    pIter = (*prevFrame).begin();iter != list.end(); iter++) {								
		Instruction *i = &(*iter);
		
		bool mustSend = false;

		for(int n=0;n<3;n++) {
			if(i->buffers[n].len > 0) {
				//If we expect a buffer back then we must send Instruction
				if(i->buffers[n].needReply) mustSend = true;
			}
		};

		if (useRepeat			 //value from config to enable/disable deltas
			&& i->id == pIter->id//if the instruction has the same id as previous
			&& !mustSend && i->id//mustSend is set when expecting a reply
		//	&& sameCount < 150//stops sameBuffer filling up indefinitely (is this needed?)
			&& i->id != 1499
		) {
			//assume the instruction is the same
			bool same = true;
			//compare all arguments
			for (int a=0;a<MAX_ARG_LEN;a++) {
				if (i->args[a]!=pIter->args[a]) {
					same = false;
					break;
				}
			}

			//if arguments the same, compare all buffers
			if (same) {
				for (int a=0;a<3;a++) {
					if (i->buffers[a].len > 0) {
						if(i->buffers[a].len != pIter->buffers[a].len) {
							same = false;
							break;
						}
						else if (i->buffers[a].needClear != pIter->buffers[a].needClear) {
							same = false;
							break;
						}
						else if (memcmp(i->buffers[a].buffer,pIter->buffers[a].buffer,i->buffers[a].len) != 0) {
							same = false;
							break;
						}
					}
				}
			}

			//if arguments and buffers match, the instruction is identical
			if (same) {
				sameCount++;
				if (pIter != (*prevFrame).end())
					pIter++;
				continue;
			}
		}
		//printf("same count: %d\n", sameCount);
		if (sameCount> 0) {		 // send a count of the duplicates before this instruction
			Instruction * skip = (Instruction *)malloc(sizeof(Instruction));
			if (skip == 0) {
				LOG("ERROR: Out of memory\n");
				exit(-1);
			}
			skip->id = CGL_REPEAT_INSTRUCTION;
			memcpy(skip->args, &sameCount, sizeof(uint32_t));
			//printf("sameCount: %d\n", sameCount);
			for(int i=0;i<3;i++) {
				skip->buffers[i].buffer = NULL;
				skip->buffers[i].len = 0;
				skip->buffers[i].needClear = false;
			}
			if(multicast) {
				netBytes += sizeof(Instruction);
			}
			else {
				netBytes += sizeof(Instruction) * numConnections;
			}
			if(myWrite(skip, sizeof(Instruction))!= sizeof(Instruction)) {
				LOG("Connection problem (didn't send instruction)!\n");
				return false;
			}
			sameCount = 0;		 // reset the count and free the memory
			free(skip);
		}

		// now send the new instruction
		netBytes += sizeof(Instruction) * numConnections;
		if(myWrite(i, sizeof(Instruction)) != sizeof(Instruction)) {
			LOG("Connection problem (didn't send instruction)!\n");
			return false;
		}

		//Now see if we need to send any buffers
		for(int n=0;n<3;n++) {
			int l = i->buffers[n].len;

			if(l > 0) {
				netBytes += l * numConnections;
				if(myWrite(i->buffers[n].buffer, l) != l) {
					LOG("Connection problem (didn't write buffer %d)!\n", l);
					return false;
				}
				//And check if we're expecting a buffer back in response
				if(i->buffers[n].needReply) {

					sendBuffer();
					if(int x = myRead(i->buffers[n].buffer, l) != l) {
						LOG("Connection problem NetClient (didn't recv buffer %d got: %d)!\n", l, x);
						return false;
					}
					//LOG("got buffer back!\n");
				}
			}
		}
		if (pIter != (*prevFrame).end()) pIter++;
		counter++;
	}

	//send any instructions that are remaining in the CGL_REPEAT_INSTRUCTION buffer
	if (sameCount> 0) {			 // send a count of the duplicates before this instruction
		Instruction * skip = (Instruction *)malloc(sizeof(Instruction));
		if (skip == 0) {
			LOG("ERROR: Out of memory\n");
			exit(-1);
		}
		skip->id = CGL_REPEAT_INSTRUCTION;
		skip->args[0] = (uint32_t) sameCount;
		//printf("sameCount: %d\n", sameCount);
		for(int i=0;i<3;i++) {
			skip->buffers[i].buffer = NULL;
			skip->buffers[i].len = 0;
			skip->buffers[i].needClear = false;
		}
		netBytes += sizeof(Instruction) * numConnections;
		if(myWrite(skip, sizeof(Instruction))!= sizeof(Instruction)) {
			LOG("Connection problem (didn't send instruction)!\n");
			return false;
		}
		sameCount = 0;			 // reset the count and free the memory
		free(skip);
	}
	sendBuffer();
*/
