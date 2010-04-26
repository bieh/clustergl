
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
};

/*******************************************************************************
						Transition between slides
*******************************************************************************/
class Transition{
public:
	virtual void init()=0;
	virtual void render()=0;
	virtual bool update()=0;
	virtual void shutdown()=0;
};

class SimpleTransition : public Transition{
public:
	virtual void init()=0;
	virtual void render()=0;
	virtual bool update()=0;
	virtual void shutdown()=0;
};
