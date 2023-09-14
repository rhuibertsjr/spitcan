#!/bin/bash

PORT="${1:-COM3}"

idf.py -p "$PORT" monitor
