all: install examples

install:
	mkdir -p build renders

examples: ex1

.PHONY: ex1
ex1:
	#gcc -g -O0 -lm examples/pulsar.c -o build/pulsar
	gcc -std=c89 -Wall -pedantic -lm -Isrc examples/pulsar.c -o build/pulsar
	./build/pulsar
	sox -r 44100 -e float -b 64 -c 2 renders/pulsar-out.raw renders/pulsar-out.wav
	play renders/pulsar-out.wav

.PHONY: clean
clean:
	rm pulsar out.raw out.wav
