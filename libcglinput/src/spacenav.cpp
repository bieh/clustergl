/*******************************************************************************
 Space Navigator interface 
*******************************************************************************/

#include "main.h"

///We disable spacenav on non-linux, as we used linux-specific headers below
#ifndef __linux__

 bool begin_spacenav(){
 	LOG("No spacenav on OSX yet!\n");
 	return true;
 }

#else


#include <pthread.h>
#include <sys/ioctl.h>
#include <error.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <arpa/inet.h>

#include <SDL/SDL.h>

pthread_t thread;


/*
//write a single axis input_event
void write_axis(int axid, float amount){
    struct input_event ev;

    ev.code = axid;
    ev.value = amount;
    ev.type = EV_REL;
    
    gettimeofday(&ev.time, NULL);

    write(fd, &ev, sizeof(ev));
}

void update_dev_file(){
    write_axis(0, x);
    write_axis(1, y);
    write_axis(2, z);
    write_axis(3, pitch);
    write_axis(4, roll);
    write_axis(5, yaw);
}*/

int read_axis(int fd, int *axis){
	struct input_event ev;

    int r = read(fd, &ev, sizeof(ev));

    if(r == 0){
    	*axis = -1;
    	return 0;
    }
    
    //printf("%d, %d, %d\n", ev.code, ev.type, ev.value);

    *axis = ev.code;

    return ev.value;
}

void on_axis_down(int axis, int dir){
	if(mConfig->axis_actions[axis][dir]){
		LOG("AXIS ACTIVE %d %s %d\n", axis, dir ? "up" : "down", mConfig->axis_actions[axis][dir]);
		mInject->keydown(mConfig->axis_actions[axis][dir]);
	}
}

void on_axis_up(int axis, int dir){
	if(mConfig->axis_actions[axis][dir]){
		LOG("AXIS INACTIVE %d %s %d\n", axis, dir ? "up" : "down", mConfig->axis_actions[axis][dir]);
		mInject->keyup(mConfig->axis_actions[axis][dir]);
	}
}

void *spacenav_thread(void *data){
	LOG("Input device: attemting to open %s\n", mConfig->device.c_str());

	int fd = open(mConfig->device.c_str(), O_RDONLY);

	if(fd <= 0){
		LOG("No input device found\n");
		pthread_exit(NULL);
	}

	int states[MAX_AXIS];

	for(int i=0;i<MAX_AXIS;i++){
		states[i] = 0;
	}
	

	while(true){

		int axis = 0;
		int val = read_axis(fd, &axis);

		//LOG("%d: %d\n", axis, val);

		if(axis < 0 || axis >= MAX_AXIS){
			continue;
		}
		
		if(val > 1024){
			continue; //TODO: better solution!
		}
		
		int thresh = mConfig->thresh[axis];

		//Positive
		if(val >= thresh && states[axis] == 0){
			states[axis] = 1;
			on_axis_down(axis, 0);
		}

		if(val < thresh && states[axis] == 1){
			states[axis] = 0;
			on_axis_up(axis, 0);
		}


		//Negative
		if(val <= -thresh && states[axis] == 0){
			states[axis] = -1;
			on_axis_down(axis, 1);
		}

		if(val > -thresh && states[axis] == -1){
			states[axis] = 0;
			on_axis_up(axis, 1);
		}
		
		SDL_Delay(1);

	}

 }

 bool begin_spacenav(){

 	//Fire up a background thread to deal with this
	pthread_create(&thread, NULL, spacenav_thread, NULL);


 	return true;
 }

 #endif
