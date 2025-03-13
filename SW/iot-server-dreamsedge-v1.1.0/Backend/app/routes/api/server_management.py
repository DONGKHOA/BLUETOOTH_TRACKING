from flask import Blueprint, request, jsonify, render_template


import asyncio

server_manage_bp = Blueprint("server_management", __name__)

@server_manage_bp.route("/server_management")
def server_management():
    return render_template("server_management.html")

# ------------------------------------------------------
# Check server status function: 
# -> Check status of TCP server and AWS server
# ------------------------------------------------------
@server_manage_bp.route("/check_status_server", methods=["GET"])
def check_status_server():
    return jsonify({
        "tcp_status": "running",
        "aws_status": "connected"
    })