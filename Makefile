# FIXME -- add a real makefile

all:
	mkdir -p build renders
	gcc -std=c89 -Wall -pedantic -lm -Isrc -Ivendor examples/pulsar.c -o build/pulsar
	./build/pulsar

