.PHONY: all wavetables

default: all

ALLSOURCES = src/oscs.sine.c \
	src/oscs.pulsar.c \
	src/oscs.tape.c \
	src/microsound.c \
	src/mir.c \
	src/soundfile.c \
	src/scheduler.c \
	src/pippicore.c

wavetables:
	./scripts/build_static_wavetables.sh

all:
	mkdir -p build renders

	echo "Building pulsarosc.c example...";
	gcc -g -std=c89 -Wall -pedantic -lm -Isrc -Ivendor examples/pulsarosc.c $(ALLSOURCES) -o build/pulsarosc
	
	echo "Building sineosc.c example...";
	gcc -g -std=c89 -Wall -pedantic -lm -Isrc -Ivendor examples/sineosc.c $(ALLSOURCES) -o build/sineosc

	echo "Building ring_buffer.c example...";
	gcc -g -std=c89 -Wall -pedantic -lm -Isrc -Ivendor examples/ring_buffer.c $(ALLSOURCES) -o build/ring_buffer

	echo "Building onset_detector.c example...";
	gcc -g -std=c89 -Wall -pedantic -lm -Isrc -Ivendor examples/onset_detector.c $(ALLSOURCES) -o build/onset_detector

	echo "Building pitch_tracker.c example...";
	gcc -gdwarf -std=c89 -Wall -pedantic -lm -Isrc -Ivendor examples/pitch_tracker.c $(ALLSOURCES) -o build/pitch_tracker

	echo "Building memory_pool.c example...";
	gcc -g -std=c89 -Wall -pedantic -lm -Isrc -Ivendor -DLP_FLOAT -DLP_STATIC examples/memory_pool.c src/oscs.sine.c src/soundfile.c src/pippicore.c -o build/memorypool

	echo "Building scheduler.c example...";
	gcc -g -std=c89 -Wall -pedantic -lm -Isrc -Ivendor examples/scheduler.c $(ALLSOURCES) -o build/scheduler

	echo "Building additive_synthesis.c example...";
	gcc -g -std=c89 -Wall -pedantic -lm -Isrc -Ivendor examples/additive_synthesis.c $(ALLSOURCES) -o build/additive_synthesis

	echo "Building graincloud.c example...";
	gcc -g -std=c89 -Wall -pedantic -lm -Isrc -Ivendor examples/graincloud.c $(ALLSOURCES) -o build/graincloud

	echo "Building tapeosc.c example...";
	gcc -g -std=c89 -Wall -pedantic -lm -Isrc -Ivendor examples/tapeosc.c $(ALLSOURCES) -o build/tapeosc

	echo "Building readsoundfile.c example...";
	gcc -g -std=c89 -Wall -pedantic -lm -Isrc -Ivendor examples/readsoundfile.c $(ALLSOURCES) -o build/readsoundfile


	echo "Rendering examples..."
	./scripts/render_examples.sh
