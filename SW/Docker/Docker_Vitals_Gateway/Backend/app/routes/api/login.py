from flask import Blueprint, request, jsonify, session, redirect, url_for, render_template
import app.environment.environment_manager as environment_manager
import app.services.auth_server as auth_server

login_bp = Blueprint("login", __name__)

# --------------------------------------------------
# Login function: 
# -> Check the username and password if it matched 
# -> Entry to the server_status page
# --------------------------------------------------
@login_bp.route("/login", methods=["GET", "POST"])
def login():
    if request.method == "GET":
        return render_template("login.html")
        
    login_username_new = request.form.get("username", "").strip()
    login_password_new = request.form.get("password", "").strip()

    print(f"[INFO] Received login request - Username: {login_username_new}")

    # Fetch stored credentials
    credentials = environment_manager.get_login_info()
    login_username_old = credentials["login_username"]
    login_password_old = credentials["login_password"]

    if login_username_new in ["admin", "supervisor"] and login_password_new == login_password_old:
        if login_username_new == "admin":
            # Enforce single admin login
            if auth_server.get_admin_session_token():
                print("[WARNING] Another Admin is already logged in! Logging out previous session.")
                auth_server.clear_admin_session_token()

            # Generate new session token for admin
            session_token = auth_server.update_admin_session()
            session["authenticated"] = True
            session["session_token"] = session_token
            session["username"] = "admin"

            return jsonify({"success": True, "redirect": url_for("server_management.server_management")})

        elif login_username_new == "supervisor":
            session["authenticated"] = True
            session["username"] = "supervisor"
            return jsonify({"success": True, "redirect": url_for("server_management.server_management")})

    return jsonify({"success": False, "message": "Incorrect username or password."})
# --------------------------------------------------
# Logout Route
# --------------------------------------------------
@login_bp.route("/logout")
def logout():
    # If logging out an admin, clear the session token
    if session.get("username") == "admin" and session.get("session_token") == auth_server.get_admin_session_token():
        print("[INFO] Admin logging out, clearing session token.")
        auth_server.clear_admin_session_token()  # Clear admin session token

    session.clear()  # Completely clear session for any user

    # return redirect(url_for("login", error="You have been logged out."))
    # return render_template("login.html")
    return redirect(url_for("login.login")) 