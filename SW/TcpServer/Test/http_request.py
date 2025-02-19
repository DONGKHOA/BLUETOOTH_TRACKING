import requests
import json

# Base URL for DreamsEdge API
BASE_URL = "http://3.106.166.172:8000"

# Authentication Token (Replace with the correct token)
AUTH_TOKEN = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJleHAiOjE3Mzk5NjIyMjMsInN1YiI6IjMifQ.mPjFqu7gmMts3n7Z0Ffr4fyqYmp4oyBC9hLnEJxrl3A"

# Headers with authentication
HEADERS = {
    "Authorization": f"Bearer {AUTH_TOKEN}",
    "Content-Type": "application/json",
    "Accept": "application/json"
}


# Step 1: Get the list of facilities
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

# Step 2: Select a facility (first one for simplicity)
def select_facility(facilities):
    if not facilities or "data" not in facilities or not facilities["data"]:
        print("No facilities available to select.")
        return None

    first_facility = facilities["data"][0]  # Extract first facility
    return first_facility["id"]  # Use "id" instead of "facility_id"

# Step 3: Generate a unique name
def generate_unique_name():
    return f"TCP_SERVER"

# Step 4: Register the TCP server (Now includes authentication headers)
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
    facilities = get_facility_list()
    if facilities:
        if facilities:
            facility_id = select_facility(facilities)
            if facility_id:
                unique_name = generate_unique_name()
                print(f"Selected Facility ID: {facility_id}, Unique Name: {unique_name}")
                register_tcp_server(facility_id, unique_name)
