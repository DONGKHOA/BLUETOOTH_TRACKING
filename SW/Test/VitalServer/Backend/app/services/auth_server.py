import time
import socket
import app.environment.environment_manager as environment_manager
import app.services.http_client as http_client
import uuid
from flask import session
import redis
import json

from flask import session

r = redis.Redis(host='localhost', port=6379, db=0)

admin_session_token = None
# --------------------------------------------------
# Create a new session token to enforce single login
# for the Admin account.
# --------------------------------------------------
def update_admin_session():
    global admin_session_token

    environment_manager.reload_environment()

    if admin_session_token:
        print("[WARNING] An admin is already logged in! Forcing logout of previous session.")
        clear_admin_session_token()  # Logout the previous session

    new_session_token = str(uuid.uuid4())  
    admin_session_token = new_session_token
    return new_session_token

# --------------------------------------------------
# Retrieve the current Admin session token.
# --------------------------------------------------
def get_admin_session_token():
    global admin_session_token
    return admin_session_token

# --------------------------------------------------
# Clear the Admin session token (force logout).
# --------------------------------------------------
def clear_admin_session_token():
    global admin_session_token
    admin_session_token = None
    print("[INFO] Admin session cleared. Previous admin is logged out.")

# --------------------------------------------------
# Validate Admin Session - Used in Restricted Routes
# --------------------------------------------------
def validate_admin_session():
    global admin_session_token

    environment_manager.reload_environment()

    if not admin_session_token:  # If no active admin session, allow login
        return True  

    if "session_token" in session and session["session_token"] == admin_session_token:
        return True  # Admin session is still valid

    return False  # Another admin has logged in

# --------------------------------------------------
# Validate User Session
# --------------------------------------------------
def validate_user_session():

    environment_manager.reload_environment()

    if not session.get("authenticated"):  
        # If the user never logged in, return valid=True (don't force logout)
        return True, None  

    # If Admin, check for single-session enforcement
    if session.get("username") == "admin" and not validate_admin_session():
        print("[WARNING] Admin session invalid. Logging out user.")
        return False, "Your session has been terminated due to another admin login."

    return True, None  # Session is valid

# --------------------------------------------------
# Logout the current session
# --------------------------------------------------
def logout_user():
    global admin_session_token

    environment_manager.reload_environment()

    # If the user is admin, clear the session token
    if session.get("username") == "admin" and session.get("session_token") == admin_session_token:
        print("[DEBUG] Admin logging out, clearing admin_session_token.")
        admin_session_token = None

    # Remove all session variables
    session.pop("authenticated", None)
    session.pop("session_token", None)
    session.pop("username", None)

# --------------------------------------------------
# Handles authentication process and updates session
# --------------------------------------------------
def authenticate_user(username, password):

    auth_token = http_client.login_access_token(username, password)

    if auth_token:

        r.set("auth_token", auth_token)
        print(f"[INFO] AUTH_TOKEN received for user '{username}': {auth_token}")

        return True, "/server_configuration"
    else:

        r.set("auth_token", "")
        print(f"[ERROR] Authentication failed: Invalid username/password for '{username}'")

        return False, "/server_configuration"

        
def facility_list():

    auth_token = r.get("auth_token") or b""

    if not auth_token:
        print("[ERROR] Authentication token is missing. Cannot fetch facilities.")
        return None

    facilities = http_client.get_facility_list()

    if facilities:
        environment_manager.update_facility_list(facilities)
        # system_log.log_to_redis("[INFO] Successfully retrieved facility list.")
        print("[INFO] Successfully retrieved facility list.")
        return facilities   
    else:
        # system_log.log_to_redis("[ERROR] No facilities found or API request failed.")
        print("[ERROR] No facilities found or API request failed.")
        return None

# ----------------------------------------------- 
# Check connection AWS IoT with TCP 
# -----------------------------------------------
def check_tcp_connection(host, port, timeout=5):
    try:
        # Resolve hostname
        addr_info = socket.getaddrinfo(host, port)
        if not addr_info:
            return False

        # Attempt to create a socket connection
        with socket.create_connection((host, port), timeout=timeout):
            return True
    except socket.gaierror:
        return False
    except (socket.timeout, ConnectionRefusedError, OSError):
        return False

# -----------------------------------------------
# Check the connection with AWS IoT Endpoint 
# before connect MQTT
# -----------------------------------------------
async def process_aws_connection_check():

    environment_manager.reload_environment()

    result = check_tcp_connection(environment_manager.get_aws_endpoint(), environment_manager.get_MQTT_PORT())

    if result:
        return "success"
    else:
        return "failure"

# -----------------------------------------------
# Check TCP Server & AWS IoT Status
# -----------------------------------------------
def check_tcp_aws_status():

    timeout = 10
    start_time = time.time()

    while time.time() - start_time < timeout:
        
        tcp_status = r.get("tcp_server_status") or b"unknown"
        aws_status = r.get("aws_server_status") or b"unknown"

        if tcp_status != b"unknown" and aws_status != b"unknown":
            return {
                "tcp_status": tcp_status.decode(),
                "aws_status": aws_status.decode()
            }
        time.sleep(0.5)

    return {
        "tcp_status": "timeout",
        "aws_status": "timeout"
    }
