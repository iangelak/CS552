#!/bin/bash


make clean
make
qemu-system-i386 -m 128 -kernel fifos
