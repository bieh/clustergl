#------------------------------------------------------#
#			Configuration
#------------------------------------------------------#


#Output filename
NAME= presentation


#files
SOURCES := $(wildcard *.cpp) 
OBJS := $(patsubst %.cpp, %.o, $(SOURCES)) 
DEPS := $(wildcard include/*.h) 

#compiler-type things
CXXFLAGS=-Iinclude -O3 -Wall
LDFLAGS=-L../lib/ -lGL -lGLU `sdl-config --cflags --libs` -lIL -lILU -lILUT -lSDL_ttf -lSDL_net -lconfuse -lGLEW
#CGL libs
#-lCg -lCgGL 

#------------------------------------------------------#
#					Compile
#------------------------------------------------------#
all: $(OBJS) $(NAME)
  
$(NAME): $(OBJS) 
	$(CXX) -o ../runtime/$@ $^ $(CXXFLAGS) $(LDFLAGS)
	
#------------------------------------------------------#
#					Test
#------------------------------------------------------#
run-app:
	 @../runtime/$(NAME)

#------------------------------------------------------#
#					Clean
#------------------------------------------------------#
clean:
	rm -f *.o
	rm -f ../runtime/$(NAME)
