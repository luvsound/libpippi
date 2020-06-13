all: build render

.PHONY: build
build:
	gcc -lm example.c -o pulsar

.PHONY: render
render:
	./pulsar
	sox -r 44100 -e float -b 64 -c 2 out.raw out.wav

.PHONY: clean
clean:
	rm pulsar
