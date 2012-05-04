CC = gcc -Wall -ansi
UNAME := $(shell uname -s)
FILE=drawelements

all: 
ifeq ($(UNAME),Darwin)
	gcc -g -I/opt/local/include -I/usr/X11R6/include -L/opt/local/lib -lSDLmain -lSDL -Wl,-framework,Cocoa -framework OpenGL $(FILE).c -o $(FILE)
endif
ifeq ($(UNAME),Linux)
	$(CC) $(FILE).c -o $(FILE) -lGL -lGLU `sdl-config --cflags --libs`  -L/usr/X11/lib/ -lSDLmain --std=c99
endif
clean:
	@rm $(FILE) -f
	

