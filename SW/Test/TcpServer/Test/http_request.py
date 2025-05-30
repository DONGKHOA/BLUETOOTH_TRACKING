import requests
import json

# Base URL for DreamsEdge API
BASE_URL = "http://3.106.166.172:8000"

# Authentication Token (Replace with the correct token)
AUTH_TOKEN = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJleHAiOjE3NDAwMjUyNzMsInN1YiI6IjMifQ.ubaZAmIiQNr52KXdMmkFBuQVnswOkZZwj9wQ9xS_nCk"

# Headers with authentication
HEADERS = {
    "Authorization": f"Bearer {AUTH_TOKEN}",
    "Content-Type": "application/json",
    "accept": "application/json"
}

#TCP server information
TCP_SERVER_NAME = "TCP_SERVER_Aerosense"

#Deivice name
DEVICE_NAME = "AerosenseDevice"

number_of_facility = 0

def get_facility_list(skip=0, limit=100):
    url = f"{BASE_URL}/facility/list?skip={skip}&limit={limit}"
    print(f"Fetching facility list from: {url}")  # Debugging

    try:
        response = requests.get(url, headers=HEADERS, timeout=10)
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
        response = requests.get(url, headers=HEADERS, timeout=10)
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
        if sub_facility["tcp_server_name"] == TCP_SERVER_NAME:
            return 1
    return 0

def check_device(device_list, device_id):
    for element in device_list:
        if element["name"] == DEVICE_NAME:
            if element["id"] == device_id:
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
        response = requests.post(url, headers=HEADERS, params=params)  # Use `params`
        response.raise_for_status()

        register_tcp = response.json()

        # Print the JSON response in a readable format
        print(json.dumps(register_tcp, indent=4, ensure_ascii=False))

    except requests.HTTPError as http_err:
        print(f"Error registering TCP server: {http_err}")
        print(f"Response Content: {response.text}")  # Print the API response for debugging
    except requests.RequestException as e:
        print("Request error:", e)


# Run the function
if __name__ == "__main__":
    # global number_of_facility
    get_device_list()

    # facilities = get_facility_list()
    #
    # if facilities:
    #     ret_val = check_facility(facilities)
    #     print(ret_val)
    #     if ret_val == 0:
    #         unique_name = TCP_SERVER_NAME
    #         print(f"Selected Facility ID: {number_of_facility}, Unique Name: {unique_name}")
    #         register_tcp_server(number_of_facility, unique_name)
    # get_facility_list()