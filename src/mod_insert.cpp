/********************************************************
	Headers
********************************************************/

#include "main.h"

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

/*********************************************************
	Insertion Globals
*********************************************************/

Instruction mInst[100];
//the number of instructions used
int instCount = 0;
//current instruction
Instruction *mCurrInst = NULL;
byte *mArgs = NULL;

/*********************************************************
	Interception module stuff
*********************************************************/
InsertModule::InsertModule(){
	netBytes= 0;
	netBytes2 = 0;
	frames = 0;
	init();
}

bool InsertModule::init(){
	/* create the list we are going to add to each frame */
	return true;
}

bool InsertModule::process(list<Instruction> &list){
	Instruction lastInstruction = list.back();
	if(lastInstruction.id == 1499) {//i.e. swap buffers

		//remove swap buffers
		list.pop_back();
		
		//add our instructions
		for(int i=0;i<instCount;i++){
			list.push_back(mInst[i]);
		}
		
		//add swap buffers
		list.push_back(lastInstruction);
	}
	
	return true;
}

void InsertModule::reply(Instruction *instr, int i){
	LOG("InsertModule::reply: Shouldn't happen!\n");
}

bool InsertModule::sync(){
	return true;
}

