from flask import Blueprint, request, jsonify, render_template
import app.services.control_server as control_server
import app.environment.environment_manager as environment_manager
import app.services.auth_server as auth_server

server_config_bp = Blueprint("server_configuration", __name__)

@server_config_bp.route("/server_configuration")
def server_configuration():
    return render_template("server_configuration.html")

# ------------------------------------------------------
# Start TCP server and accept data from devices 
# ------------------------------------------------------ 
@server_config_bp.route("/start_server", methods=["GET", "POST"])
def start_server():

    environment_manager.update_start_server_status(True)
    environment_manager.update_stop_server_status(False)

    return jsonify({"success": True, "message": "TCP Server started successfully!"})
    
# ------------------------------------------------------
# Stop TCP server and not accept data from devices 
# Stop TCP server to config server
# ------------------------------------------------------     
@server_config_bp.route("/stop_server", methods=["GET"])
def stop_server():

    environment_manager.update_start_server_status(False)
    environment_manager.update_stop_server_status(True)

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

    # Call authentication service
    success, redirect_url = auth_server.authenticate_user(username, password)

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

        # Check if the facility exists
        facilities = environment_manager.get_facility_list()
        facility_exists = any(str(fac["id"]) == facility_id for fac in facilities.get("data", []))

        if not facility_exists:
            return jsonify({"success": False, "message": "Facility ID does not exist!"}), 404

        # Update Environment variable (Store selected Facility)
        facility_id = environment_manager.update_facility_id()
        print(f"[INFO] Facility ID Updated: {facility_id}")

        return jsonify({"success": True, "message": f"Facility ID '{facility_id}' updated successfully!"})

    except Exception as e:
        return jsonify({"success": False, "message": f"Error: {str(e)}"}), 500