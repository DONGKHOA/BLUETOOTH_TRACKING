from flask import Flask, render_template, request, redirect, url_for, jsonify, session

# from main import enqueue_internal_command

import json
import os
import redis

redis_client = redis.Redis(host="127.0.0.1", port=6379, db=0)

app = Flask(__name__, template_folder='pages')

app.secret_key = 'your-secret-key'

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
    all_users = load_users()
    merged_users = []
    new_user_id = session.pop('new_user_id', None)
    for device_id, users in all_users.items():
        for user in users:
            user["device_id"] = device_id
            merged_users.append(user)
    return render_template('server.html', users=merged_users, new_user_id=new_user_id)

@app.route('/append_blank', methods=['POST'])
def append_blank():
    device_id = request.form.get('device_id')
    users = load_users()
    if device_id not in users:
        users[device_id] = []

    last_id = max([u.get("id", 0) for u in users[device_id]], default=0)
    new_user_id = last_id + 1
    users[device_id].append({
        "id": new_user_id,
        "name": "",
        "employee_id": "",
        "role": "user",
        "face": 0,
        "finger": 0
    })

    save_users(users)
    session['new_user_id'] = new_user_id
    return redirect(url_for('index'))

@app.route('/update', methods=['POST'])
def update_users():
    name = request.form.get('name', '').strip()
    employee_id = request.form.get('employee_id', '').strip()
    role = request.form.get('role', 'user').strip()
    device_id = request.form.get('device_id', '').strip()

    if not device_id:
        return "Device ID is missing.", 400

    users_all = load_users()
    users = users_all.setdefault(device_id, [])

    id_raw = request.form.get('id')
    user_id = int(id_raw) if id_raw else max([u.get("id", 0) for u in users], default=0) + 1

    updated = False
    for user in users:
        if user.get("id") == user_id:
            user["name"] = name
            user["employee_id"] = employee_id
            user["role"] = role
            updated = True
            print(f"Updated user {user_id} in device {device_id}")
            redis_client.rpush("mqtt_queue", json.dumps({
                "command": "ADD_USER_DATA",
                "id": user_id,
                "name": name,
                "device_id": device_id
            }))
            break

    if not updated:
        new_user = {
            "id": user_id,
            "name": name,
            "employee_id": employee_id,
            "role": role,
            "face": 0,
            "finger": 0
        }
        users.append(new_user)
        print(f"Added new user {user_id} to device {device_id}")
        redis_client.rpush("mqtt_queue", json.dumps({
            "command": "ADD_USER_DATA",
            "id": user_id,
            "name": name,
            "device_id": device_id
        }))
        session['new_user_id'] = user_id

    save_users(users_all)
    return redirect(url_for('index'))

@app.route('/set_role/<device_id>/<int:user_id>', methods=['POST'])
def set_role(device_id, user_id):
    new_role = request.form['role']
    users = load_users()
    if device_id in users:
        for user in users[device_id]:
            if user['id'] == user_id:
                user['role'] = new_role
                save_users(users)
                print(f"Set new role of user ID {user_id} in {device_id}")
                redis_client.rpush("mqtt_queue", json.dumps({
                    "command": "SET_ROLE",
                    "id": user_id,
                    "role": new_role,
                    "device_id": device_id
                }))
                break
    return redirect(url_for('index'))


@app.route('/delete/<device_id>/<int:user_id>', methods=['POST'])
def delete_user(device_id, user_id):
    users_all = load_users()
    
    if device_id in users_all:
        original_len = len(users_all[device_id])
        users_all[device_id] = [u for u in users_all[device_id] if u.get("id") != user_id]

        if len(users_all[device_id]) < original_len:
            print(f"Deleted user {user_id} from device {device_id}")

            if not users_all[device_id]:
                users_all[device_id] = []

            save_users(users_all)

            redis_client.rpush("mqtt_queue", json.dumps({
                "command": "DELETE_USER_DATA",
                "id": user_id,
                "device_id": device_id
            }))

    return redirect(url_for('index'))

@app.route('/delete_face/<device_id>/<int:user_id>', methods=['POST'])
def delete_face(device_id, user_id):
    users = load_users()
    if device_id in users:
        for user in users[device_id]:
            if user['id'] == user_id:
                user['face'] = 0
                save_users(users)

                print(f"Deleted face for user ID {user_id} on device {device_id}")

                redis_client.rpush("mqtt_queue", json.dumps({
                    "command": "DELETE_FACEID_USER",
                    "id": user_id,
                    "device_id": device_id
                }))
                break

    return redirect(url_for('index'))

@app.route('/delete_finger/<device_id>/<int:user_id>', methods=['POST'])
def delete_finger(device_id, user_id):
    users = load_users()
    if device_id in users:
        for user in users[device_id]:
            if user['id'] == user_id:
                user['finger'] = 0
                save_users(users)

                print(f"Deleted finger for user ID {user_id} on device {device_id}")

                redis_client.rpush("mqtt_queue", json.dumps({
                    "command": "DELETE_FINGER_USER",
                    "id": user_id,
                    "device_id": device_id
                }))
                break

    return redirect(url_for('index'))

@app.route("/available_devices")
def available_devices():
    users = load_users()
    return jsonify(list(users.keys()))


@app.route('/attendance')
def show_attendance():
    attendance = load_attendance()
    return render_template('attendance.html', attendance_data=attendance)


if __name__ == '__main__':
    app.run(host="0.0.0.0", port=9000, debug=True)
