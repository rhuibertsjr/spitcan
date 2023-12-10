@echo off

pushd "%~dp0"

if not exist "sdkconfig" idf.py set-target esp32

idf.py -B .\build build 

popd
