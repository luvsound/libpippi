#!/bin/bash
echo "Rendering pulsar example..."
time ./build/pulsarosc

echo "Rendering sineosc example..."
time ./build/sineosc

echo "Rendering ring_buffer example..."
time ./build/ring_buffer

echo "Rendering memorypool example..."
time ./build/memorypool
