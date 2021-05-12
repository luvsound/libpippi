.PHONY: all wavetables

default: all

ALLSOURCES = src/oscs.sine.c \
	src/oscs.pulsar.c \
	src/soundfile.c \
	src/window.c \
	src/wavetable.c \
	src/pippicore.c

wavetables:
	./scripts/build_static_wavetables.sh

all:
	mkdir -p build renders

	echo "Building pulsarosc.c example...";
	gcc -std=c89 -Wall -pedantic -lm -Isrc -Ivendor examples/pulsarosc.c $(ALLSOURCES) -o build/pulsarosc
	
	echo "Building sineosc.c example...";
	gcc -std=c89 -Wall -pedantic -lm -Isrc -Ivendor examples/sineosc.c $(ALLSOURCES) -o build/sineosc

	echo "Building memory_pool.c example...";
	gcc -std=c89 -Wall -pedantic -lm -Isrc -Ivendor -DLP_FLOAT -DLP_STATIC examples/memory_pool.c src/oscs.sine.c src/soundfile.c src/window.c src/pippicore.c -o build/memorypool

	echo "Rendering examples..."
	./scripts/render_examples.sh
