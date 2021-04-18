# FIXME -- add a real makefile

all:
	mkdir -p build renders
	gcc -std=c89 -Wall -pedantic -lm -Isrc -Ivendor examples/pulsar.c src/soundfile.c src/interpolation.c src/pippicore.c -o build/pulsar
	./build/pulsar

