#################################
# ClusterGL main makefile
#################################

#Architectecture
UNAME := $(shell uname -s)

#Output filename
NAME= cgl

#files
SOURCES := $(wildcard *.cpp) 
OBJS := $(patsubst %.cpp, %.o, $(SOURCES)) 

#OK. So, we don't want to link mod_app with the standalone app. So we link in
#a dummy one instead. This works, as at runtime we don't use it, it just makes
#the linker happy. In order to do this, we build up two objects lines
OBJS_SHARED := $(patsubst mod_app_dummy.o,,$(OBJS)) 
OBJS_APP := $(patsubst mod_app.o,,$(OBJS)) 
DEPS := $(wildcard include/*.h) 

#Compiler flags. Note the arch-specific ones
CXXFLAGS=-I/opt/local/include/ -Iinclude -O3 -Wextra -g -fPIC -fvisibility-inlines-hidden
LDFLAGS=-lconfuse -llzo2 -lz `sdl-config --cflags --libs` -lSDL_net -lconfuse -lGLEW
DARWIN_LDFLAGS=-framework,Cocoa -framework OpenGL -lintl
LINUX_LDFLAGS=-lGL -lGLU

#Compile
all: $(OBJS) $(NAME)
  
$(NAME): $(OBJS) 
ifeq ($(UNAME),Darwin)
	$(CXX) -o ../runtime/$@-render $(OBJS_APP) $(CXXFLAGS) $(LDFLAGS) $(DARWIN_LDFLAGS)
	$(CXX) -shared -o ../runtime/lib$@-capture.so $(OBJS_SHARED) $(CXXFLAGS) $(LDFLAGS) $(DARWIN_LDFLAGS) 
endif
ifeq ($(UNAME),Linux)
	$(CXX) -o ../runtime/$@-render $(OBJS_APP) $(CXXFLAGS) $(LDFLAGS) $(LINUX_LDFLAGS)
	$(CXX) -shared -o ../runtime/lib$@-capture.so $(OBJS_SHARED) $(CXXFLAGS) $(LDFLAGS) $(LINUX_LDFLAGS)
endif

#Cleanup
clean:
	rm -f *.o
	rm -f ../runtime/$(NAME)
	rm -f ../runtime/lib$(NAME).so.1
