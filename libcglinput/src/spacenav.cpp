/*******************************************************************************
 Space Navigator interface 
*******************************************************************************/

#include "main.h"
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

const int THRESH = 10;

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

    *axis = ev.code;
}

void on_axis_down(int axis){
	mInject->keydown(mConfig->spacenav_codes[axis]);
}

void on_axis_up(int axis){
	mInject->keyup(mConfig->spacenav_codes[axis]);
}

void *spacenav_thread(void *data){
	LOG("Spacenav: opening %s\n", mConfig->spacenav_device);

	int fd = open(mConfig->spacenav_device, O_RDONLY);

	if(fd <= 0){
		LOG("No spacenav found, bailing out\n");
		pthread_exit(NULL);
	}

	int states[10];

	for(int i=0;i<10;i++){
		states[i] = 0;
	}

	while(true){

		for(int i=0;i<6;i++){
			int axis = 0;
			int val = read_axis(fd, &axis);

			LOG("%d: %d", axis, val);

			if(axis < 0){
				continue;
			}

			if(val > THRESH && states[axis] == 0){
				states[axis] = 1;
				on_axis_down(axis);
			}

			if(val < THRESH && states[axis] == 1){
				states[axis] = 0;
				on_axis_up(axis);
			}
		}

		SDL_Delay(100);
	}

 }

 bool begin_spacenav(){

 	//Fire up a background thread to deal with this


	pthread_create(&thread, NULL, spacenav_thread, NULL);


 	return true;
 }