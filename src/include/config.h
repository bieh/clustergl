/*******************************************************************************
	Compile-time configuration
*******************************************************************************/

#define CGL_REPEAT_INSTRUCTION 1498
#define MAX_INSTRUCTIONS 500000

//Quick LOG hack that we can make a proper log system out of later
#define LOG printf

/*******************************************************************************
	Runtime configuration
*******************************************************************************/
class Config{
public:
    int sizeX;
    int sizeY;
    int offsetX;
    int offsetY;
    float scaleX;
    float scaleY;
    int fakeWindowX;
    int fakeWindowY;
    bool glFrustumUsage;
    bool bezelCompensation;

    /* connection */
    //string addresses[5];
    int port;
    bool multicast;
    char * multicastServer;

    /* clusterGL configs */
    int syncRate;
    int compressingMethod;
    bool usingSendCompression;
    bool usingReplyCompression;
    bool useRepeat;
    
    bool enableProfile;
    bool enableStats;
    
    Config(string filename);
};
