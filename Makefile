CFLAGS += -I/usr/local/include
LDFLAGS += -L/usr/local/lib

all: install examples

install:
	sudo cp src/pippi.h /usr/local/include/pippi.h
	sudo cp src/wavetables.h /usr/local/include/wavetables.h
	sudo cp src/pulsar.h /usr/local/include/pulsar.h
	mkdir -p build renders

examples: ex1

.PHONY: ex1
ex1:
	#gcc -g -O0 -lm examples/pulsar.c -o build/pulsar
	gcc -std=c89 -Wall -pedantic -lm examples/pulsar.c -o build/pulsar
	./build/pulsar
	sox -r 44100 -e float -b 64 -c 2 renders/pulsar-out.raw renders/pulsar-out.wav
	play renders/pulsar-out.wav

.PHONY: clean
clean:
	rm pulsar out.raw out.wav
