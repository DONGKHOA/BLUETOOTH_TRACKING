#!/bin/bash

# Start Redis servers
redis-server --daemonize yes
sudo redis-server /etc/redis/redis6380.conf

# Start NanoMQ in the background
nanomq start --log_level debug &
NANOMQ_PID=$!

# Wait for NanoMQ to be fully ready
echo "[INFO] Waiting for NanoMQ to start..."
sleep 3

# Run the Python scripts
python3 listener.py &
python3 processor.py &
python3 ui.py