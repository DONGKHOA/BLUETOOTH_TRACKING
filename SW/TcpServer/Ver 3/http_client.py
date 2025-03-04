import requests
import json

import environment

# Base URL for DreamsEdge API
BASE_URL = f"http://{environment.IP_API}:8000"

# # Headers with authentication
# HEADERS = {
#     "Authorization": f"Bearer {environment.AUTH_TOKEN}",
#     "Content-Type": "application/json",
#     "Accept": "application/json"
# }

number_of_facility = 0

def get_headers():
    return {
        "Authorization": f"Bearer {environment.AUTH_TOKEN}",
        "Content-Type": "application/json",
        "Accept": "application/json"
    }

def login_access_token(user_name, password):
    url = f"{BASE_URL}/login/access-token"

    header_login = {
        "accept": "application/json",
        "Content-Type": "application/x-www-form-urlencoded",
    }

    # Send data as form-encoded body (not query parameters)
    data = {
        "grant_type": "password",
        "username": user_name,  # Ensure the key matches what the API expects
        "password": password,
        "scope": "",
        "client_id": "string",
        "client_secret": "string",
    }

    try:
        response = requests.post(url, headers=header_login, data=data)  # Use `data` instead of `params`
        response.raise_for_status()

        print(response.json()["access_token"])

        return response.json()["access_token"]

    except requests.HTTPError as http_err:
        print(f"Error getting access token: {http_err}")
        print(f"Response: {response.text}")
    except requests.RequestException as e:
        print("Request error:", e)


def get_facility_list(skip=0, limit=100):
    url = f"{BASE_URL}/facility/list?skip={skip}&limit={limit}"
    print(f"Fetching facility list from: {url}")  # Debugging

    try:
        response = requests.get(url, headers=get_headers(), timeout=10)
        print(f"Response status code: {response.status_code}")  # Debugging

        response.raise_for_status()  # Raise an error for non-200 responses

        facility_list = response.json()

        # Print the JSON response in a readable format
        print(json.dumps(facility_list, indent=4, ensure_ascii=False))

        return facility_list
    except requests.exceptions.Timeout:
        print("Request timed out! The server may be down.")
    except requests.exceptions.ConnectionError:
        print("Failed to connect to the API. Check your network or server status.")
    except requests.exceptions.HTTPError as http_err:
        print(f"HTTP error occurred: {http_err}")
    except requests.RequestException as e:
        print(f"Error fetching facility list: {e}")

    return None



def get_device_list(skip=0, limit=100):
    url = f"{BASE_URL}/device/list?skip={skip}&limit={limit}"
    print(f"Fetching device list from: {url}")  # Debugging

    try:
        response = requests.get(url, headers=get_headers(), timeout=10)
        print(f"Response status code: {response.status_code}")  # Debugging

        response.raise_for_status()  # Raise an error for non-200 responses

        device_list = response.json()

        # Print the JSON response in a readable format
        print(json.dumps(device_list, indent=4, ensure_ascii=False))

        return device_list
    except requests.exceptions.Timeout:
        print("Request timed out! The server may be down.")
    except requests.exceptions.ConnectionError:
        print("Failed to connect to the API. Check your network or server status.")
    except requests.exceptions.HTTPError as http_err:
        print(f"HTTP error occurred: {http_err}")
    except requests.RequestException as e:
        print(f"Error fetching facility list: {e}")

    return None


def check_facility(facility_list):
    global number_of_facility
    number_of_facility = facility_list["count"]
    for i in range(number_of_facility):
        sub_facility = facility_list["data"][i]
        if sub_facility["tcp_server_name"] == environment.TCP_SERVER_NAME:
            return 1
    return 0

def register_tcp_server(facility_id, unique_name):
    url = f"{BASE_URL}/facility/register-tcp-server"

    # Query parameters instead of JSON body
    params = {
        "facility_id": facility_id,
        "tcp_server_name": unique_name
    }
    try:
        response = requests.post(url, headers=get_headers(), params=params)  # Use `params`
        response.raise_for_status()

        register_tcp = response.json()

        # Print the JSON response in a readable format
        print(json.dumps(register_tcp, indent=4, ensure_ascii=False))

    except requests.HTTPError as http_err:
        print(f"Error registering TCP server: {http_err}")
        print(f"Response Content: {response.text}")  # Print the API response for debugging
    except requests.RequestException as e:
        print("Request error:", e)

def register_device(device_id):
    url = f"{BASE_URL}/device/create"

    # Payload with Aerosense Device ID
    data = {
        "name": environment.DEVICE_NAME,
        "id": device_id
    }

    try:
        response = requests.post(url, json=data, headers=get_headers())
        response.raise_for_status()  # Raise an error for non-200 responses

        new_device = response.json()

        # Print the JSON response in a readable format
        print(json.dumps(new_device, indent=4, ensure_ascii=False))

        print("Device successfully registered:", response.json())
        return response.json()

    except requests.HTTPError as http_err:
        print(f"HTTP error occurred: {http_err}")
        print(f"Response Content: {response.text}")
    except requests.RequestException as e:
        print("Request error:", e)

    return None

def generate_topic(device_id):
    return f"{environment.TCP_SERVER_NAME}/{device_id}"  # Format: [tcp_server_unique_name]/[device_id]