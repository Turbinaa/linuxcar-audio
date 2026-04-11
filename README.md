# linuxcar-audio

Provides LED audio-based illumination for linuxcar project

## Overview

Captures audio from a PipeWire sink monitor, catches amplitude and smoothens it out, published over ZeroMQ IPC. Designed for linuxcar project.

## Dependencies

pipewire
libpipewire-0.3-dev
libspa-0.2-dev
libzmq3-dev

## Building

Run `make` with provided Makefile

## Usage
-hz   sample rate                (default: 1000)
-ss   sample size                (default: 16)
-sn   sensitivity                (default: 10000)
--socket path-to-socket          (default: /run/cardash/bus.sock)

## Architecture

PipeWire sink monitor
       ↓
process() [RT thread]
       ↓ mutex
out_thread — EMA smoothing
       ↓
ZeroMQ PUB
ipc:///run/cardash/bus.sock
topic: audio.avg_int <0-1000>
