@echo off
setlocal

set PORT=%~1
if "%PORT%"=="" set PORT=COM3

pushd "%~dp0"

call ".\scripts\flash.bat" %PORT%

call ".\scripts\monitor.bat" %PORT%

popd
