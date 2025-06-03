from flask import Blueprint, session, redirect, url_for, render_template, jsonify
import app.environment.environment_manager as environment_manager
import app.services.auth_server as auth_server
import asyncio
import time

server_manage_bp = Blueprint("server_management", __name__)

@server_manage_bp.route("/server_management")
def server_management():
    if not session.get("authenticated"):
        return render_template("login.html")

    if session.get("username") == "admin" and not auth_server.validate_admin_session():
        print("[WARNING] Admin session invalid. Forcing logout.")
        session.clear()
        return render_template("login.html")

    return render_template("server_management.html")

# ------------------------------------------------------
# Check server status function: 
# -> Check status of TCP server and AWS server
# ------------------------------------------------------
@server_manage_bp.route("/check_status_server", methods=["GET"])
def check_status_server():

    status = auth_server.check_tcp_aws_status()

    return jsonify(status)

# ------------------------------------------------------
# Fetch and return facility list, selected facility ID
# and AWS IoT endpoint.
# ------------------------------------------------------
@server_manage_bp.route("/get_current_config", methods=["GET"])
def get_current_config():

    try:
        facility_list_response = environment_manager.get_facility_list()  # This is a dictionary {'data': [...], 'count': X}
        facility_list = facility_list_response.get("data", [])  # Extract only the list of facilities
        count = facility_list_response.get("count", 0)
        aws_iot_endpoint = environment_manager.get_aws_endpoint()
        facility_id = environment_manager.get_facility_id()

        config_data = {
            "aws_iot_endpoint": aws_iot_endpoint,
            "facility_id": facility_id,
            "facilities": facility_list,
            "count": count
        }

        return jsonify({"success": True, "config": config_data})

    except Exception as e:
        return jsonify({"success": False, "message": f"Error retrieving config: {str(e)}"}), 500

@server_manage_bp.route("/check_session", methods=["GET"])
def check_session():
    
    if not session.get("authenticated"):
        return jsonify({"valid": True})

    session_valid, message = auth_server.validate_user_session()  # Call function from auth_server

    if not session_valid:
        session.clear()  # Clear session only if already logged in
        return jsonify({"valid": False, "message": message})

    return jsonify({"valid": True})