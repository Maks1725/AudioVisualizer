audiovisualizer: main.c
	gcc -o audiovisualizer main.c miniaudio.h -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
