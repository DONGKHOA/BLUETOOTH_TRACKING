from flask import Flask, render_template, request, redirect, url_for, jsonify, session
from flask_socketio import SocketIO, emit
import redis
import json
import os
import paho.mqtt.client as mqtt
from dotenv import load_dotenv
from collections import defaultdict
import time
import threading
from functools import wraps
import hashlib
from datetime import datetime

from system_log import system_log_bp

app = Flask(__name__)
app.secret_key = os.getenv("SECRET_KEY", "your-secret-key")
socketio = SocketIO(app)

app.register_blueprint(system_log_bp)

load_dotenv()
BROKER_HOST = os.getenv("BROKER_HOST", "localhost")
BROKER_PORT = int(os.getenv("BROKER_PORT", 1883))
QUEUE_HOST = os.getenv("QUEUE_HOST", "localhost")
QUEUE_PORT = int(os.getenv("QUEUE_PORT", 6380))
redis_client = redis.Redis(host=QUEUE_HOST, port=QUEUE_PORT, db=0)

mqtt_client = mqtt.Client()
mqtt_client.connect(BROKER_HOST, BROKER_PORT, 60)
mqtt_client.loop_start()

# Set your desired username and password
username = 'admin'
password = '123'

#-------------------------------------------------------------------------------------
# PUBLIC VARIABLES
#-------------------------------------------------------------------------------------
gateways = []
#-------------------------------------------------------------------------------------
# PUBLIC FUNCTIONS
#-------------------------------------------------------------------------------------
def init_users():
    if not redis_client.exists("users"):
        redis_client.hset("users", "admin", hashlib.md5("admin123".encode()).hexdigest())

def update_realtime_data():
    while True:
        msg_rate = redis_client.llen("beacon_data")
        aws_rate = redis_client.llen("aws_queue")
        beacons_detected = len(redis_client.hkeys("beacon_state") or [])

        system_status = {
            "gateways": len(gateways),
            "gateways_online": sum(1 for gw in gateways if gw["status"] == "Online"),
            "beacons_detected": beacons_detected,
            "broker": "Online" if mqtt_client.is_connected() else "Offline",
            "redis": "Online" if redis_client.ping() else "Offline",
        }
        
        socketio.emit('update_dashboard', {
            'status': system_status
        })
        time.sleep(1)

#-------------------------------------------------------------------------------------
# INIT SERVER
#-------------------------------------------------------------------------------------
init_users()
threading.Thread(target=update_realtime_data, daemon=True).start()

#-------------------------------------------------------------------------------------
# API FOR AUTHENTICATE SERVER
#-------------------------------------------------------------------------------------
def login_required(f):
    @wraps(f)
    def decorated_function(*args, **kwargs):
        if 'username' not in session:
            return redirect(url_for('login'))
        return f(*args, **kwargs)
    return decorated_function

@app.route('/login', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':

        username = request.form['username']
        password = request.form['password']

        stored_password = redis_client.hget("users", username)

        if stored_password and stored_password.decode() == hashlib.md5(password.encode()).hexdigest():

            session['username'] = username
            return redirect(url_for('dashboard'))

        return render_template('login.html', error="Invalid credentials")

    return render_template('login.html')

@app.route('/change-password', methods=['GET', 'POST'])
@login_required
def change_password():

    if request.method == 'POST':

        old_password = request.form['old_password']
        new_password = request.form['new_password']

        username = session['username']
        stored_password = redis_client.hget("users", username).decode()

        if hashlib.md5(old_password.encode()).hexdigest() == stored_password:

            redis_client.hset("users", username, hashlib.md5(new_password.encode()).hexdigest())
            return redirect(url_for('dashboard'))

        return render_template('change_password.html', error="Old password incorrect")

    return render_template('change_password.html')

@app.route('/logout')
def logout():
    session.pop('username', None)
    return redirect(url_for('login'))

#-------------------------------------------------------------------------------------
# API FOR PROCESSING DASHBOARD
#-------------------------------------------------------------------------------------
@app.route('/api/dashboard', methods=['GET'])
@login_required
def api_dashboard():
    msg_rate = redis_client.llen("beacon_data")
    aws_rate = redis_client.llen("aws_queue")
    beacons_detected = len(redis_client.hkeys("beacon_state") or [])
    system_status = {
        "gateways": len(gateways),
        "gateways_online": sum(1 for gw in gateways if gw["status"] == "Online"),
        "beacons_detected": beacons_detected,
        "broker": "Online" if mqtt_client.is_connected() else "Offline",
        "redis": "Online" if redis_client.ping() else "Offline",
    }
    return jsonify({
        "status": system_status
    })

@app.route('/')
@login_required
def dashboard():
    return render_template('dashboard.html')

#-------------------------------------------------------------------------------------
# API FOR PROCESSING GATEWAY
#-------------------------------------------------------------------------------------
@app.route('/api/gateways', methods=['GET'])
def api_gateways():
    global gateways
    gateways = []  # Reset the global list

    current_time = time.time()  # Get current time
    offline_threshold = 30 

    # Track beacons per gateway
    gateway_beacon_counts = defaultdict(int)

    for key, value in redis_client.hgetall("beacon_state").items():
        beacon_data = json.loads(value.decode())

        # Handle missing "gateways" key
        detected_gateways = beacon_data.get("gateways", [])  # Default to an empty list if missing

        for gw in detected_gateways:
            gateway_beacon_counts[gw] += 1

    # Retrieve gateway info
    for key, value in redis_client.hgetall("gateway_status").items():
        gateway_id = key.decode()
        data = json.loads(value.decode())

        total_beacons = gateway_beacon_counts.get(gateway_id, 0)

        last_seen = data.get("last_seen", 0)
        gateway_status = "Offline" if (current_time - last_seen) > offline_threshold else "Online"

        gateways.append({
            "id": gateway_id,
            "ip": data.get("ip", "Unknown"),
            "status": gateway_status,
            "last_seen": last_seen,
            "beacons": total_beacons  # Now includes beacons from multiple gateways
        })

    return jsonify(gateways)

@app.route('/api/gateways/delete_all', methods=['POST'])
def delete_all_gateways():

    # Delete gateway status and beacon state from Redis
    redis_client.delete("gateway_status")
    redis_client.delete("beacon_state")

    return jsonify({"success": True, "message": "All gateways deleted."})

@app.route('/gateways')
@login_required
def gateways_list():
    return render_template('gateways.html')

#-------------------------------------------------------------------------------------
# API FOR PROCESSING BEACONS
#-------------------------------------------------------------------------------------    
@app.route('/api/beacons/<beacon_id>', methods=['DELETE'])
# @login_required
def delete_beacon(beacon_id):

    deleted = redis_client.hdel("beacon_state", beacon_id)

    new_queue = []
    for log in redis_client.lrange("aws_queue", 0, -1):
        log_data = json.loads(log.decode())
        if log_data.get("beacon_id") != beacon_id:
            new_queue.append(log)

    redis_client.delete("aws_queue")  # Clear old queue
    for log in new_queue:
        redis_client.rpush("aws_queue", log)  # Re-add logs without deleted beacon
    
    if deleted:
        return jsonify({"success": True, "message": f"Deleted beacon {beacon_id}"})
    return jsonify({"success": False, "message": "Beacon not found"}), 404

#--------------------------------------------------
# Receive data of beacons and show in UI (Beacons)
#--------------------------------------------------
@app.route('/api/beacons', methods=['GET'])
@login_required
def api_beacons():

    beacons = {}

    # Fetch all detected beacons from `beacon_state`
    for beacon_id in redis_client.hkeys("beacon_state"):
        beacon_id = beacon_id.decode()
        beacon_data = redis_client.hget("beacon_state", beacon_id)
        if beacon_data:
            beacon_info = json.loads(beacon_data.decode())

            detected_gateways = beacon_info.get("gateways", [])
            rssi_scores = beacon_info.get("rssi_scores", {})
            timestamp = beacon_info.get("timestamp", None)  # Get timestamp

            # Convert timestamp to Unix time (seconds) if needed
            if isinstance(timestamp, str):
                try:
                    dt = datetime.strptime(timestamp, "%Y-%m-%dT%H:%M:%S")
                    timestamp = int(time.mktime(dt.timetuple()))
                except ValueError:
                    timestamp = None  # Handle invalid timestamps

            elif isinstance(timestamp, int) and len(str(timestamp)) > 10:
                timestamp = int(str(timestamp)[:10])  # Trim to 10 digits (seconds)

            # Convert timestamp to Vietnam Time (UTC+7) manually
            if timestamp:
                timestamp += 7 * 3600  # Add 7 hours (7 * 3600 seconds)
                vietnam_time = datetime.utcfromtimestamp(timestamp)
                formatted_time = vietnam_time.strftime("%Y-%m-%d %H:%M:%S")  # Store as string
            else:
                formatted_time = "N/A"

            # Ensure the beacon is still detected
            detected = 1 if detected_gateways else 0  

            beacons[beacon_id] = {
                "gateways": detected_gateways,  
                "rssi_scores": rssi_scores,  
                "last_seen": formatted_time,
                "detected": detected
            }
            
            redis_client.rpush("thingsboard_queue", json.dumps({
                "gateways": detected_gateways,
                "beacon_id": beacon_id,
                "timestamp": formatted_time
            }))

    return jsonify(beacons)

@app.route('/api/beacon_logs', methods=['GET'])
def api_beacon_logs():
    logs = []
    for log in redis_client.lrange("aws_queue", -100, -1):
        log_data = json.loads(log.decode())

        timestamp = log_data.get("timestamp", None)

        # Convert timestamp to Unix time (seconds) if needed
        if isinstance(timestamp, str):
            try:
                dt = datetime.strptime(timestamp, "%Y-%m-%dT%H:%M:%S")
                timestamp = int(time.mktime(dt.timetuple()))
            except ValueError:
                timestamp = None  # Handle invalid timestamps

        elif isinstance(timestamp, int) and len(str(timestamp)) > 10:
            timestamp = int(str(timestamp)[:10])  # Trim to 10 digits (seconds)

        # Convert timestamp to Vietnam Time (UTC+7) manually
        if timestamp:
            timestamp += 7 * 3600  # Add 7 hours (7 * 3600 seconds)
            vietnam_time = datetime.utcfromtimestamp(timestamp)
            log_data["timestamp"] = vietnam_time.strftime("%Y-%m-%d %H:%M:%S")  # Store as string
        else:
            log_data["timestamp"] = "N/A"

        logs.append(log_data)

    # Sort logs by timestamp (newest first)
    logs.sort(key=lambda x: datetime.strptime(x["timestamp"], "%Y-%m-%d %H:%M:%S") if x["timestamp"] != "N/A" else datetime.min, reverse=True)

    return jsonify(logs)

@app.route('/api/clear_beacon_logs', methods=['DELETE'])
def clear_beacon_logs():
    redis_client.delete("beacon_state")  # Clear beacon state (all tag data)
    redis_client.delete("beacon_last_event")
    redis_client.delete("aws_queue")  # Delete the Redis list storing logs
    return jsonify({"success": True, "message": "All beacon logs deleted."})

@app.route('/api/clear_dectection_beacon_logs', methods=['DELETE'])
def clear_dectection_beacon_logs():
    redis_client.delete("aws_queue")  # Delete the Redis list storing logs
    return jsonify({"success": True, "message": "All beacon logs deleted."})

@app.route('/api/beacons/<beacon_id>', methods=['PUT'])
# @login_required
def edit_beacon(beacon_id):

    return jsonify({"success": True, "message": f"Updated {beacon_id}"})

@app.route('/beacons')
@login_required
def beacons_list():
    return render_template('beacons.html')

#-------------------------------------------------------------------------------------
# API FOR CONFIGURATION SERVER
#-------------------------------------------------------------------------------------
@app.route('/api/config', methods=['GET', 'POST'])
@login_required
def api_config():
    return jsonify({"success": True})

@app.route('/config')
@login_required
def config():
    return render_template('config.html')

#-------------------------------------------------------------------------------------
# API FOR SHOW LOGS OF SYSTEM
#-------------------------------------------------------------------------------------
@app.route('/api/logs', methods=['GET'])
@login_required
def api_logs():
    service_filter = request.args.get('service')
    time_filter = request.args.get('time')
    logs = [json.loads(log.decode()) for log in redis_client.lrange("logs", -50, -1)]
    if service_filter:
        logs = [log for log in logs if log["service"] == service_filter]
    if time_filter:
        logs = [log for log in logs if time_filter in log["time"]]
    stats = {"processing_time": "50ms", "latency": "200ms"}
    return jsonify({"logs": logs, "stats": stats})

@app.route('/logs')
@login_required
def logs():
    return render_template('logs.html')

#-------------------------------------------------------------------------------------
#-------------------------------------------------------------------------------------
@socketio.on('connect')
def handle_connect():
    if 'username' not in session:
        return False
    print("Client connected")

#-------------------------------------------------------------------------------------
#-------------------------------------------------------------------------------------
if __name__ == "__main__":

    redis_client.flushall()

    # Hash the password
    hashed_password = hashlib.md5(password.encode()).hexdigest()

    # Store in Redis
    redis_client.hset("users", username, hashed_password)

    socketio.run(app, debug=True, host="0.0.0.0", port=8000)