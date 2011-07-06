/*******************************************************************************
	Runtime configuration
*******************************************************************************/
class Config{
public:
    Config(string filename, string id);
        
/*******************************************************************************
	Rendering output config
*******************************************************************************/
	
	//The id of this node. We look for a config file section with this name
	//id is set on the command line on startup
	string id;

	//Size of this renderer
    int sizeX;
    int sizeY;
    
    //The position of this renderer inside the total screen size
    int offsetX;
    int offsetY;
                
    //Size of the entire viewport
    int totalWidth;
    int totalHeight;
        
    int screenWidth;
    int screenGap;
    
    int syncRate;
    bool enableStats;
    
    float scaleX;
    float scaleY;
    
    int clientPort;
    
    bool enableWindowPositioning;
    
/*******************************************************************************
	Capture configuration
*******************************************************************************/
	int serverPort;
	
    //Size of the client window    
    int fakeWindowX;
    int fakeWindowY;
	    
    //Location of renderers. Automatically calculated
    int numOutputs;    
	vector<string> outputAddresses;
	vector<int> outputPorts;
	
};

/*
main.h:const float SYMPHONY_SCREEN_WIDTH = 1680.0;
main.h:const float SYMPHONY_SCREEN_TOTAL_WIDTH = 8880.0;
main.h:const float SYMPHONY_SCREEN_TOTAL_HEIGHT = 4560.0;
main.h:const float SYMPHONY_SCREEN_GAP = 120.0;
*/
