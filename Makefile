.PHONY: all wavetables

default: all

ALLSOURCES = src/sineosc.c \
	src/pulsar.c \
	src/soundfile.c \
	src/window.c \
	src/wavetable.c \
	src/pippicore.c

wavetables:
	./scripts/build_static_wavetables.sh

all:
	mkdir -p build renders

	echo "Building pulsar.c example...";
	gcc -std=c89 -Wall -pedantic -lm -Isrc -Ivendor examples/pulsar.c $(ALLSOURCES) -o build/pulsar
	
	echo "Building sineosc.c example...";
	gcc -std=c89 -Wall -pedantic -lm -Isrc -Ivendor examples/sineosc.c $(ALLSOURCES) -o build/sineosc

	echo "Building memory_pool.c example...";
	gcc -std=c89 -Wall -pedantic -lm -Isrc -Ivendor -DLP_FLOAT -DLP_STATIC examples/memory_pool.c src/sineosc.c src/soundfile.c src/window.c src/pippicore.c -o build/memorypool

	echo "Rendering examples..."
	./scripts/render_examples.sh
