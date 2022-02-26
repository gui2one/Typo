@echo off
mkdir build

cmake -b . -B ./build -G "MinGW Makefiles"