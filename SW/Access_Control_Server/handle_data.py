import json
from datetime import datetime
from zoneinfo import ZoneInfo

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
    
def get_user_data(user_id, device_id):
    users_all = load_users()
    users = users_all.get(device_id, [])
    
    for user in users:
        if user.get("id") == user_id:
            return user.get("name")
    
    return None

def reponse_user_data(data, device_id):
    users_all = load_users()

    # Ensure the device_id key exists
    if device_id not in users_all:
        users_all[device_id] = []

    users_all[device_id] = data.get("user_data", [])

    save_users(users_all)
    
def response_attendance_data(data, device_id):
    attendance_all = load_attendance()
    attendance = attendance_all.setdefault(device_id, [])

    index = data.get("index", 0)
    timestamps_str = data.get("timestamp", "")
    user_id = data.get("id")

    if not isinstance(user_id, int) or not timestamps_str:
        return {"command": "ATTENDANCE_DATA", "response": "fail"}

    timestamps = timestamps_str.split("|")
    timestamps = [int(ts) for ts in timestamps if ts.isdigit()]

    users_all = load_users()
    users = users_all.get(device_id, [])
    name = next((u.get("name") for u in users if u.get("id") == user_id), "")

    for ts in timestamps:
        dt = datetime.utcfromtimestamp(ts).astimezone(ZoneInfo("Asia/Ho_Chi_Minh"))
        date_str = dt.strftime("%d/%m/%Y")
        time_str = dt.strftime("%H:%M:%S")

        # Find existing record
        existing_record = next((r for r in attendance if r["id"] == user_id and r["date"] == date_str), None)

        if existing_record:
            # Find next available check field
            i = 1
            while f"check{i}" in existing_record:
                if existing_record[f"check{i}"] == time_str:
                    break  # already stored
                i += 1
            else:
                existing_record[f"check{i}"] = time_str
        else:
            # Create a new record with check1
            new_record = {
                "id": user_id,
                "name": name,
                "device_id": device_id,
                "date": date_str,
                "check1": time_str
            }
            attendance.append(new_record)

    save_attendance(attendance_all)

    return {"command": "ATTENDANCE_DATA", "response": "success"}
    
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

    now = datetime.now(ZoneInfo("Asia/Ho_Chi_Minh"))
    date_str = now.strftime("%d/%m/%Y")
    time_str = now.strftime("%H:%M:%S")

    for user in users:
        if user.get("id") == user_id:
            name = user.get("name", "")

            attendance_all = load_attendance()
            attendance = attendance_all.setdefault(device_id, [])

            existing_record = next((r for r in attendance if r["id"] == user_id and r["date"] == date_str), None)

            if existing_record:
                # Add to next checkN if not already present
                i = 1
                while f"check{i}" in existing_record:
                    if existing_record[f"check{i}"] == time_str:
                        break  # already stored
                    i += 1
                else:
                    existing_record[f"check{i}"] = time_str
            else:
                # First record of the day
                attendance.append({
                    "id": user_id,
                    "name": name,
                    "device_id": device_id,
                    "date": date_str,
                    "check1": time_str
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
    
def get_latest_attendance_timestamp(attendance, user_id):
    
    latest_ts = None

    for record in attendance:
        if record.get("id") == user_id:
            date_str = record.get("date", "")
            for key, value in record.items():
                if key.startswith("check"):
                    try:
                        dt_str = f"{date_str} {value}"
                        dt = datetime.strptime(dt_str, "%d/%m/%Y %H:%M:%S")
                        ts = int(dt.replace(tzinfo=ZoneInfo("Asia/Ho_Chi_Minh")).timestamp())
                        if latest_ts is None or ts > latest_ts:
                            latest_ts = ts
                    except Exception:
                        continue

    return latest_ts
