
/*******************************************************************************
						Holds slides
*******************************************************************************/
class Transition{
public:
	virtual bool init()=0;
	virtual bool render()=0;
};

class HitInFace : public Transition{

	float fZoom;
	float fZoomVel;
	float x;
	float xVel;
	float xTarget;
public:
	bool init();
	bool render();
};


/*******************************************************************************
						individual slide
*******************************************************************************/
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
						Controls slides
*******************************************************************************/
class Presentation{
public:
	bool init(vector<string> files);
	void render();
	void render2D();
	void update();
	void shutdown();
	
	//controls
	void next();
	void prev();
	void start();
	
	//slide getters
	Slide *getNextSlide();
	Slide *getCurrentSlide();
	Slide *getPrevSlide();
	
	//transitions
	void doSimpleTransition(bool init);
	
	bool isTransition;
	bool transitionInit;
	
	vector<Transition *> mTransitions;
	Transition *mCurrentTransition;
	
	bool allowCache;
};

//hacky - these are globals so transitions can access em
extern vector<Slide *> mSlides;
extern int iCurrentSlide;
	

