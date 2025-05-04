import json
from datetime import datetime

def load_users():
    with open("users.json", "r") as f:
        return json.load(f)

def save_users(users):
    with open("users.json", "w") as f:
        json.dump(users, f, indent=2)

def load_attendance():
    with open("attendance.json", "r") as f:
        return json.load(f)

def save_attendance(attendance):
    with open("attendance.json", "w") as f:
        json.dump(attendance, f, indent=2)
        
def get_users_for_device(device_id):
    users_all = load_users()
    return users_all.get(device_id, [])

def response_sync():
    users_all = load_users()
    next_id_num = 1
    while f"AC{next_id_num}" in users_all:
        next_id_num += 1

    new_id = f"AC{next_id_num}"
    users_all[new_id] = []
    save_users(users_all)

    return {
        "command": "SYN",
        "id": new_id
    }
  
def reponse_user_data(data, device_id):
    users_all = load_users()

    # Ensure the device_id key exists
    if device_id not in users_all:
        users_all[device_id] = []

    users_all[device_id] = data.get("user_data", [])

    save_users(users_all)
    
def response_enroll_face(user_id, device_id):
    users_all = load_users()
    users = users_all.get(device_id, [])

    for user in users:
        if user.get("id") == user_id:
            user["face"] = 1
            save_users(users_all)
            return {"command": "ENROLL_FACE", "user_id": user_id, "name": user.get("name"), "response": "success"}

    return {"command": "ENROLL_FACE", "user_id": user_id, "response": "fail"}

def response_enroll_finger(user_id, device_id):
    users_all = load_users()
    users = users_all.get(device_id, [])

    for user in users:
        if user.get("id") == user_id:
            user["finger"] = 1
            save_users(users_all)
            return {"command": "ENROLL_FINGERPRINT", "user_id": user_id, "name": user.get("name"), "response": "success"}

    return {"command": "ENROLL_FINGERPRINT", "user_id": user_id, "response": "fail"}


def response_attendance(user_id, device_id, timestamp):
    users_all = load_users()
    users = users_all.get(device_id, [])

    if isinstance(timestamp, (int, float, str)):
        try:
            timestamp = int(timestamp)
            now = datetime.fromtimestamp(timestamp)
        except Exception:
            now = datetime.now()
    else:
        now = datetime.now()

    date_str = now.strftime("%d/%m/%Y")
    time_str = now.strftime("%H:%M:%S")

    for user in users:
        if user.get("id") == user_id:
            name = user.get("name", "")

            attendance_all = load_attendance()
            attendance = attendance_all.setdefault(device_id, [])

            # Check if already logged today
            for row in attendance:
                if row["id"] == user_id and row["date"] == date_str:
                    # Just update the latest time
                    row["check"] = time_str
                    break
            else:
                # Add new row
                attendance.append({
                    "id": user_id,
                    "name": name,
                    "device_id": device_id,
                    "date": date_str,
                    "check": time_str
                })

            save_attendance(attendance_all)
            return {
                "command": "ATTENDANCE",
                "response": "success"
            }

    return {
        "command": "ATTENDANCE",
        "response": "fail"
    }