# FIXME -- add a real makefile

all:
	mkdir -p build renders
	#gcc -g -O0 -lm examples/pulsar.c -o build/pulsar
	gcc -std=c89 -Wall -pedantic -lm -Isrc examples/pulsar.c -o build/pulsar
	./build/pulsar
	sox -r 44100 -e float -b 64 -c 2 renders/pulsar-out.raw renders/pulsar-out.wav
	play renders/pulsar-out.wav

