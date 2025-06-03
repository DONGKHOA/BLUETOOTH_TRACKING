#!/bin/bash
# start redis server
redis-server --daemonize yes
sudo redis-server /etc/redis/redis6381.conf
# Run both scripts in the background
python3 ui.py &
python3 main.py &
python3 publish_thingsboard.py
