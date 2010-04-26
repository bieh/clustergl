#include "main.h"

bool bRequestShutdown = false;

void requestShutdown(){
	LOG("Shutting down!\n");
	bRequestShutdown = true;
}

int Application::run(vector<string> args){

	if(!init(args)){
		return 1;
	}
	
	while(!bRequestShutdown){
		if(!update()){
			break;
		}
		render();
	}
	
	return shutdown() ? 0 : 1;
}

Application::Application(){

}





int main(int argc, char **argv){
	Application *a = new Application();
	
	vector<string> args;
	for(int i=1;i<argc;i++){
		args.push_back(argv[i]);
	}
	
	int ret = a->run(args);
	
	delete a;
	
	return ret;
}	
