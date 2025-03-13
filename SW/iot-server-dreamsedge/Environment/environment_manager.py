import os
import importlib
import re
import json
import environment 

ENV_FILE_PATH = os.path.join(os.path.dirname(__file__), "environment.py")
UPLOAD_FOLDER = "Environment/upload"

# -------------------------------------------------
# Reset Authenticate Token
# -------------------------------------------------
def reset_auth_token():
    # Read the data in environment.py
    with open(ENV_FILE_PATH, "r", encoding="utf-8") as f:
        lines = f.readlines()

    # Find the "keyword" and replace None
    lines = _update_env_line(lines, "AUTH_TOKEN", None)

    # Open and Write in environment.py
    with open(ENV_FILE_PATH, "w", encoding="utf-8") as f:
        f.writelines(lines)

    # Reload to get updated values
    importlib.reload(environment)

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
# Reset USERNAME, PASSWORD, AUTH_TOKEN, 
# FACILITY_LIST to None.
# -------------------------------------------------
def reset_credentials():
    # Read the data in environment.py
    with open(ENV_FILE_PATH, "r", encoding="utf-8") as f:
        lines = f.readlines()

    # Find the "keyword" and replace None
    # lines = _update_env_line(lines, "USERNAME", None)
    # lines = _update_env_line(lines, "PASSWORD", None)
    # lines = _update_env_line(lines, "AUTH_TOKEN", None)
    # lines = _update_env_line(lines, "FACILITY_LIST", None)
    # lines = _update_env_line(lines, "FACILITY_ID", None)
    # lines = _update_env_line(lines, "AWS_IOT_ENDPOINT", None)
    # lines = _update_env_line(lines, "CONNECTION_AWS", None)
    # lines = _update_env_line(lines, "START_CHECK_AWS", None)
    
    
    # Open and Write in environment.py
    with open(ENV_FILE_PATH, "w", encoding="utf-8") as f:
        f.writelines(lines)

    # Reload to get updated values
    importlib.reload(environment)

# -------------------------------------------------
# Update USERNAME and PASSWORD in environment.py
# -------------------------------------------------
def update_temp_info_user(username, password):
    with open(ENV_FILE_PATH, "r", encoding="utf-8") as f:
        lines = f.readlines()

    lines = _update_env_line(lines, "TEMP_USERNAME", username)
    lines = _update_env_line(lines, "TEMP_PASSWORD", password)

    with open(ENV_FILE_PATH, "w", encoding="utf-8") as f:
        f.writelines(lines)

    # Reload to updated values
    importlib.reload(environment)

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

    # Reload to updated values
    importlib.reload(environment)

# -------------------------------------------------
# Return the latest USERNAME and PASSWORD from 
# environment.py.
# -------------------------------------------------
def get_login_info():
    importlib.reload(environment)
    return {"login_username": environment.LOGIN_USERNAME, "login_password": environment.LOGIN_PASSWORD}

# -------------------------------------------------
# Return the latest USERNAME and PASSWORD from 
# environment.py.
# -------------------------------------------------
def get_info_user():
    importlib.reload(environment)
    return {"username": environment.USERNAME, "password": environment.PASSWORD}

# -------------------------------------------------
# Return the latest USERNAME and PASSWORD from 
# environment.py.
# -------------------------------------------------
def get_temp_info_user():
    importlib.reload(environment)
    return {"temp_username": environment.TEMP_USERNAME, "temp_password": environment.TEMP_PASSWORD}

# -------------------------------------------------
# Return the latest AUTH_TOKEN from environment.py
# -------------------------------------------------
def get_auth_token():
    importlib.reload(environment)
    return environment.AUTH_TOKEN

# -------------------------------------------------
# Return the latest TEMP_AUTH_TOKEN from environment.py
# -------------------------------------------------
def get_temp_auth_token():
    importlib.reload(environment)
    return environment.TEMP_AUTH_TOKEN

# -------------------------------------------------
# Update AUTH_TOKEN in environment.py.
# -------------------------------------------------
def update_temp_auth_token(token):
    with open(ENV_FILE_PATH, "r", encoding="utf-8") as f:
        lines = f.readlines()

    lines = _update_env_line(lines, "TEMP_AUTH_TOKEN", token)

    with open(ENV_FILE_PATH, "w", encoding="utf-8") as f:
        f.writelines(lines)

    # Reload so environment.AUTH_TOKEN is updated
    importlib.reload(environment)

# -------------------------------------------------
# Update AUTH_TOKEN in environment.py.
# -------------------------------------------------
def update_auth_token(token):
    with open(ENV_FILE_PATH, "r", encoding="utf-8") as f:
        lines = f.readlines()

    lines = _update_env_line(lines, "AUTH_TOKEN", token)

    with open(ENV_FILE_PATH, "w", encoding="utf-8") as f:
        f.writelines(lines)

    # Reload so environment.AUTH_TOKEN is updated
    importlib.reload(environment)

# -------------------------------------------------
# Store the facility list in environment.py
# -------------------------------------------------
def update_facility_list(facilities):
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

    # Reload to updated values
    importlib.reload(environment)  
    
# -------------------------------------------------
# Retrieve the facility list from environment.py 
# -------------------------------------------------
def get_facility_list():
    # Reload environment to get updated values
    importlib.reload(environment)  
    return environment.FACILITY_LIST

# -------------------------------------------------
# Update FACILITY_ID in environment.py.
# -------------------------------------------------
def update_facility_id(token):
    with open(ENV_FILE_PATH, "r", encoding="utf-8") as f:
        lines = f.readlines()

    token = int(token)
    lines = _update_env_line(lines, "FACILITY_ID", token)

    
    with open(ENV_FILE_PATH, "w", encoding="utf-8") as f:
        f.writelines(lines)

    # Reload to updated  
    importlib.reload(environment)
    
# -------------------------------------------------
# Retrieve the facility id from environment.py 
# -------------------------------------------------
def get_facility_id():
    # Reload environment to get updated values
    importlib.reload(environment)  
    return environment.FACILITY_ID

# -------------------------------------------------
# Retrieve the aws iot endpoint from environment.py 
# -------------------------------------------------
def get_aws_endpoint():
    # Reload environment to get updated values
    importlib.reload(environment)  
    return environment.AWS_IOT_ENDPOINT

# -------------------------------------------------
# Update AWS_IOT_ENDPOINT in environment.py.
# -------------------------------------------------
def update_aws_endpoint(token):
    with open(ENV_FILE_PATH, "r", encoding="utf-8") as f:
        lines = f.readlines()

    lines = _update_env_line(lines, "AWS_IOT_ENDPOINT", token)

    with open(ENV_FILE_PATH, "w", encoding="utf-8") as f:
        f.writelines(lines)

    # Reload the updated environment variables
    importlib.reload(environment)

# -------------------------------------------------
# Retrieve the CONNECTION_AWS from environment.py 
# -------------------------------------------------
def get_connection_aws():
    # Reload environment to get updated values
    importlib.reload(environment)  
    return environment.CONNECTION_AWS

# -------------------------------------------------
# Update CONNECTION_AWS in environment.py.
# -------------------------------------------------    
def update_connection_aws(value):
    with open(ENV_FILE_PATH, "r", encoding="utf-8") as f:
        lines = f.readlines()

    # Find the "keyword" and replace None
    lines = _update_env_line(lines, "CONNECTION_AWS", value)
    
    # Open and Write in environment.py
    with open(ENV_FILE_PATH, "w", encoding="utf-8") as f:
        f.writelines(lines)

    # Reload to get updated values
    importlib.reload(environment)

# -------------------------------------------------
# Retrieve the START_CHECK_AWS from environment.py 
# -------------------------------------------------
def get_start_check_aws():
    # Reload environment to get updated values
    importlib.reload(environment)  
    return environment.START_CHECK_AWS

# -------------------------------------------------
# Update START_CHECK_AWS in environment.py.
# -------------------------------------------------    
def update_start_check_aws(value):
    with open(ENV_FILE_PATH, "r", encoding="utf-8") as f:
        lines = f.readlines()

    # Find the "keyword" and replace None
    lines = _update_env_line(lines, "START_CHECK_AWS", value)
    
    # Open and Write in environment.py
    with open(ENV_FILE_PATH, "w", encoding="utf-8") as f:
        f.writelines(lines)

    # Reload to get updated values
    importlib.reload(environment)

# -------------------------------------------------
# Retrieve the TCP_SERVER_STATUS from environment.py 
# -------------------------------------------------    
def get_status_tcp_server():
    # Reload environment to get updated values
    importlib.reload(environment)  
    return environment.TCP_SERVER_STATUS

# -------------------------------------------------
# Update TCP_SERVER_STATUS in environment.py.
# -------------------------------------------------    
def update_status_tcp_server(value):
    with open(ENV_FILE_PATH, "r", encoding="utf-8") as f:
        lines = f.readlines()

    # Find the "keyword" and replace None
    lines = _update_env_line(lines, "TCP_SERVER_STATUS", value)
    
    # Open and Write in environment.py
    with open(ENV_FILE_PATH, "w", encoding="utf-8") as f:
        f.writelines(lines)

    # Reload to get updated values
    importlib.reload(environment)  
    
# -------------------------------------------------
# Retrieve the AWS_SERVER_STATUS from environment.py 
# -------------------------------------------------    
def get_status_aws_server():
    # Reload environment to get updated values
    importlib.reload(environment)  
    return environment.AWS_SERVER_STATUS

# -------------------------------------------------
# Update AWS_SERVER_STATUS in environment.py.
# -------------------------------------------------    
def update_status_aws_server(value):
    with open(ENV_FILE_PATH, "r", encoding="utf-8") as f:
        lines = f.readlines()

    # Find the "keyword" and replace None
    lines = _update_env_line(lines, "AWS_SERVER_STATUS", value)
    
    # Open and Write in environment.py
    with open(ENV_FILE_PATH, "w", encoding="utf-8") as f:
        f.writelines(lines)

    # Reload to get updated values
    importlib.reload(environment)  

# -------------------------------------------------
# Retrieve the START_SERVER from environment.py 
# -------------------------------------------------    
def get_start_server():
    # Reload environment to get updated values
    importlib.reload(environment)  
    return environment.START_SERVER

# -------------------------------------------------
# Update START_SERVER in environment.py.
# -------------------------------------------------    
def update_start_server(value):
    with open(ENV_FILE_PATH, "r", encoding="utf-8") as f:
        lines = f.readlines()

    # Find the "keyword" and replace None
    lines = _update_env_line(lines, "START_SERVER", value)
    
    # Open and Write in environment.py
    with open(ENV_FILE_PATH, "w", encoding="utf-8") as f:
        f.writelines(lines)

    # Reload to get updated values
    importlib.reload(environment)  
    
# -------------------------------------------------
# Retrieve the STOP_SERVER from environment.py 
# -------------------------------------------------    
def get_stop_server():
    # Reload environment to get updated values
    importlib.reload(environment)  
    return environment.STOP_SERVER

# -------------------------------------------------
# Update STOP_SERVER in environment.py.
# -------------------------------------------------    
def update_stop_server(value):
    with open(ENV_FILE_PATH, "r", encoding="utf-8") as f:
        lines = f.readlines()

    # Find the "keyword" and replace None
    lines = _update_env_line(lines, "STOP_SERVER", value)
    
    # Open and Write in environment.py
    with open(ENV_FILE_PATH, "w", encoding="utf-8") as f:
        f.writelines(lines)

    # Reload to get updated values
    importlib.reload(environment)  

# -------------------------------------------------
# Retrieve the RECONFIG_SERVER from environment.py 
# -------------------------------------------------    
def get_reconfig_server():
    # Reload environment to get updated values
    importlib.reload(environment)  
    return environment.RECONFIG_SERVER
 
# -------------------------------------------------
# Update RECONFIG_SERVER in environment.py.
# -------------------------------------------------   
def update_reconfig_server(value):
    with open(ENV_FILE_PATH, "r", encoding="utf-8") as f:
        lines = f.readlines()

    # Find the "keyword" and replace None
    lines = _update_env_line(lines, "RECONFIG_SERVER", value)
    
    # Open and Write in environment.py
    with open(ENV_FILE_PATH, "w", encoding="utf-8") as f:
        f.writelines(lines)

    # Reload to get updated values
    importlib.reload(environment)  

# -------------------------------------------------
# Retrieve the INFO_UPDATED from environment.py 
# -------------------------------------------------    
def get_reconfig_info():
    # Reload environment to get updated values
    importlib.reload(environment)  
    return environment.INFO_UPDATED
 
# -------------------------------------------------
# Update INFO_UPDATED in environment.py.
# -------------------------------------------------   
def update_reconfig_info(value):
    with open(ENV_FILE_PATH, "r", encoding="utf-8") as f:
        lines = f.readlines()

    # Find the "keyword" and replace None
    lines = _update_env_line(lines, "INFO_UPDATED", value)
    
    # Open and Write in environment.py
    with open(ENV_FILE_PATH, "w", encoding="utf-8") as f:
        f.writelines(lines)

    # Reload to get updated values
    importlib.reload(environment)  

# -------------------------------------------------
# Retrieve the FACILITY_LIST_UPDATED from environment.py 
# -------------------------------------------------    
def get_reconfig_facility_list():
    # Reload environment to get updated values
    importlib.reload(environment)  
    return environment.FACILITY_LIST_UPDATED
 
# -------------------------------------------------
# Update FACILITY_LIST_UPDATED in environment.py.
# -------------------------------------------------   
def update_reconfig_facility_list(value):
    with open(ENV_FILE_PATH, "r", encoding="utf-8") as f:
        lines = f.readlines()

    # Find the "keyword" and replace None
    lines = _update_env_line(lines, "FACILITY_LIST_UPDATED", value)
    
    # Open and Write in environment.py
    with open(ENV_FILE_PATH, "w", encoding="utf-8") as f:
        f.writelines(lines)

    # Reload to get updated values
    importlib.reload(environment)  

# -------------------------------------------------
# Retrieve the FACILITY_ID_UPDATED from environment.py 
# -------------------------------------------------    
def get_reconfig_facility_id():
    # Reload environment to get updated values
    importlib.reload(environment)  
    return environment.FACILITY_ID_UPDATED
 
# -------------------------------------------------
# Update FACILITY_ID_UPDATED in environment.py.
# -------------------------------------------------   
def update_reconfig_facility_id(value):
    with open(ENV_FILE_PATH, "r", encoding="utf-8") as f:
        lines = f.readlines()

    # Find the "keyword" and replace None
    lines = _update_env_line(lines, "FACILITY_ID_UPDATED", value)
    
    # Open and Write in environment.py
    with open(ENV_FILE_PATH, "w", encoding="utf-8") as f:
        f.writelines(lines)

    # Reload to get updated values
    importlib.reload(environment)  

# -------------------------------------------------
# Retrieve the AWS_IOT_ENDPOINT_UPDATED from 
# environment.py 
# -------------------------------------------------    
def get_reconfig_aws_endpoint():
    # Reload environment to get updated values
    importlib.reload(environment)  
    return environment.AWS_IOT_ENDPOINT_UPDATED
 
# -------------------------------------------------
# Update AWS_IOT_ENDPOINT_UPDATED in environment.py.
# -------------------------------------------------   
def update_reconfig_aws_endpoint(value):
    with open(ENV_FILE_PATH, "r", encoding="utf-8") as f:
        lines = f.readlines()

    # Find the "keyword" and replace None
    lines = _update_env_line(lines, "AWS_IOT_ENDPOINT_UPDATED", value)
    
    # Open and Write in environment.py
    with open(ENV_FILE_PATH, "w", encoding="utf-8") as f:
        f.writelines(lines)

    # Reload to get updated values
    importlib.reload(environment)  
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
        elif file_type == "private_key" and line.startswith("private_key_path"):
            updated_lines.append(f'private_key_path = "{relative_path}"\n')
        elif file_type == "cert" and line.startswith("cert_path"):
            updated_lines.append(f'cert_path = "{relative_path}"\n')
        else:
            updated_lines.append(line)  # Keep other lines unchanged

    # Write the updated content back to the file
    with open(ENV_FILE_PATH, "w") as file:
        file.writelines(updated_lines)

    # Reload to get updated values
    importlib.reload(environment) 

    return relative_path  # Return the updated relative path
    
def get_uploaded_files():
    """Retrieve files with the required extensions from the upload folder."""
    
    # Reload to get the latest AWS IoT Endpoint dynamically
    importlib.reload(environment) 
    
    files = [
        f for f in os.listdir(UPLOAD_FOLDER) 
        if os.path.isfile(os.path.join(UPLOAD_FOLDER, f)) and 
        any(f.endswith(ext) for ext in environment.ALLOWED_EXTENSIONS)
    ]
    return files, environment.AWS_IOT_ENDPOINT
    
# ------------------------------------------------- 
# Reload value of variables in Environment.py
# -------------------------------------------------
def reload_environment():
    # Reload environment.py to get the latest updates
    importlib.reload(environment)

