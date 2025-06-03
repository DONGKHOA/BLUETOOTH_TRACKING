#!/bin/bash

redis-server --daemonize yes

# Run the backend
echo "Starting backend..."
python3 Backend/main.py &

# Run the frontend
echo "Starting frontend..."
python3 Frontend/ui.py
