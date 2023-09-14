#!/bin/bash

PORT="${1:-COM3}"

idf.py -p "$PORT" -b 460800 monitor
