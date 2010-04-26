//loading flags
#define TEXTURE_NO_GL 1

class Texture{
	GLuint mHandle;
	ILuint iDevilID;
	int iSizeX, iSizeY;	
	bool bIsLoaded;
public:
	Texture(string filename);
	~Texture(){destroy();}
	
	bool load(string filename, int flags);	
	void bind();
	void destroy();
	
	bool isLoaded(){ return bIsLoaded; }
};
