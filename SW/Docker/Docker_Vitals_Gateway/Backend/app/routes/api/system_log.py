from flask import Blueprint, session, redirect, url_for, render_template, jsonify, request
import app.environment.environment_manager as environment_manager
import app.services.auth_server as auth_server
import redis

import os
import logging
import shutil
from datetime import datetime
from logging.handlers import RotatingFileHandler

from dotenv import load_dotenv

load_dotenv()

REDIS_HOST = os.getenv("REDIS_HOST", "redis")
REDIS_PORT = int(os.getenv("REDIS_PORT", 6379)) 

# Initialize Flask Blueprint
system_log_bp = Blueprint("system_log", __name__)

# Create a new handler with updated settings
log_folder = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../../../logs"))
os.makedirs(log_folder, exist_ok=True)  # Ensure directory exists

# Connect to Redis
redis_client = redis.Redis(host=REDIS_HOST, port=REDIS_PORT, db=0, decode_responses=True)

LOG_KEY = "system_logs"  # Key for storing logs in Redis

# ------------------------------------------------------
# Custom RotatingFileHandler with Date Format
# ------------------------------------------------------
class CustomRotatingFileHandler(RotatingFileHandler):
    def __init__(self, filename, maxBytes=1024 * 1024, backupCount=5):
        self.current_date = datetime.now().strftime("%Y-%m-%d")
        dated_filename = filename.replace(".txt", f"_{self.current_date}.txt")

        # Call base constructor with correct dated filename
        super().__init__(dated_filename, maxBytes=maxBytes, backupCount=backupCount)
        
        # Set baseFilename to the dated one
        self.baseFilename = dated_filename

    def shouldRollover(self, record):
        self.stream.flush()  # Ensure data is written
        if os.path.getsize(self.baseFilename) >= self.maxBytes:
            return True

        # Check if the date changed
        new_date = datetime.now().strftime("%Y-%m-%d")
        if new_date != self.current_date:
            return True

        return False

    def doRollover(self):
        if self.stream:
            self.stream.close()
            self.stream = None

        new_date = datetime.now().strftime("%Y-%m-%d")
        new_filename = self.baseFilename.replace(self.current_date, new_date)

        if new_date != self.current_date:
            self.current_date = new_date
            self.baseFilename = new_filename  # 🛠️ update to new file
        else:
            # Perform regular size-based rotation
            for i in range(self.backupCount - 1, 0, -1):
                src = self.baseFilename.replace(".txt", f"_{i}.txt")
                dst = self.baseFilename.replace(".txt", f"_{i+1}.txt")
                if os.path.exists(src):
                    try:
                        shutil.move(src, dst)
                    except Exception as e:
                        print(f"Failed to rotate {src} to {dst}: {e}")

            if os.path.exists(self.baseFilename):
                try:
                    shutil.move(self.baseFilename, self.baseFilename.replace(".txt", "_1.txt"))
                except Exception as e:
                    print(f"Failed to rotate {self.baseFilename}: {e}")

        # Open new stream for the new log file
        self.stream = self._open()

    def emit(self, record):
        if self.shouldRollover(record):
            self.doRollover()
        super().emit(record)

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

    log_file = os.path.join(log_folder, "system_logs.txt")
    log_handler = CustomRotatingFileHandler(log_file, maxBytes=max_bytes, backupCount=backup_count)

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