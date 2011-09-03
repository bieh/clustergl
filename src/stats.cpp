#include "main.h"

#define INTERVAL 1000

std::map<string, int> mCounts;
std::map<string, int> mIncrements;
int lastOutputTime = 0;
int frames = 0;

void Stats::count(string key, int count){

	if(mCounts.find(key) != mCounts.end()){
		mCounts[key] += count;
	}else{
		mCounts[key] = count;
	}
}

void Stats::increment(string key, int count){

	if(mIncrements.find(key) != mIncrements.end()){
		mIncrements[key] += count;
	}else{
		mIncrements[key] = count;
	}
}

void Stats::update(){
	int time = SDL_GetTicks();
	frames++;
	
	increment("ticks");
	
	if(time - lastOutputTime > INTERVAL){
		output();
		lastOutputTime = time;
		frames = 0;
		mCounts.clear();
		mIncrements.clear();
	}	
}

void Stats::output(){

	printf("\n********************************************\n");
	
	printf("Total over the last %dms:\n", INTERVAL);	
	for (map<string, int>::iterator i = mIncrements.begin(); i != mIncrements.end(); i++){
		int val = i->second;		
		printf(" %s: %d\n", i->first.c_str(), val);
	}	
	
	printf("\nAverage per tick over the last %dms:\n", INTERVAL);	
	for (map<string, int>::iterator i = mCounts.begin(); i != mCounts.end(); i++){
		int val = i->second;		
		val /= frames;
		printf(" %s: %d\n", i->first.c_str(), val);
	}	
	printf("********************************************\n");
}
