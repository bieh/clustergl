CC = gcc -Wall -ansi

all:
	$(CC) texture.c -o texture -lGL -lGLU `sdl-config --cflags --libs` --std=c99

clean:
	@rm texture -f
