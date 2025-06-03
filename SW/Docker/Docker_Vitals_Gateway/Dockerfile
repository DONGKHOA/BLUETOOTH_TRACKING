FROM python:3.10

# Install required system packages
RUN apt-get update && apt-get install -y gcc libffi-dev libssl-dev && rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y netcat-openbsd

# Set working directory
WORKDIR /app

# Copy project files
COPY . .

# Make it executable
RUN chmod +x /app/wait-for-redis.sh

# Install Python dependencies
RUN pip install --no-cache-dir -r requirements.txt

# Expose Flask app port
EXPOSE 5000

# Default command
CMD ["python", "ui.py"]

