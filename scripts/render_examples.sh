#!/bin/bash
echo "Rendering pulsar example..."
time ./build/pulsar

echo "Rendering sineosc example..."
time ./build/sineosc

echo "Rendering memorypool example..."
time ./build/memorypool

