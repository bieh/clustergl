#------------------------------------------------------#
#			Configuration
#------------------------------------------------------#


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

#compiler-type things
CXXFLAGS=-Iinclude -O3 -W -Wextra -g -pg -fPIC
LDFLAGS=-lGL -lGLU `sdl-config --cflags --libs` -lSDL_net -lconfuse -lGLEW
#CGL libs
#-lCg -lCgGL 

#------------------------------------------------------#
#					Compile
#------------------------------------------------------#
all: $(OBJS) $(NAME)
  
$(NAME): $(OBJS) 
	$(CXX) -o ../runtime/$@ $(OBJS_APP) $(CXXFLAGS) $(LDFLAGS)
	$(CXX) -o ../runtime/lib$@.so $(OBJS_SHARED) $(CXXFLAGS) $(LDFLAGS) -shared 
	
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
	rm -f ../runtime/lib$(NAME).so
