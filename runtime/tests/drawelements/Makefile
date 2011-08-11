CC = gcc -Wall -ansi

all:
	$(CC) drawelements.c -o drawelements -lGL -lGLU `sdl-config --cflags --libs` --std=c99

clean:
	@rm drawelements -f
