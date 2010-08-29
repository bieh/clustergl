class Transition;

class Slide{
public:
    Slide();

    Texture *mThumb;
    Texture *mFull;

    string mFullFilename;
    string mThumbFilename;

    bool loadFull();
};

/*******************************************************************************
						Holds slides
*******************************************************************************/
class Presentation{
public:
	bool init(vector<string> files);
	void render();
	void render2D();
	void update();
	void shutdown();
	
	vector<Slide *> mSlides;
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

