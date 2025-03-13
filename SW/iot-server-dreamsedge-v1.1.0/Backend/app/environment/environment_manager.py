import os
import importlib
import re
import json
import app.environment.environment as environment

ENV_FILE_PATH = os.path.join(os.path.dirname(__file__), "environment.py")
UPLOAD_FOLDER = "Environment/upload"

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
# Update USERNAME and PASSWORD in environment.py
# -------------------------------------------------
def update_start_server_status(token):
    with open(ENV_FILE_PATH, "r", encoding="utf-8") as f:
        lines = f.readlines()

    lines = _update_env_line(lines, "START_SERVER", token)

    with open(ENV_FILE_PATH, "w", encoding="utf-8") as f:
        f.writelines(lines)

    # Reload to updated values
    importlib.reload(environment)

# -------------------------------------------------
# Update USERNAME and PASSWORD in environment.py
# -------------------------------------------------
def update_stop_server_status(token):
    with open(ENV_FILE_PATH, "r", encoding="utf-8") as f:
        lines = f.readlines()

    lines = _update_env_line(lines, "STOP_SERVER", token)

    with open(ENV_FILE_PATH, "w", encoding="utf-8") as f:
        f.writelines(lines)

    # Reload to updated values
    importlib.reload(environment)

# -------------------------------------------------
# Return the latest AUTH_TOKEN from environment.py
# -------------------------------------------------
def get_auth_token():
    importlib.reload(environment)
    return environment.AUTH_TOKEN

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
# Return the latest Facility Lists from environment.py
# -------------------------------------------------
def get_facility_list():
    importlib.reload(environment)
    return environment.FACILITY_LIST

# -------------------------------------------------
# Update the facility list in environment.py
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
# Update FACILITY_ID in environment.py.
# -------------------------------------------------
def update_facility_id(token):
    with open(ENV_FILE_PATH, "r", encoding="utf-8") as f:
        lines = f.readlines()

    lines = _update_env_line(lines, "FACILITY_ID", token)

    with open(ENV_FILE_PATH, "w", encoding="utf-8") as f:
        f.writelines(lines)

    # Reload so environment.FACILITY_ID is updated
    importlib.reload(environment)