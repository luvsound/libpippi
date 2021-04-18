# FIXME -- add a real makefile

all:
	mkdir -p build renders
	gcc -std=c89 -Wall -pedantic -lm -Isrc -Ivendor examples/pulsar.c src/soundfiles.c src/pippicore.c -o build/pulsar
	./build/pulsar

