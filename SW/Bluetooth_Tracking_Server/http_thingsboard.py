import requests
import json
import aiohttp
import redis
import environment 
import os
from dotenv import load_dotenv

# Connect to Redis
load_dotenv()
QUEUE_HOST = os.getenv("QUEUE_HOST", "localhost")
QUEUE_PORT = int(os.getenv("QUEUE_PORT", 6380))
r = redis.Redis(host=QUEUE_HOST, port=QUEUE_PORT, db=0)

# Function to get headers for the request
def get_headers():
    auth_token = r.get("auth_token").decode()
    return {
        "Content-Type": "application/json",
        "Authorization": f"Bearer {auth_token}",
    }

def login_auth_token(user_name, password):
    url = f"{environment.BASE_URL}/api/auth/login"

    header_login = {
        "Content-Type": "application/json"
    }

    # Send data as form-encoded body (not query parameters)
    data = {
        "username": user_name,  # Ensure the key matches what the API expects
        "password": password
    }

    try:
        response = requests.post(url, headers=header_login, json=data)
        response.raise_for_status()

        print(response.json()["token"])

        return response.json()["token"]

    except requests.HTTPError as http_err:

        print(f"Error getting auth token: {http_err}")
        print(f"Response: {response.text}")
    except requests.RequestException as e:
        print("Request error:", e)

async def get_device_id_by_name(device_name):
    url = f"{environment.BASE_URL}/api/tenant/devices?deviceName={device_name}"
    
    try:
        async with aiohttp.ClientSession() as session:
            
            async with session.get(url, headers=get_headers()) as response:
                response.raise_for_status()
                device_info = await response.json()
                return device_info['id']['id']
            
    except Exception as e:
        print(f"Error getting device ID by name: {e}")
        return None

async def register_device(device_name):
    url = f"{environment.BASE_URL}/api/device"

    data = {
        "name": device_name,
        "type": "gateway",
        "label": device_name  # optional
    }

    try:
        async with aiohttp.ClientSession() as session:
            async with session.post(url, json=data, headers=get_headers()) as response:

                if response.status == 200:
                    new_device = await response.json()
                    device_id = new_device['id']['id']
                    print("Device created:")
                    print(json.dumps(new_device, indent=4, ensure_ascii=False))

                    # Automatically assign device to Public customer
                    await public_device(device_id)

                    return device_id, True 

                elif response.status == 400:
                    error_msg = await response.text()

                    if "already exists" in error_msg:
                        print("Device already exists. Fetching ID by name...")
                        existing_device_id = await get_device_id_by_name(device_name)

                        # Make sure the already-existing device is public
                        if existing_device_id:
                            await public_device(existing_device_id)

                        return existing_device_id, False 
                else:
                    print(f"[ERROR] Status: {response.status}")
                    print(f"[ERROR] Text: {await response.text()}")
                    response.raise_for_status()

    except Exception as e:
        print(f"[ERROR] Exception registering device: {e}")
        return None, False
    
async def public_device(device_id):
    try:
        url_get_customers = f"{environment.BASE_URL}/api/customers?page=0&pageSize=100"

        async with aiohttp.ClientSession() as session:
            
            async with session.get(url_get_customers, headers=get_headers()) as resp:
                if resp.status != 200:
                    print(f"[ERROR] Failed to get customers. Status: {resp.status}")
                    return False
                customers_data = await resp.json()

            public_customer = next((c for c in customers_data["data"] if c["title"] == "Public"), None)
            if not public_customer:
                print("[ERROR] Public customer not found.")
                return False

            public_customer_id = public_customer["id"]["id"]

            url_assign = f"{environment.BASE_URL}/api/customer/{public_customer_id}/device/{device_id}"
            async with session.post(url_assign, headers=get_headers()) as assign_resp:
                if assign_resp.status == 200:
                    print(f"[INFO] Device {device_id} assigned to Public.")
                    return True
                else:
                    print(f"[ERROR] Failed to assign device. Status: {assign_resp.status}")
                    return False

    except Exception as e:
        print(f"[ERROR] Exception while assigning public device: {e}")
        return False

async def get_access_token(device_id):
    url = f"{environment.BASE_URL}/api/device/{device_id}/credentials"

    try:
        async with aiohttp.ClientSession() as session:
            async with session.get(url, headers=get_headers()) as response:
                response.raise_for_status()  # Raise an error for non-200 responses

                credentials = await response.json()

                print(json.dumps(credentials, indent=4, ensure_ascii=False))

                # print("Access token:", credentials['credentialsId'])
                return credentials['credentialsId']

    except Exception as e:
        print(f"Error getting access token: {e}")
        return None