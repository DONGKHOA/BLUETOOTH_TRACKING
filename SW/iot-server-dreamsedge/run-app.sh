#!/bin/bash

# Run the frontend script in the background and redirect output
nohup python3 FrontEnd/main.py > frontend.log 2>&1 &

# Run the backend script in the background and redirect output
nohup python3 BackEnd/main.py > backend.log 2>&1 &

# Exit the script without waiting for processes to complete
exit 0
