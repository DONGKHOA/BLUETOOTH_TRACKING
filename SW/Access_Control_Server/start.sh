#!/bin/bash
# start redis server
redis-server --daemonize yes
# Run both scripts in the background
python3 ui.py &
python3 main.py
