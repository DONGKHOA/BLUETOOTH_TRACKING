from flask import Blueprint, request, jsonify, render_template


login_bp = Blueprint("login", __name__)

USERS = {
    "admin": "123", 
    "user": "pass456"
}

# --------------------------------------------------
# Login function: 
# -> Check the username and password if it matched 
# -> Entry to the server_status page
# --------------------------------------------------
@login_bp.route("/login", methods=["POST"])
def login():
    data = request.form
    username = data.get("username")
    password = data.get("password")

    print(f"Login attempt: {username} / {password}")

    if username in USERS and USERS[username] == password:
        return jsonify({"success": True, "redirect": "/server_management"})
    else:
        return jsonify({"success": False, "message": "Invalid username or password"}), 401