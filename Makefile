# FIXME -- add a real makefile

all:
	mkdir -p build renders

	echo "Building pulsar.c example...";
	gcc -std=c99 -Wall -pedantic -lm -Isrc -Ivendor examples/pulsar.c src/memorypool.c src/sineosc.c src/pulsar.c src/soundfile.c src/interpolation.c src/window.c src/wavetable.c src/pippicore.c -o build/pulsar
	./build/pulsar
	
	echo "Building sineosc.c example...";
	gcc -std=c99 -Wall -pedantic -lm -Isrc -Ivendor examples/sineosc.c src/memorypool.c src/sineosc.c src/pulsar.c src/soundfile.c src/interpolation.c src/window.c src/wavetable.c src/pippicore.c -o build/sineosc
	./build/sineosc

	echo "Building memory_pool.c example...";
	gcc -std=c99 -Wall -pedantic -lm -Isrc -Ivendor -DLP_FLOAT -DLP_STATIC examples/memory_pool.c src/memorypool.c src/sineosc.c src/pulsar.c src/soundfile.c src/interpolation.c src/window.c src/wavetable.c src/pippicore.c -o build/memorypool
	./build/memorypool
