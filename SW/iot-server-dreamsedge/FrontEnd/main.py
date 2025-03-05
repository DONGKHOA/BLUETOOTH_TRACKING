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
    get_status_tcp_server
)

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
# -> Received username and password and update 
# to Environment.py
# -> Get auth_token and update to Environment.py
# --------------------------------------------------
@app.route("/login", methods=["GET", "POST"])
def login():

    if request.method == "POST":
        username = request.form.get("username", "").strip()
        password = request.form.get("password", "").strip()

        print(f"[INFO] Received login request - Username: {username}, Password: {'*' * len(password)}")

        update_info_user(username, password)

        timeout = 7  # Stop waiting after 7 seconds
        start_time = time.time()
        
        # --------------------------------------------------
        # Check for AUTH_TOKEN every second
        # If received, set session and redirect to dashboard
        # --------------------------------------------------
        while True:
            auth_token = get_auth_token()

            if auth_token:
                print(f"[INFO] AUTH_TOKEN received: {auth_token}")
                session["authenticated"] = True  # Set session
                # return redirect(url_for("dashboard"))
                return redirect(url_for("server_status"))

            if time.time() - start_time > timeout:
                print("Timeout: AUTH_TOKEN not received.")
                reset_credentials()
                break

            print("[INFO] Waiting for AUTH_TOKEN from BackEnd...")
            
            time.sleep(1)
            
        return render_template("login.html", error="Login failed. Please try again.")
    return render_template("login.html")

@app.route("/server_status")
def server_status():
    # Optional: check if user is authenticated
    if not session.get("authenticated"):
        return redirect(url_for("login"))
    
    # Render the configuration.html template
    return render_template("server_status.html")

# --------------------------------------------------
# Dashboard function: 
# -> Get facility lists and show in GUI
# --------------------------------------------------
@app.route("/dashboard", methods=["GET"])
def dashboard():
    if not session.get("authenticated"):
        return redirect(url_for("login"))

    # Check if Facility List Fetching Should Be Skipped
    skip_fetch = request.args.get("skip_fetch", "false").lower() == "true"
    if not skip_fetch:
        timeout = 3  # Stop waiting after 7 seconds
        start_time = time.time()

        # Fetch stored API data from environment.py
        while True:
            api_data = get_facility_list()
            
            if api_data and "data" in api_data and len(api_data["data"]) > 0:
                print("[INFO] Facility list updated, rendering dashboard...")
                break

            if time.time() - start_time > timeout:
                print("Timeout: Can not get Facility List from HTTP.")
                reset_credentials()
                return redirect(url_for("login"))
                
            time.sleep(1)
        
    else:
        # Reset the api_data variable, not rest FACILITY_LIST (avoid error - NoneType)
        api_data = {"data": [], "count": 0}
        

    return render_template("dashboard.html", data=api_data["data"], count=api_data["count"])

# --------------------------------------------------
# Upload Certificates function: 
# -> Get 3 files of certificates and stored it in 
# upload folder
# --------------------------------------------------
@app.route("/upload_cert", methods=["POST"])
def upload_cert():
    if not session.get("authenticated"):
        return redirect(url_for("login"))

    saved_files = []
    for key in ['file1', 'file2', 'file3']:
        file = request.files.get(key)

        if not file or file.filename == '':
            continue

        try:
            file_path = os.path.join(app.config['UPLOAD_FOLDER'], file.filename)

            if os.path.exists(file_path):
                os.remove(file_path)

            file.save(file_path)
            saved_files.append(file.filename)   

        except Exception as e:
            flash(f"Error: {str(e)}", "error")
            return redirect(url_for("dashboard"))

    if saved_files:
        flash(f"Uploaded files: {', '.join(saved_files)}", "success")
    else:
        flash("No files selected!", "error")

    return redirect(url_for("dashboard", skip_fetch = "true"))

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
    if not session.get("authenticated"):
        return redirect(url_for("login"))

    try:
        new_endpoint = request.form.get("aws_iot_endpoint")

        if not new_endpoint:
            return "Invalid input", 400

        update_aws_endpoint(new_endpoint)

        return "Success", 200

    except Exception as e:
        print(f"Error updating AWS IoT Endpoint: {str(e)}")
        return f"Error: {str(e)}", 500

# ------------------------------------------------------
# Check AWS Connection function: 
# -> Check connection with new AWS IoT Endpoint
# ------------------------------------------------------
@app.route("/check_aws_connection", methods=["GET"])
def check_aws_connection():
    
    if not session.get("authenticated"):
        return redirect(url_for("login"))
    
    update_start_check_aws(True)
    update_connection_aws(None)     
    
    timeout = 10  # Stop waiting after 7 seconds
    start_time = time.time()
    
    while time.time() - start_time < timeout:
        
        connection_aws = get_connection_aws()
        if connection_aws is True:
            update_connection_aws(None) 
            return jsonify({"status": "success"})
        elif connection_aws is False:
            update_connection_aws(None) 
            return jsonify({"status": "failure"})
        
        time.sleep(0.5)
    
    return jsonify({"status": "timeout"})

# ------------------------------------------------------
# Check server status function: 
# -> Check status of TCP server and AWS server
# ------------------------------------------------------
@app.route("/check_status_server", methods=["GET"])
def check_status_server():
    
    if not session.get("authenticated"):
        return redirect(url_for("login"))
    
    # update_status_tcp_server(None)
    # update_status_aws_server(None)
    
    timeout = 10  # Stop waiting after 10 seconds
    start_time = time.time()
    
    while time.time() - start_time < timeout:
        tcp_status = get_status_tcp_server()  
        aws_status = get_status_aws_server()
        print("tcp status", tcp_status)

        if tcp_status is not None and aws_status is not None:
            return jsonify({
                "tcp_status": "connected" if tcp_status else "disconnected",
                "aws_status": "connected" if aws_status else "disconnected"
            })

        time.sleep(0.5) 
        
    return jsonify({
        "tcp_status": "timeout",
        "aws_status": "timeout"
    })
    
# ------------------------------------------------------
# Logout function: 
# -> Log out dashboard and return login screen
# ------------------------------------------------------
@app.route("/logout")
def logout():
    session.pop("authenticated", None)
    
    reset_credentials() 
    
    return redirect(url_for("login"))


if __name__ == "__main__":
    app.run(debug=True, use_reloader=False)
    


    
    
    

