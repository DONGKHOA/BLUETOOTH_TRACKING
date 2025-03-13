# -------------------------------------------------
# Import libraries to initialize GUI
# -------------------------------------------------

from flask import Flask, render_template, request, redirect, url_for, session, flash, jsonify
import time
import json

# -------------------------------------------------
# Link the Environment and FrontEnd folders
# Therefore, can import environment.py
# -------------------------------------------------

import os
import sys

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))

sys.path.append(os.path.join(PROJECT_ROOT, "Environment"))

import environment
import importlib
# -------------------------------------------------
# Included function to get data from environment.py
# -------------------------------------------------

from environment_manager import (
    update_info_user, 
    reset_credentials, 
    get_auth_token,
    get_facility_list,
    update_facility_id,
    update_aws_endpoint,
    update_connection_aws,
    update_start_check_aws,
    get_connection_aws,
    update_status_tcp_server,
    update_status_aws_server,
    get_status_aws_server,
    get_status_tcp_server,
    update_start_server,
    update_stop_server,
    get_reconfig_server,
    reset_auth_token,
    get_info_user,
    update_reconfig_info,
    update_reconfig_facility_id,
    update_reconfig_aws_endpoint,
    get_start_server,
    get_login_info,
    update_environment_file,
    reload_environment,
    get_uploaded_files,
    update_temp_auth_token,
    get_temp_auth_token,
    update_temp_info_user,
    get_temp_auth_token
)

server_running = False

# -------------------------------------------------
# Application to interface with User
# -------------------------------------------------

app = Flask(__name__, static_folder="static")
app.secret_key = "your_secret_key_here"  # Change this to a strong, random value

# Absolute path to upload folder
ENVIRONMENT_FOLDER = os.path.join(os.getcwd(), 'Environment')
UPLOAD_FOLDER = os.path.join(ENVIRONMENT_FOLDER, 'upload')
os.makedirs(UPLOAD_FOLDER, exist_ok=True)

app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER

# Print paths for verification
print("Current working directory:", os.getcwd())
print("Upload folder path:", app.config['UPLOAD_FOLDER'])

# Reset credentials on startup
reset_credentials() 

# --------------------------------------------------
# Home -> Return login screen
# --------------------------------------------------
@app.route("/")
def home():
    return redirect(url_for("login"))  # Redirect to login page

# --------------------------------------------------
# Login function: 
# -> Check the username and password if it matched 
# -> Entry to the server_status page
# --------------------------------------------------
@app.route("/login", methods=["GET", "POST"])
def login():
    if request.method == "POST":
        login_username_new = request.form.get("username", "").strip()
        login_password_new = request.form.get("password", "").strip()

        print(f"[INFO] Received login request - Username: {login_username_new}, Password: {'*' * len(login_password_new)}")

        # Fetch stored credentials
        credentials = get_login_info()  # ⬅ Fetch stored username & password
        login_username_old = credentials["login_username"]
        login_password_old = credentials["login_password"]

        # Check if username and password match
        if login_username_new == login_username_old and login_password_new == login_password_old:
            print("[INFO] Login successful! Redirecting to server.")
            session["authenticated"] = True  # Set session
            return jsonify({"success": True, "redirect": url_for("server_status")}) 

        else:
            print("[ERROR] Incorrect username or password.")
            return jsonify({"success": False, "message": "Incorrect username or password.", "redirect": url_for("login")})

    # 🔹 Fix: Return login page on GET request
    return render_template("login.html")

# --------------------------------------------------
# Authenticate function: 
# -> Received username and password and update 
# to Environment.py
# -> Get auth_token and update to Environment.py
# --------------------------------------------------
@app.route("/authenticate", methods=["GET", "POST"])
def authenticate():
    if not session.get("authenticated"):
        return redirect(url_for("login"))

    if request.method == "POST":
        username = request.form.get("username", "").strip()
        password = request.form.get("password", "").strip()

        print(f"[INFO] Received authenticate request - Username: {username}, Password: {'*' * len(password)}")

        # update_temp_auth_token(None)

        update_temp_info_user(username, password)

        timeout = 2  # Stop waiting after 5 seconds
        start_time = time.time()

        while time.time() - start_time <= timeout:
            test = get_temp_auth_token()
            
            time.sleep(0.5)

        if test:
            update_info_user(username, password)
            update_temp_auth_token(None)
            update_reconfig_info(True)

        start_time = time.time()
        while time.time() - start_time <= timeout:
            print("[INFO] Waiting for Auth Token to update...")
            auth_token = get_auth_token()
            time.sleep(0.5)

        # --------------------------------------------------
        # Check for AUTH_TOKEN every second
        # If received, set session and redirect to dashboard
        # --------------------------------------------------
        start_time = time.time()
        while True:
            auth_token = get_auth_token()
            temp_auth_token = get_temp_auth_token()
            tcp_server_status = get_status_tcp_server()

            reload_environment()

            if environment.TEMP_USERNAME == environment.USERNAME and environment.TEMP_PASSWORD == environment.PASSWORD:
                print(f"[INFO] AUTH_TOKEN received: {auth_token}")
                session["authenticated"] = True  # Set session
                return jsonify({"success": True, "redirect": url_for("dashboard")})
            else:
                return jsonify({"success": False, "redirect": url_for("dashboard")})
                    
            if time.time() - start_time > timeout:
                print("Timeout: AUTH_TOKEN not received.")

                if tcp_server_status:  
                    print("[INFO] Redirecting to Server Status due to TCP Server running")
                    session["authenticated"] = True
                    return jsonify({"success": True, "redirect": url_for("dashboard")})
                break

            print("[INFO] Waiting for AUTH_TOKEN from BackEnd...")
            
            time.sleep(1)
            
        return jsonify({"success": False, "redirect": url_for("dashboard")})
    
    return jsonify({"success": False, "redirect": url_for("dashboard")})

@app.route("/server_status")
def server_status():
    if "authenticated" in session and session["authenticated"]:
        return render_template("server_status.html")

    return render_template("login.html")

# --------------------------------------------------
# Dashboard function: 
# -> Get facility lists and show in GUI
# --------------------------------------------------
@app.route("/dashboard", methods=["GET"])
def dashboard():
    if not session.get("authenticated"):
        return redirect(url_for("login"))

    skip_fetch = request.args.get("skip_fetch", "false").lower() == "true"
    
    api_data = {"data": [], "count": 0}

    if not skip_fetch:
        timeout = 3  # Stop waiting after 3 seconds
        start_time = time.time()
        
        auth_token = get_auth_token()
        reconfig_server = get_reconfig_server()

        while reconfig_server and time.time() - start_time <= timeout:
            print("[INFO] Waiting for Auth Token to update...")
            auth_token = get_auth_token()
            time.sleep(0.5)

        # if not auth_token and reconfig_server:
        #     print("[WARNING] No Auth Token found. Showing blank table.")
        #     api_data = {"data": [], "count": 0}
        #     return render_template("dashboard.html", data=api_data["data"], count=api_data["count"])
        
        start_time = time.time()
        while time.time() - start_time <= timeout:
            api_data = get_facility_list()

            if api_data and "data" in api_data and len(api_data["data"]) > 0:
                
                print("[INFO] Facility list updated, rendering dashboard...")
                break

            time.sleep(1)

        if not api_data or not api_data.get("data"):
            print("[WARNING] Facility list fetch failed. Showing blank table.")
            api_data = {"data": [], "count": 0}
        
    return render_template("dashboard.html", data=api_data["data"], count=api_data["count"])

# --------------------------------------------------
# Upload Certificates function: 
# -> Get 3 files of certificates and stored it in 
# upload folder
# --------------------------------------------------
@app.route("/upload_cert", methods=["POST"])
def upload_cert():
    """Handles uploading certificate files and replaces old ones if the extension is valid."""
    if not session.get("authenticated"):
        return redirect(url_for("login"))

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
            continue  # Skip empty file selections

        file_ext = os.path.splitext(file.filename)[1]  # Extract the extension correctly

        # Check if the extension is allowed
        if not any(file.filename.endswith(ext) for ext in environment.ALLOWED_EXTENSIONS):
            return jsonify({
                "success": False,
                "message": f"Invalid file type: {file.filename}. Allowed types: {', '.join(environment.ALLOWED_EXTENSIONS)}"
            }), 400

        try:
            # Delete all existing files with the same extension
            for existing_file in os.listdir(UPLOAD_FOLDER):
                if existing_file.endswith(file_ext):  # Match only by file extension
                    os.remove(os.path.join(UPLOAD_FOLDER, existing_file))
                    print(f"Deleted old file: {existing_file}")  # Debugging

            # Save the new file
            file_path = os.path.join(UPLOAD_FOLDER, file.filename)
            file.save(file_path)

            # Ensure path format consistency
            file_path = file_path.replace("\\", "/")

            # Update environment.py with the new path and store the updated path
            updated_paths[key] = update_environment_file(file_path, file_type)

            saved_files.append(file.filename)

            update_status_aws_server(False)

        except Exception as e:
            return jsonify({"success": False, "error": str(e)}), 500

    if saved_files:
        return jsonify({"success": True, "uploaded_files": saved_files, "updated_paths": updated_paths}), 200
    else:
        return jsonify({"success": False, "message": "No files selected"}), 400



# --------------------------------------------------
# Select facility ID function: 
# -> Choose facility ID and update to Environment.py
# --------------------------------------------------
@app.route("/select_facility_id", methods=["POST"])
def select_facility_id():
    if not session.get("authenticated"):
        return redirect(url_for("login"))

    facility_id = request.form.get("facility_id")  # Get selected Facility ID

    if not facility_id:
        flash("No facility ID selected!", "facility_id_error")
        return redirect(url_for("dashboard"))

    try:
        update_facility_id(facility_id)
        flash(f"Facility ID updated to {facility_id}", "facility_id_success")
        
        update_reconfig_facility_id(True)
        print("[INFO] Notified backend: Facility ID has changed.")

    except Exception as e:
        flash(f"Error updating Facility ID: {str(e)}", "facility_id_error")

    return redirect(url_for("dashboard", skip_fetch = "true"))


# ---------------------------------------------------
# Update AWS IoT Endpoint function: 
# -> Get endpoint from GUI and update to 
# Environment.py
# ---------------------------------------------------
@app.route("/update_aws_endpoint", methods=["POST"])
def update_aws_endpoint_route():
    """Update AWS IoT Endpoint in Environment.py and reload dynamically."""
    if not session.get("authenticated"):
        return redirect(url_for("login"))

    try:
        new_endpoint = request.form.get("aws_iot_endpoint", "").strip()

        if not new_endpoint:
            return jsonify({"success": False, "message": "AWS IoT Endpoint cannot be empty!"}), 400

        # Update the environment variable
        update_aws_endpoint(new_endpoint)

        # Reload environment.py to ensure `AWS_IOT_ENDPOINT` is updated
        importlib.reload(environment)

        print(f"New AWS IoT Endpoint after update: {environment.AWS_IOT_ENDPOINT}")  # Debugging

        return jsonify({
            "success": True,
            "message": "AWS IoT Endpoint updated successfully!",
            "new_endpoint": environment.AWS_IOT_ENDPOINT  # Send updated endpoint to frontend
        })

    except Exception as e:
        return jsonify({"success": False, "message": f"Error: {str(e)}"}), 500


# ------------------------------------------------------
# Check AWS Connection function: 
# -> Check connection with new AWS IoT Endpoint
# ------------------------------------------------------
@app.route("/check_aws_connection", methods=["GET"])
def check_aws_connection():
    if not session.get("authenticated"):
        return redirect(url_for("login"))

    # Initiate AWS connection check
    update_start_check_aws(True)
    update_connection_aws(None)
    update_reconfig_aws_endpoint(True)  

    timeout = 10  # Timeout in seconds
    start_time = time.time()

    while time.time() - start_time < timeout:
        connection_aws = get_connection_aws()
        if connection_aws is not None:
            update_connection_aws(connection_aws)
            update_reconfig_aws_endpoint(False)
            return jsonify({"status": "success" if connection_aws else "failure"})

        time.sleep(0.5)  # Reduce CPU usage

    return jsonify({"status": "timeout"})

# ------------------------------------------------------
# Check server status function: 
# -> Check status of TCP server and AWS server
# ------------------------------------------------------
@app.route("/check_status_server", methods=["GET"])
def check_status_server():
    if not session.get("authenticated"):
        return redirect(url_for("login"))

    timeout = 10  # Stop waiting after 10 seconds
    start_time = time.time()
    
    while time.time() - start_time < timeout:
        tcp_status = get_status_tcp_server()  
        aws_status = get_status_aws_server()
        print("tcp status", tcp_status)

        if tcp_status is not None and aws_status is not None:
            return jsonify({
                "tcp_status": "running" if tcp_status else "stopping",
                "aws_status": "connected" if aws_status else "disconnected"
            })

        time.sleep(0.5) 
        
    return jsonify({
        "tcp_status": "timeout",
        "aws_status": "timeout"
    })
    
# ------------------------------------------------------
# Set START_SERVER to True and check if server needs 
# to start 
# ------------------------------------------------------ 
@app.route("/start_server", methods=["GET"])  
def start_server():
    if not session.get("authenticated"):
        return redirect(url_for("login"))

    global server_running

    if not get_start_server():  # If the server is NOT running
        
        server_running = True

        update_start_server(True)
        update_stop_server(False)

        return jsonify({"message": "TCP Server started successfully."}), 200

    return jsonify({"message": "TCP Server is already running."}), 200
    
# ------------------------------------------------------
# Set STOP_SERVER to True and check if server needs 
# to stop 
# ------------------------------------------------------   

@app.route("/stop_server", methods=["GET"])
def stop_server():
    if not session.get("authenticated"):
        return redirect(url_for("login"))

    global server_running

    if get_start_server():  # If the server IS running

        server_running = False

        update_stop_server(True)
        update_start_server(False)

        return jsonify({"message": "TCP Server stopped successfully."}), 200

    return jsonify({"message": "TCP Server is already stopped."}), 200   

# ------------------------------------------------------
# Check the current status of server to disable buttons 
# ------------------------------------------------------
@app.route("/button_server_status")
def button_server_status():
    global server_running

    status_server = get_start_server()

    if status_server:
        server_running = True
    else:
        server_running = False

    auth_token = get_auth_token()
    if auth_token:
        enable_start_button = True
    else:
        enable_start_button = False

    return jsonify({"running": server_running, "enable_start_button": enable_start_button})

# ------------------------------------------------------
# Get data of device: number of devices connect to 
# server, the last messages server received
# ------------------------------------------------------
@app.route("/get_device_management", methods=["GET"])
def get_device_management():
    # Simulating device data (Replace this with actual data retrieval logic)
    device_data = {
        "devices": [
            {"id": 1, "last_message": "2025-03-07 14:30:00"},
            {"id": 2, "last_message": "2025-03-07 14:32:15"},
        ]
    }
    return jsonify(device_data)  # Return JSON response

# ------------------------------------------------------
# Get existing files and path of files: aws certificates
# ------------------------------------------------------
@app.route('/get_existing_files', methods=['GET'])
def get_existing_files():

    # Reload environment.py to get the latest updates
    reload_environment()

    return jsonify({
        "success": True,
        "root_ca_path": environment.root_ca_path,
        "private_key_path": environment.private_key_path,
        "cert_path": environment.cert_path
    })

# ------------------------------------------------------
# Get uploaded files
# ------------------------------------------------------
@app.route("/get_uploaded_files", methods=["GET"])
def uploaded_files():
    """API endpoint to return uploaded files and AWS IoT endpoint."""
    files, aws_iot_endpoint = get_uploaded_files()  # Get files + endpoint dynamically
    return jsonify({"files": files, "aws_endpoint": aws_iot_endpoint})

# ------------------------------------------------------
# Logout function: 
# -> Log out dashboard and return login screen
# ------------------------------------------------------
@app.route("/logout")
def logout():
    session.pop("authenticated", None)
    return redirect(url_for("login"))


if __name__ == "__main__":
    app.run(host="0.0.0.0", port = 8500, debug=True, use_reloader=False)
    


    
    
    

