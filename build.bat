@echo off

if not exist ".\sdkconfig" idf.py set-target esp32

idf.py build
