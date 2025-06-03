from flask import Blueprint, session, redirect, url_for, render_template, jsonify, request

import app.services.control_server as control_server
import app.environment.environment_manager as environment_manager
import app.services.auth_server as auth_server
import app.routes.api.system_log as system_log

import app.services.http_client as http_client
import app.environment.environment as environment

import os
import asyncio

import redis
import json

from dotenv import load_dotenv

load_dotenv()

REDIS_HOST = os.getenv("REDIS_HOST", "redis")
REDIS_PORT = int(os.getenv("REDIS_PORT", 6379)) 

r = redis.Redis(host=REDIS_HOST, port=REDIS_PORT, db=0)

server_config_bp = Blueprint("server_configuration", __name__)

UPLOAD_FOLDER = "/app/Backend/app/upload"

@server_config_bp.route("/server_configuration")
def server_configuration():
    if not session.get("authenticated"):
        return render_template("login.html")

    if session.get("username") == "supervisor":

        print("[WARNING] Unauthorized access attempt by Supervisor.")
        system_log.log_to_redis("[WARNING] Unauthorized access attempt by Supervisor.")
        
        return redirect(url_for("server_management.server_management"))

    if session.get("username") == "admin" and not auth_server.validate_admin_session():

        print("[WARNING] Admin session invalid. Forcing logout.")
        system_log.log_to_redis("[WARNING] Admin session invalid. Forcing logout.")

        session.clear()
        return render_template("login.html")

    # Data handling with error protection
    try:
        # Get and parse facility data
        raw_facilities = environment_manager.get_facility_list()
        
        # Convert string to dictionary if needed
        if isinstance(raw_facilities, str):
            facilities = json.loads(raw_facilities)
        else:
            facilities = raw_facilities

        # Validate data structure
        if not isinstance(facilities, dict):
            raise ValueError("Invalid data format")
            
        facilities.setdefault("data", [])
        facilities.setdefault("count", len(facilities["data"]))

    except (json.JSONDecodeError, ValueError) as e:
        print(f"[ERROR] Facility data corruption: {str(e)}")
        system_log.log_to_redis(f"[ERROR] Facility data parsing failed: {str(e)}")
        facilities = {
            "data": [],
            "count": 0
        }

    return render_template("server_configuration.html", data=facilities["data"], count=facilities["count"])

# ------------------------------------------------------
# Start TCP server and accept data from devices 
# ------------------------------------------------------ 
@server_config_bp.route("/start_server", methods=["GET", "POST"])
def start_server():

    r.set("tcp_server_status", "running")

    return jsonify({"success": True, "message": "TCP Server started successfully!"})
    
# ------------------------------------------------------
# Stop TCP server and not accept data from devices 
# Stop TCP server to config server
# ------------------------------------------------------     
@server_config_bp.route("/stop_server", methods=["GET"])
def stop_server():

    r.set("tcp_server_status", "stopped")

    return jsonify({"success": True, "message": "TCP Server stopped successfully!"})

# --------------------------------------------------
# Authenticate Function
# --------------------------------------------------
@server_config_bp.route("/authenticate", methods=["POST"])
def authenticate():

    # Extract username & password
    username = request.form.get("username", "").strip()
    password = request.form.get("password", "").strip()

    print(f"[INFO] Received authenticate request - Username: {username}, Password: {'*' * len(password)}")
    system_log.log_to_redis(f"[INFO] Received authenticate request - Username: {username}, Password: {'*' * len(password)}")
    # Call authentication service
    success, redirect_url = auth_server.authenticate_user(username, password)

    r.set("username", username)
    r.set("password", password)

    return jsonify({"success": success, "redirect": redirect_url})

# --------------------------------------------------
# Facility list Function
# --------------------------------------------------
@server_config_bp.route("/facility_list", methods=["GET"])
def facility_list():

    facilities = auth_server.facility_list()

    if not facilities:
        return jsonify({"success": False, "message": "Authentication failed or no facilities available"}), 403

    return jsonify({"success": True, "data": facilities["data"], "count": facilities["count"]})

# ------------------------------------------------------
# Select Facility ID - Update Environment & Confirm Selection
# ------------------------------------------------------
@server_config_bp.route("/select_facility_id", methods=["POST"])
def select_facility_id():

    try:
        facility_id = request.form.get("facility_id")
        if not facility_id:
            return jsonify({"success": False, "message": "Facility ID not provided!"}), 400

        # Get the facility list
        facilities = environment_manager.get_facility_list()

        # Check if facility exists
        facility_exists = any(str(fac["id"]) == facility_id for fac in facilities.get("data", []))

        if not facility_exists:
            return jsonify({"success": False, "message": "Facility ID does not exist!"}), 404

        # Update the selected Facility ID in environment
        environment_manager.update_facility_id(facility_id)

        print(f"[INFO] Facility ID Updated: {facility_id}")
        system_log.log_to_redis(f"[INFO] Facility ID Updated: {facility_id}")

        http_client.register_tcp_server(facility_id, environment.TCP_SERVER_NAME)

        return jsonify({"success": True, "message": f"Facility ID '{facility_id}' updated successfully!"})

    except Exception as e:
        return jsonify({"success": False, "message": f"Error: {str(e)}"}), 500

# --------------------------------------------------
# Upload Certificates function: 
# -> Get 3 files of certificates and stored it in 
# upload folder
# --------------------------------------------------
@server_config_bp.route("/upload_cert", methods=["POST"])
def upload_cert():
    file_mapping = {
        "file1": "root_ca",
        "file2": "private_key",
        "file3": "cert"
    }

    saved_files = []
    updated_paths = {}

    for key, file_type in file_mapping.items():
        file = request.files.get(key)

        if not file or file.filename == '':
            continue

        ALLOWED_EXTENSIONS = environment_manager.get_allow_extensions()

        file_ext = os.path.splitext(file.filename)[1]  # Extract the extension correctly

        # Check if the extension is allowed
        if not any(file.filename.endswith(ext) for ext in ALLOWED_EXTENSIONS):
            return jsonify({
                "success": False,
                "message": f"Invalid file type: {file.filename}. Allowed types: {', '.join(ALLOWED_EXTENSIONS)}"
            }), 400

        try:
            # Delete all existing files with the same extension
            for existing_file in os.listdir(UPLOAD_FOLDER):
                if existing_file.endswith(file_ext):  # Match only by file extension
                    os.remove(os.path.join(UPLOAD_FOLDER, existing_file))
                    print(f"Deleted old file: {existing_file}")  # Debugging
                    system_log.log_to_redis(f"Deleted old file: {existing_file}")

            file_path = os.path.join(UPLOAD_FOLDER, file.filename)
            file.save(file_path)

            updated_paths[key] = file_path.replace("\\", "/")

            updated_paths[key] = environment_manager.update_environment_file(file_path, file_type)

            saved_files.append(file.filename)

            r.set("aws_server_status", "disconnected")

        except Exception as e:
            return jsonify({"success": False, "error": str(e)}), 500

    if saved_files:
        return jsonify({"success": True, "uploaded_files": saved_files, "updated_paths": updated_paths}), 200
    else:
        return jsonify({"success": False, "message": "No files selected"}), 400

# ---------------------------------------------------
# Update AWS IoT Endpoint function: 
# -> Get endpoint from GUI and update to 
# Environment.py
# ---------------------------------------------------
@server_config_bp.route("/update_aws_endpoint", methods=["POST"])
def update_aws_endpoint_route():

    new_endpoint = request.form.get("aws_iot_endpoint", "").strip()

    if not new_endpoint:
        return jsonify({"success": False, "message": "AWS IoT Endpoint cannot be empty!"}), 400

    environment_manager.update_aws_endpoint(new_endpoint)

    return jsonify({"success": True, "new_endpoint": new_endpoint})

# ------------------------------------------------------
# Get existing files and path of files: aws certificates
# ------------------------------------------------------
@server_config_bp.route('/get_existing_files', methods=['GET'])
def get_existing_files():

    # Reload environment.py to get the latest updates
    environment_manager.reload_environment()

    return jsonify({
        "success": True,
        "root_ca_path": environment_manager.get_root_ca_path(),
        "private_key_path": environment_manager.get_private_key_path(),
        "cert_path": environment_manager.get_cert_path()
    })

# ------------------------------------------------------
# Get uploaded files
# ------------------------------------------------------
@server_config_bp.route("/get_uploaded_files", methods=["GET"])
def uploaded_files():

    files, aws_iot_endpoint = environment_manager.get_uploaded_files()  # Get files + endpoint dynamically
    return jsonify({"files": files, "aws_endpoint": aws_iot_endpoint})

# ------------------------------------------------------
# Check AWS Connection function: 
# -> Check connection with new AWS IoT Endpoint
# ------------------------------------------------------
@server_config_bp.route("/check_aws_connection", methods=["GET"])
def check_aws_connection():

    # Run the async function inside an event loop
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    status = loop.run_until_complete(auth_server.process_aws_connection_check())

    return jsonify({"status": status})

# ------------------------------------------------------
# Check the current status of server to disable buttons 
# ------------------------------------------------------
@server_config_bp.route("/button_server_status")
def button_server_status():
    status_server = environment_manager.get_start_server()
    
    # Get auth token status from Redis
    auth_token = r.get("auth_token")
    enable_start_button = bool(auth_token)

    return jsonify({
        "running": status_server,
        "enable_start_button": enable_start_button
    }) 
