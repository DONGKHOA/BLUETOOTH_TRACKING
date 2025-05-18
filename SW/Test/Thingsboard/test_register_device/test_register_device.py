import requests
import environment
import asyncio
import json
import aiohttp

new_device = None
access_token = None

# Function to get headers for the request
def get_headers():
    return {
        "Content-Type": "application/json",
        "Authorization": f"Bearer {environment.AUTH_TOKEN}",
    }

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
                    return new_device['id']['id']
                
                elif response.status == 400:
                    # Try to parse error
                    error_msg = await response.text()
                    
                    if "already exists" in error_msg:
                        print("Device already exists. Fetching ID by name...")
                        return await get_device_id_by_name(device_name)
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

async def main():
    device_name = "Aerosense-Device-12345"
    
    device_id = await register_device(device_name)
    
    if device_id:
        token = await get_access_token(device_id)
        if token:
            print(f" Access token for {device_name}: {token}")
        
if __name__ == "__main__":
    asyncio.run(main())