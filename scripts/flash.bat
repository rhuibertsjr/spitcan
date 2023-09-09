@echo off
setlocal

set PORT=%~1
if "%PORT%"=="" set PORT=COM3

idf.py -p %PORT% flash

endlocal
