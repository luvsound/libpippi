#!/bin/bash
echo "Rendering pulsar example..."
time ./build/pulsarosc

echo "Rendering sineosc example..."
time ./build/sineosc

echo "Rendering memorypool example..."
time ./build/memorypool

