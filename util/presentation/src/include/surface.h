class Surface{
	
	int videoFlags;		
	SDL_Surface *surface;
	
	void resizeWindow( int width, int height );
		
public:
		
	bool init(int w, int h, bool f);
	void beginRender();
	void begin2D();
	void endRender();
	bool shutdown();
	
	void onResize(int x, int y);
};
