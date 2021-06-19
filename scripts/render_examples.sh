#!/bin/bash
echo "Rendering pulsar example..."
time ./build/pulsarosc

echo "Rendering sineosc example..."
time ./build/sineosc

echo "Rendering ring_buffer example..."
time ./build/ring_buffer

echo "Rendering onset_detector example..."
time ./build/onset_detector

echo "Rendering pitch_tracker example..."
time ./build/pitch_tracker

echo "Rendering memorypool example..."
time ./build/memorypool

echo "Rendering scheduler example..."
time ./build/scheduler

echo "Rendering additive synthesis example..."
time ./build/additive_synthesis

echo "Rendering graincloud example..."
time ./build/graincloud

echo "Rendering tapeosc example..."
time ./build/tapeosc

echo "Rendering readsoundfile example..."
time ./build/readsoundfile

