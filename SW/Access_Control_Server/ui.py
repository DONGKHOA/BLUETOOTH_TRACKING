from flask import Flask, render_template, request, redirect, url_for, jsonify

# from main import enqueue_internal_command

import json
import os
import redis

redis_client = redis.Redis(host="127.0.0.1", port=6379, db=0)

app = Flask(__name__, template_folder='pages')

DATA_FILE = 'users.json'
ATTENDANCE_FILE = 'attendance.json'

# Load users from JSON file
def load_users():
    if os.path.exists(DATA_FILE):
        with open(DATA_FILE, 'r') as f:
            return json.load(f)
    return []

# Load users from JSON file
def load_attendance():
    if os.path.exists(ATTENDANCE_FILE):
        with open(ATTENDANCE_FILE, 'r') as f:
            return json.load(f)
    return []

def save_users(users):
    with open(DATA_FILE, 'w') as f:
        json.dump(users, f, indent=2)

@app.route('/')
def index():
    users = load_users()
    return render_template('server.html', users=users)

@app.route('/append_blank', methods=['POST'])
def append_blank():
    users = load_users()
    last_id = max([u.get("id", 0) for u in users], default=0)
    users.append({
        "id": last_id + 1,
        "name": "",
        "employee_id": "",
        "role": "User",
        "face": 0,
        "finger": 0
    })
    return render_template('server.html', users=users)

@app.route('/update', methods=['POST'])
def update_users():
    name = request.form.get('name')
    employee_id = request.form.get('employee_id')
    role = request.form.get('role')

    users = load_users()

    if name and employee_id:
        # Find the highest current ID, or start from 0
        last_id = max([u.get("id", 0) for u in users], default=0)

        # Append new user with next ID
        users.append({
            "id": last_id + 1,
            "name": name,
            "employee_id": employee_id,
            "role": role,
            "face": 0,
            "finger": 0
        })

        # Send a DELETE_USER_DATA command to the queue
        print(f"Adding user data for ID: {last_id + 1}")

        redis_client.rpush("mqtt_queue", json.dumps({
            "command": "ADD_USER_DATA",
            "id": last_id + 1,
            "name": name,
        }))

    save_users(users)
    return redirect(url_for('index'))

@app.route('/delete/<int:index>', methods=['POST'])
def delete_user(index):
    users = load_users()

    if 0 <= index < len(users):
        delete_id_user = users.pop(index)
        user_id = delete_id_user.get("id")

        save_users(users)

        if user_id:
            # Send a DELETE_USER_DATA command to the queue
            print(f"Deleting user data for ID: {user_id}")

            redis_client.rpush("mqtt_queue", json.dumps({
                "command": "DELETE_USER_DATA",
                "id": user_id
            }))

    return redirect(url_for('index'))

@app.route('/attendance')
def show_attendance():
    attendance = load_attendance()
    return render_template('attendance.html', attendance_data=attendance)


if __name__ == '__main__':
    app.run(host="0.0.0.0", port=9000, debug=True)
