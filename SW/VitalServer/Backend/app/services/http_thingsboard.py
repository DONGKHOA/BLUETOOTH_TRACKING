import requests
import json
import aiohttp
import redis

import app.environment.environment as environment

# Connect to Redis
r = redis.Redis(host='localhost', port=6379, db=0)

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

    # Payload with Aerosense Device ID
    data = {
        "name": device_name,
        "type": "default"
    }

    try:
        async with aiohttp.ClientSession() as session:
            
            async with session.post(url, json=data, headers=get_headers()) as response:
                
                if response.status == 200:
                    new_device = await response.json()
                    print("Device created:")
                    print(json.dumps(new_device, indent=4, ensure_ascii=False))
                    return new_device['id']['id'], True 
                
                elif response.status == 400:
                    # Try to parse error
                    error_msg = await response.text()
                    
                    if "already exists" in error_msg:
                        print("Device already exists. Fetching ID by name...")
                    
                        existing_device_id = await get_device_id_by_name(device_name)
                        return existing_device_id, False 
                else:
                    print(f" Status: {response.status}")
                    print(f" Text: {await response.text()}")
                    response.raise_for_status()

    except Exception as e:
        print(f"Error registering device: {e}")
        return None

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