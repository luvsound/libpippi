#!/bin/bash

mkdir -p build generated

echo "Generating static internal wavetables..."
gcc -std=c89 -Wall -pedantic -lm -Isrc -Ivendor tools/internal_static_wavetables.c src/pippicore.c -o build/internal_static_wavetables

./build/internal_static_wavetables > generated/internal_static_wavetables.c

echo "Done!"
