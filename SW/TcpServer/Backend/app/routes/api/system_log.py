from flask import Blueprint, session, redirect, url_for, render_template, jsonify, request
import app.environment.environment_manager as environment_manager
import app.services.auth_server as auth_server
import redis

import os
import logging
from datetime import datetime
from logging.handlers import RotatingFileHandler

# Initialize Flask Blueprint
system_log_bp = Blueprint("system_log", __name__)

# Connect to Redis
redis_client = redis.Redis(host='127.0.0.1', port=6379, db=0, decode_responses=True)

LOG_KEY = "system_logs"  # Key for storing logs in Redis

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
    global logger

    # Update data
    environment_manager.update_logging_configuration(max_bytes, backup_count)

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
def log_to_redis(message):
    logs = redis_client.lrange(LOG_KEY, -10, -1)  # Get the last 10 logs

    if message not in logs:
        redis_client.rpush(LOG_KEY, message)
        redis_client.ltrim(LOG_KEY, -1000, -1)  # Keep last 10000 logs

        logger.info(message)   # Store log locally

        # Only set expiry if not already set!
        if redis_client.ttl(LOG_KEY) == -1:
            redis_client.expire(LOG_KEY, 7*24*60*60)  # Auto-delete after 7 days

# Initialize Logger
logger = logging.getLogger()
logger.setLevel(logging.INFO)

max_bytes, backup_count = environment_manager.get_logging_configuration()

configure_logging(max_bytes, backup_count)
# ------------------------------------------------------
# System log home
# ------------------------------------------------------
@system_log_bp.route("/system_log")
def system_log_home():
    if not session.get("authenticated"):
        return render_template("login.html")

    if session.get("username") == "admin" and not auth_server.validate_admin_session():
        print("[WARNING] Admin session invalid. Forcing logout.")
        session.clear()
        return render_template("login.html")

    return render_template("system_log.html")

# ------------------------------------------------------
# Fetch logs from Redis and remove duplicates 
# before returning.
# ------------------------------------------------------
@system_log_bp.route("/get_logs", methods=["GET"])
def get_logs():
    try:
        logs = redis_client.lrange(LOG_KEY, 0, -1)  # Fetch all logs
        logs = list(dict.fromkeys(logs))  # Remove duplicates while keeping order

        return jsonify({"success": True, "logs": logs})
    except Exception as e:
        return jsonify({"success": False, "message": f"Error fetching logs: {str(e)}"}), 500

# ------------------------------------------------------
# Route: Clear Logs from Redis
# ------------------------------------------------------
@system_log_bp.route("/clear_logs", methods=["POST"])
def clear_logs():
    try:
        redis_client.delete(LOG_KEY)  # Delete log key from Redis
        return jsonify({"success": True, "message": "Logs cleared successfully"})
    except Exception as e:
        return jsonify({"success": False, "message": f"Error clearing logs: {str(e)}"}), 500


# ------------------------------------------------------
# Update Logging Configuration via API
# ------------------------------------------------------
@system_log_bp.route("/get_logging_config", methods=["GET"])
def get_logging_config():

    max_bytes, backup_count = environment_manager.get_logging_configuration()
    try:
        return jsonify({
            "success": True,
            "maxBytes": max_bytes,
            "backupCount": backup_count
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