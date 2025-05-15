from flask import Blueprint, session, redirect, url_for, render_template, jsonify, request
from datetime import datetime
from logging.handlers import RotatingFileHandler

import redis
import os
import logging

# Initialize Flask Blueprint
system_log_bp = Blueprint("system_log", __name__)

# Connect to Redis
redis_client = redis.Redis(host='127.0.0.1', port=6380, db=0, decode_responses=True)

LOG_KEY = "system_logs"  # Key for storing logs in Redis

local_max_bytes = 1024 * 1024 * 1  # 1 MB
local_backup_count = 10  # Keep 10 backup files

# ------------------------------------------------------
# Custom RotatingFileHandler with Date Format
# ------------------------------------------------------
class CustomRotatingFileHandler(RotatingFileHandler):
    def __init__(self, filename, maxBytes=1024*1024, backupCount=5):
        self.current_date = datetime.now().strftime("%Y-%m-%d")
        self.base_filename = filename.replace(".txt", f"_{self.current_date}.txt")
        super().__init__(self.base_filename, maxBytes=maxBytes, backupCount=backupCount)
        
    # Check if rollover is needed (size limit or new day).
    def shouldRollover(self, record):
        self.stream.flush()  # Ensure file size is updated
        if os.path.getsize(self.baseFilename) >= self.maxBytes:  # Force check file size
            return True

        new_date = datetime.now().strftime("%Y-%m-%d")
        if new_date != self.current_date:
            self.current_date = new_date
            self.base_filename = self.baseFilename.replace(self.baseFilename.split("_")[-1], f"{new_date}.txt")
            return True

        return False

    # Perform log rotation while ensuring the file is reopened correctly.
    def doRollover(self):
        if self.stream:
            self.stream.close()
            self.stream = None  # Ensure the stream is properly closed

        # If the date changed, start a new log file
        new_date = datetime.now().strftime("%Y-%m-%d")
        if new_date != self.current_date:
            self.current_date = new_date
            self.base_filename = self.baseFilename.replace(self.baseFilename.split("_")[-1], f"{new_date}.txt")

        # Rotate old logs with new format
        for i in range(self.backupCount - 1, 0, -1):
            old_file = f"system_logs_{i}_{self.current_date}.txt"
            new_file = f"system_logs_{i+1}_{self.current_date}.txt"
            if os.path.exists(old_file):
                os.rename(old_file, new_file)

        # Rename the current log file
        log_backup = f"system_logs_1_{self.current_date}.txt"
        if os.path.exists(self.base_filename):
            os.rename(self.base_filename, log_backup)

        # **Reopen the log file to prevent "I/O operation on closed file" error**
        self.stream = self._open()
    # Ensure rollover happens before writing logs.
    def emit(self, record):
        if self.shouldRollover(record):  # Check file size and date
            self.doRollover()
        super().emit(record)  # Write log after rollover check

# ------------------------------------------------------
# Configure Logging Dynamically
# ------------------------------------------------------
def configure_logging(max_bytes, backup_count):
    global logger, local_max_bytes, local_backup_count

    # Update data
    local_max_bytes, local_backup_count = max_bytes, backup_count

    # Remove existing handlers
    for handler in logger.handlers[:]:
        logger.removeHandler(handler)

    # Create a new handler with updated settings
    log_handler = CustomRotatingFileHandler("system_logs.txt", maxBytes=max_bytes, backupCount=backup_count)
    log_handler.setFormatter(logging.Formatter('%(asctime)s - %(levelname)s - %(message)s'))
    
    # Apply the new handler
    logger.addHandler(log_handler)

# ------------------------------------------------------
# Log message to Redis and store it locally.
# ------------------------------------------------------
def log_to_redis(message, source="listener"):
    key = f"{LOG_KEY}_{source}"  # e.g., system_logs_listener or system_logs_processor

    # Push the new message into Redis
    redis_client.rpush(key, message)

    # Keep only the last 1000 logs
    redis_client.ltrim(key, -1000, -1)

    # Also log to file
    logger.info(f"[{source.upper()}] {message}")

    # Set expiry (7 days) only if not already set
    if redis_client.ttl(key) == -1:
        redis_client.expire(key, 7*24*60*60)

# Initialize Logger
logger = logging.getLogger()
logger.setLevel(logging.INFO)

configure_logging(local_max_bytes, local_backup_count)

# ------------------------------------------------------
# Fetch logs from Redis and remove duplicates 
# before returning.
# ------------------------------------------------------
@system_log_bp.route("/get_logs", methods=["GET"])
def get_logs():
    log_type = request.args.get("type", "listener")  # default to listener
    try:
        key = f"{LOG_KEY}_{log_type}"
        logs = redis_client.lrange(key, 0, -1)
        logs = list(dict.fromkeys(logs))

        return jsonify({"success": True, "logs": logs})
    except Exception as e:
        return jsonify({"success": False, "message": f"Error fetching logs: {str(e)}"}), 500

# ------------------------------------------------------
# Route: Clear Logs from Redis
# ------------------------------------------------------
@system_log_bp.route("/clear_logs", methods=["POST"])
def clear_logs():
    log_type = request.form.get("type", "listener")
    try:
        redis_client.delete(f"{LOG_KEY}_{log_type}")
        return jsonify({"success": True, "message": f"{log_type.capitalize()} logs cleared successfully"})
    except Exception as e:
        return jsonify({"success": False, "message": f"Error clearing logs: {str(e)}"}), 500

# ------------------------------------------------------
# Update Logging Configuration via API
# ------------------------------------------------------
@system_log_bp.route("/get_logging_config", methods=["GET"])
def get_logging_config():
    global local_max_bytes, local_backup_count

    try:
        return jsonify({
            "success": True,
            "maxBytes": local_max_bytes,
            "backupCount": local_backup_count
        })
    except Exception as e:
        return jsonify({"success": False, "message": f"Error fetching config: {str(e)}"}), 500

# ------------------------------------------------------
# Update Logging Configuration via API
# ------------------------------------------------------
@system_log_bp.route("/update_logging_config", methods=["POST"])
def update_logging_config():
    try:
        max_bytes = int(request.form["maxBytes"]) * 1024 * 1024  # Convert MB to Bytes
        backup_count = int(request.form["backupCount"])

        if max_bytes <= 0 or backup_count <= 0:
            return jsonify({"success": False, "message": "Values must be greater than 0!"})

        # Reload configuration dynamically
        configure_logging(max_bytes, backup_count)

        return jsonify({"success": True, "message": "Logging configuration updated successfully!"})
    except ValueError:
        return jsonify({"success": False, "message": "Invalid input. Please enter valid numbers!"})