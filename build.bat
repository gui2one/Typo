@echo off
CALL ./generate_project.bat

cd build
mingw32-make -j 4