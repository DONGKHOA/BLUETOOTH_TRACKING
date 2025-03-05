import os
import importlib
import re
import json
import environment 

ENV_FILE_PATH = os.path.join(os.path.dirname(__file__), "environment.py")

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
    lines = _update_env_line(lines, "USERNAME", None)
    lines = _update_env_line(lines, "PASSWORD", None)
    lines = _update_env_line(lines, "AUTH_TOKEN", None)
    lines = _update_env_line(lines, "FACILITY_LIST", None)
    lines = _update_env_line(lines, "FACILITY_ID", None)
    lines = _update_env_line(lines, "AWS_IOT_ENDPOINT", None)
    lines = _update_env_line(lines, "CONNECTION_AWS", None)
    lines = _update_env_line(lines, "START_CHECK_AWS", None)
    
    
    # Open and Write in environment.py
    with open(ENV_FILE_PATH, "w", encoding="utf-8") as f:
        f.writelines(lines)

    # Reload to get updated values
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
def get_info_user():
    importlib.reload(environment)
    return {"username": environment.USERNAME, "password": environment.PASSWORD}

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
# Return the latest AUTH_TOKEN from environment.py
# -------------------------------------------------
def get_auth_token():
    importlib.reload(environment)
    return environment.AUTH_TOKEN

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