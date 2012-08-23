
#Architectecture
UNAME := $(shell uname -s)

#Output filename
NAME= cglinput

#files
SOURCES := $(wildcard src/*.cpp) 
OBJS := $(patsubst %.cpp, %.o, $(SOURCES)) 

OBJS_APP := $(patsubst mod_app.o,,$(OBJS)) 
DEPS := $(wildcard include/*.h) 

#Compiler flags. Note the arch-specific ones
CXXFLAGS=$(ARCH) -I/opt/local/include/ -Isrc/include -O3 -Wextra -g -fPIC -fvisibility-inlines-hidden 
LDFLAGS=-lconfuse -lpthread
DARWIN_LDFLAGS= -lintl -L/opt/local/lib -lSDL
LINUX_LDFLAGS=-lSDL

#Compile
all: testprog $(OBJS) $(NAME)
  
$(NAME): $(OBJS) 
ifeq ($(UNAME),Darwin)
	$(CXX) -shared -o ./lib$@.so $(OBJS) $(CXXFLAGS) $(LDFLAGS) $(DARWIN_LDFLAGS) 
endif
ifeq ($(UNAME),Linux)
	$(CXX) -shared -o ./lib$@.so $(OBJS) $(CXXFLAGS) $(LDFLAGS) $(LINUX_LDFLAGS)
endif

testprog:
ifeq ($(UNAME),Darwin)
	gcc --std=c99 -g -I/opt/local/include -I/usr/X11R6/include -L/opt/local/lib -lSDLmain -lSDL -Wl,-framework,Cocoa -framework OpenGL lesson05.c -o lesson05
endif
ifeq ($(UNAME),Linux)
	gcc --std=c99 -g `sdl-config --cflags --libs`  lesson05.c -o lesson05 -lGL -lGLU -lSDL
endif

#Cleanup
clean:
	rm -f src/*.o
	rm -f ./lib$(NAME).so
	rm -f lesson05
