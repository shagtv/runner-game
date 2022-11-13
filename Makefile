all:
	gcc -I/usr/local/include/SDL2/ src/*.c -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -o build/game

clean:
	rm build/*
