class Transition;

/*******************************************************************************
						Holds slides
*******************************************************************************/
class Presentation{
public:
	bool init(vector<string> files);
	void render();
	void update();
	void shutdown();
	
	vector<Texture *> mSlides;
	int iCurrentSlide;
	
	//controls
	void next();
	void prev();
	void start();
	
	//transitions
	void doSimpleTransition(bool init);
	
	bool isTransition;
	bool transitionInit;
};

