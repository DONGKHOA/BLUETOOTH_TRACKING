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


def response_attendance(user_id, device_id):
    users_all = load_users()
    users = users_all.get(device_id, [])

    now = datetime.now()
    date_str = now.strftime("%d/%m/%Y")
    time_str = now.strftime("%H:%M:%S")

    # Find user with matching ID
    for user in users:
        if user.get("id") == user_id:
            name = user.get("name")
            attendance_all = load_attendance()
            attendance = attendance_all.setdefault(device_id, [])

            # Check if user already has an entry for today
            for row in attendance:
                if row["id"] == user_id and row["date"] == date_str:

                    # Get the time of attendance already exist
                    checks = []
                    for i in range(1, 7):
                        key = f"check{i}"
                        check_time = row.get(key, "").strip()
                        if check_time:
                            try:
                                check_dt = datetime.strptime(f"{date_str} {check_time}", "%d/%m/%Y %H:%M:%S")
                                checks.append((key, check_dt))
                            except ValueError:
                                pass

                    # Find the latest check time
                    if checks:
                        latest_key, latest_dt = max(checks, key=lambda x: x[1])
                        delta = now - latest_dt
                        if delta.total_seconds() <= 60:
                            # Update the latest check time
                            row[latest_key] = time_str
                            break  # Exit the row loop after updating

                    # If no checks to update, find the next empty slot
                    for i in range(1, 7):
                        key = f"check{i}"
                        current_value = row.get(key, "").strip()
                        if not current_value:
                            row[key] = time_str
                            break
                    break  # Exit the row loop after processing

            else:
                new_attendance = {
                    "id": user_id,
                    "name": name,
                    "date": date_str,
                    "check1": time_str
                }
                attendance.append(new_attendance)

            save_attendance(attendance)
            return {
                "command": "ATTENDANCE",
                "response": "success"
            }

    # If user not found
    return {
        "command": "ATTENDANCE",
        "response": "fail"
    }