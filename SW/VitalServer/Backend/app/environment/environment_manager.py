import os
import importlib
import re
import json
import app.environment.environment as environment

import redis

# Connect to Redis
r = redis.Redis(host='localhost', port=6379, db=0)

ENV_FILE_PATH = os.path.join(os.path.dirname(__file__), "environment.py")
UPLOAD_FOLDER = "Backend/app/upload"

# -------------------------------------------------
# Convert Python values to the correct format for 
# environment.py.
# - Integers/Floats: No quotes
# - Strings: Enclosed in quotes ""
# - None: Written as None (not "None")
# -------------------------------------------------
def _serialize_value(value):
    if value is None: 
        return "None"
    if isinstance(value, (int, float)):
        return str(value)
    else:  
        return f'"{value}"'
    
# -------------------------------------------------
# Finds a line that starts with 'var_name =', and  
# replaces its value with new_value.
# -------------------------------------------------
def _update_env_line(lines, var_name, new_value):
    pattern = re.compile(rf"^\s*{var_name}\s*=\s*")
    replaced = False
    
    for i, line in enumerate(lines):
        if pattern.match(line):  
            # Found the line for var_name
            lines[i] = f"{var_name} = {_serialize_value(new_value)}\n"
            replaced = True
            break

    return lines

# -------------------------------------------------
# Return the latest USERNAME and PASSWORD from 
# environment.py.
# -------------------------------------------------
def get_login_info():
    importlib.reload(environment)
    return {"login_username": environment.LOGIN_USERNAME, "login_password": environment.LOGIN_PASSWORD}

# -------------------------------------------------
# Update USERNAME and PASSWORD in environment.py
# -------------------------------------------------
def update_info_user(username, password):
    with open(ENV_FILE_PATH, "r", encoding="utf-8") as f:
        lines = f.readlines()

    lines = _update_env_line(lines, "USERNAME", username)
    lines = _update_env_line(lines, "PASSWORD", password)

    with open(ENV_FILE_PATH, "w", encoding="utf-8") as f:
        f.writelines(lines)
        f.flush()  # Force Python to flush its internal buffers
        os.fsync(f.fileno())  # Force the OS to write buffers to the disk

    # Reload to updated values
    importlib.reload(environment)

# -------------------------------------------------
# Return the latest AUTH_TOKEN from environment.py
# -------------------------------------------------
def get_auth_token():
    importlib.reload(environment)
    return environment.AUTH_TOKEN

# -------------------------------------------------
# Return the latest Facility Lists from environment.py
# -------------------------------------------------
def get_facility_list():
    
    facility_list = r.get("facility_list")
    if facility_list:
        return json.loads(facility_list.decode())
    
    importlib.reload(environment)
    return environment.FACILITY_LIST

# -------------------------------------------------
# Update the facility list in environment.py
# -------------------------------------------------
def update_facility_list(facilities):

    # Update to Redis
    r.set("facility_list", json.dumps(facilities))

    with open(ENV_FILE_PATH, "r", encoding="utf-8") as f:
        lines = f.readlines()

    # Update the FACILITY_LIST entry in environment.py
    updated = False
    for i, line in enumerate(lines):
        if line.strip().startswith("FACILITY_LIST"):
            lines[i] = f'FACILITY_LIST = {facilities}\n'
            updated = True
            break

    # If FACILITY_LIST is not found, append it
    if not updated:
        lines.append(f'FACILITY_LIST = {facilities}\n')

    with open(ENV_FILE_PATH, "w", encoding="utf-8") as f:
        f.writelines(lines)
        f.flush()  # Force Python to flush its internal buffers
        os.fsync(f.fileno())  # Force the OS to write buffers to the disk

    # Reload to updated values
    importlib.reload(environment)

# -------------------------------------------------
# Update FACILITY_ID in environment.py.
# -------------------------------------------------
def update_facility_id(token):
    with open(ENV_FILE_PATH, "r", encoding="utf-8") as f:
        lines = f.readlines()

    token = int(token)
    lines = _update_env_line(lines, "FACILITY_ID", token)

    r.set("facility_id", token)

    with open(ENV_FILE_PATH, "w", encoding="utf-8") as f:
        f.writelines(lines)
        f.flush()  # Force Python to flush its internal buffers
        os.fsync(f.fileno())  # Force the OS to write buffers to the disk

    # Reload to updated  
    importlib.reload(environment)

# -------------------------------------------------
# Return the latest AWS_IOT_ENDPOINT 
# from environment.py
# -------------------------------------------------
def get_aws_endpoint():

    aws_endpoint = r.get("aws_iot_endpoint")
    if aws_endpoint:
        return aws_endpoint.decode()
    
    importlib.reload(environment)
    return environment.AWS_IOT_ENDPOINT


# -------------------------------------------------
# Update AWS_IOT_ENDPOINT in environment.py.
# -------------------------------------------------
def update_aws_endpoint(token):

    # Update to Redis
    r.set("aws_iot_endpoint", token)

    with open(ENV_FILE_PATH, "r", encoding="utf-8") as f:
        lines = f.readlines()

    lines = _update_env_line(lines, "AWS_IOT_ENDPOINT", token)

    with open(ENV_FILE_PATH, "w", encoding="utf-8") as f:
        f.writelines(lines)
        f.flush()  # Force Python to flush its internal buffers
        os.fsync(f.fileno())  # Force the OS to write buffers to the disk

    # Reload the updated environment variables
    importlib.reload(environment)

# -------------------------------------------------
# Return the latest Facility Lists from environment.py
# -------------------------------------------------
def get_allow_extensions():
    importlib.reload(environment)
    return environment.ALLOWED_EXTENSIONS

# ------------------------------------------------- 
# Update environment.py with new file paths
# ------------------------------------------------- 
def update_environment_file(new_file_path, file_type):
    
    # Convert to relative path
    relative_path = os.path.relpath(new_file_path, start=os.getcwd())  

    with open(ENV_FILE_PATH, "r") as file:
        lines = file.readlines()

    # Identify which variable needs updating
    updated_lines = []
    for line in lines:
        if file_type == "root_ca" and line.startswith("root_ca_path"):
            updated_lines.append(f'root_ca_path = "{relative_path}"\n')
            r.set("root_ca_path", relative_path)
        elif file_type == "private_key" and line.startswith("private_key_path"):
            updated_lines.append(f'private_key_path = "{relative_path}"\n')
            r.set("private_key_path", relative_path)
        elif file_type == "cert" and line.startswith("cert_path"):
            updated_lines.append(f'cert_path = "{relative_path}"\n')
            r.set("cert_path", relative_path)
        else:
            updated_lines.append(line)  # Keep other lines unchanged

    # Write the updated content back to the file
    with open(ENV_FILE_PATH, "w") as file:
        file.writelines(updated_lines)

    # Reload to get updated values
    importlib.reload(environment) 

    return relative_path  # Return the updated relative path

# -------------------------------------------------
# Return path of aws certificates
# -------------------------------------------------
def get_root_ca_path():
    root_ca = r.get("root_ca_path")
    if root_ca:
        return root_ca.decode()
    importlib.reload(environment)
    return environment.root_ca_path

def get_private_key_path():
    private_key = r.get("private_key_path")
    if private_key:
        return private_key.decode()
    importlib.reload(environment)
    return environment.private_key_path

def get_cert_path():
    cert = r.get("cert_path")
    if cert:
        return cert.decode()
    importlib.reload(environment)
    return environment.cert_path

# ------------------------------------------------- 
# Retrieve files with the required extensions 
# from the upload folder
# -------------------------------------------------
def get_uploaded_files():
    
    # Reload to get the latest AWS IoT Endpoint dynamically
    importlib.reload(environment) 
    
    files = [
        f for f in os.listdir(UPLOAD_FOLDER) 
        if os.path.isfile(os.path.join(UPLOAD_FOLDER, f)) and 
        any(f.endswith(ext) for ext in environment.ALLOWED_EXTENSIONS)
    ]
    return files, environment.AWS_IOT_ENDPOINT

# -------------------------------------------------
# Return MQTT PORT
# -------------------------------------------------
def get_MQTT_PORT():
    importlib.reload(environment)
    return environment.MQTT_PORT

# -------------------------------------------------
# Return START_SERVER
# -------------------------------------------------
def get_start_server():
    # Get server status from Redis instead of environment
    status = r.get("tcp_server_status").decode()
    return status == "running"  # Returns True/False

# -------------------------------------------------
# Return Facility ID
# -------------------------------------------------
def get_facility_id():

    facility_id = r.get("facility_id")
    if facility_id:
        return facility_id.decode()
    
    importlib.reload(environment)
    return environment.FACILITY_ID

# -------------------------------------------------
# Retrieve the Logging Configuration from 
# environment.py 
# ------------------------------------------------- 
def get_logging_configuration():
    # Reload environment to get updated values
    max_bytes = r.get("max_bytes")
    backup_count = r.get("backup_count")
    if max_bytes and backup_count:
        # Convert decoded strings to integers
        return int(max_bytes.decode()), int(backup_count.decode())
    
    importlib.reload(environment)  
    # Ensure environment variables are integers
    return int(environment.MAX_BYTES), int(environment.BACKUP_COUNT)

# -------------------------------------------------
# Update Logging Configuration in environment.py.
# -------------------------------------------------    
def update_logging_configuration(max_bytes, backup_count):
    with open(ENV_FILE_PATH, "r", encoding="utf-8") as f:
        lines = f.readlines()

    # Find the "keyword" and replace None
    lines = _update_env_line(lines, "MAX_BYTES", max_bytes)
    lines = _update_env_line(lines, "BACKUP_COUNT", backup_count)

    # Update to Redis
    r.set("max_bytes", max_bytes)
    r.set("backup_count", backup_count)
    
    # Open and Write in environment.py
    with open(ENV_FILE_PATH, "w", encoding="utf-8") as f:
        f.writelines(lines)
        f.flush()  # Force Python to flush its internal buffers
        os.fsync(f.fileno())  # Force the OS to write buffers to the disk

    # Reload to get updated values
    importlib.reload(environment) 

# ------------------------------------------------- 
# Reload value of variables in Environment.py
# -------------------------------------------------
def reload_environment():
    # Reload environment.py to get the latest updates
    importlib.reload(environment)