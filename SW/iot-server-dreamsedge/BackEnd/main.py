# -----------------------------------------------
# Link the Environment and FrontEnd folders
# Therefore, can import environment.py
# -----------------------------------------------
import sys
import os

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))

sys.path.append(os.path.join(PROJECT_ROOT, "Environment"))

sys.path.append(os.path.join(PROJECT_ROOT, "FrontEnd"))

import environment
import importlib

import socket

# -----------------------------------------------
# Import libraries to initialize TCP Server
# -----------------------------------------------
import http_client
import handle_data

import asyncio
import numpy as np
import json
import ssl
import paho.mqtt.client as mqtt

from datetime import datetime
import time

# -------------------------------------------------
# Included function to get data from environment.py
# -------------------------------------------------
from environment_manager import (
    get_info_user, 
    update_auth_token,
    update_facility_list,
    get_facility_id,
    update_connection_aws,
    update_start_check_aws,
    get_start_check_aws,
    update_status_tcp_server,
    update_status_aws_server
)

# Global condition variables
environment_data_ready = False
auth_token_received = False
facilities_received = False
facility_id_received = False
aws_iot_endpoint_connection = False

# Global counter (uint32)
count = np.uint32(0)

# Maps client_ip -> client_id_hex
ip_to_id_map = {}

# Maps client_ip -> current writer (so we can close the old writer if the same IP reconnects)
ip_to_writer_map = {}

# Global MQTT client
mqtt_client = None

update_status_tcp_server(None)
update_status_aws_server(None)

def check_tcp_connection(host, port, timeout=5):
    try:
        # Resolve hostname
        addr_info = socket.getaddrinfo(host, port)
        if not addr_info:
            print(1)  # Failed DNS resolution
            return False

        # Attempt to create a socket connection
        with socket.create_connection((host, port), timeout=timeout):
            return True
    except socket.gaierror:
        print(1)  # DNS resolution failure
        return False
    except (socket.timeout, ConnectionRefusedError, OSError):
        print(1)  # TCP connection failure
        return False

# -----------------------------------------------
# Background task to refresh the AUTH_TOKEN
# every 3 hours (10800 seconds)
# -----------------------------------------------
async def refresh_auth_token():
    while True:
        await asyncio.sleep(10800)  # 3 hours in seconds
        try:
            environment.AUTH_TOKEN = http_client.login_access_token(environment.USERNAME, environment.PASSWORD)
            print("[Auth Refresh] Successfully refreshed AUTH_TOKEN.")
        except Exception as e:
            print(f"[Auth Refresh] Exception while refreshing token: {e}")

# -----------------------------------------------
# Update Variables in Environment.py via UI
# -----------------------------------------------            
async def update_data_environment():
    global environment_data_ready ,info_user_received, auth_token_received, facilities_received, facility_id_received
    while not environment_data_ready:
        
        # Reload environment variables
        importlib.reload(environment)
        
        credentials = get_info_user()
        username = credentials["username"]
        password = credentials["password"]
        
        # Get AUTH_TOKEN    
        if not auth_token_received:
            print(username)
            print(password)
            environment.AUTH_TOKEN = http_client.login_access_token(username, password)
        
            if environment.AUTH_TOKEN:
                auth_token_received = True
                update_auth_token(environment.AUTH_TOKEN)
                print("[INFO] Auth token received:", environment.AUTH_TOKEN)
            else:
                print("[WARNING] Failed to get Auth Token.")
       
        # Get FACILITY_LIST         
        if auth_token_received and not facilities_received:
            facilities = http_client.get_facility_list()
            
            if facilities:
                facilities_received = True 
                update_facility_list(facilities)
                print("[INFO] Facility list updated in environment.py")
            else:
                print("[WARNING] No facilities available.")
                
        # Get FACILITY_ID 
        if facilities_received and not facility_id_received:
            facility_id = get_facility_id()
            
            if facility_id:
                facility_id_received = True
                print("[INFO] Facility ID:", environment.FACILITY_ID)

                http_client.register_tcp_server(environment.FACILITY_ID, environment.TCP_SERVER_NAME)
        
        # If all data ready -> break
        if auth_token_received and facilities_received and facility_id_received:
            environment_data_ready = True
            break
                  
        await asyncio.sleep(1)

# -----------------------------------------------
# Check the connection with AWS IoT Endpoint 
# before connect MQTT
# -----------------------------------------------
async def verify_aws_connection():
    global aws_iot_endpoint_connection, environment_data_ready
    
    while not aws_iot_endpoint_connection and environment_data_ready:
        
        start_check_aws = get_start_check_aws()
        print("Start Check AWS", start_check_aws)
        
        if start_check_aws:
            print("[BackEnd] Detected START_CHECK_AWS flag. Checking AWS IoT connection...")
            
            if check_tcp_connection(environment.AWS_IOT_ENDPOINT, environment.MQTT_PORT):
                mqtt_client.connect(environment.AWS_IOT_ENDPOINT, environment.MQTT_PORT, 60)
                mqtt_client.loop_start()
                update_connection_aws(True)  
                aws_iot_endpoint_connection = True
                update_status_tcp_server(True)
                print("MQTT Connected Successfully")
                break
                
            else:
                update_connection_aws(False)  
                aws_iot_endpoint_connection = False
                print("Failed to establish TCP connection to AWS IoT")
                
            update_start_check_aws(False)
            
        await asyncio.sleep(1)
        
# ------------------------------------------------
# Always check status of TCP server and AWS Server
# ------------------------------------------------       
# async def verify_status_server():
#     while True:
        
#         tcp_server_status = get_status_tcp_server()
#         aws_server_status = get_status_aws_server()
        
            
#         time.sleep(1)


async def handle_client(reader: asyncio.StreamReader, writer: asyncio.StreamWriter):
    global count
    global mqtt_client

    client_address = writer.get_extra_info('peername')
    if not client_address:
        # Some cases where writer.get_extra_info('peername') could return None (if the socket closed quickly)
        print("[handle_client] Could not retrieve peername (client_address)")
        writer.close()
        await writer.wait_closed()
        return

    client_ip = client_address[0]
    print(f"[Async] Handling new client from IP={client_ip}")

    # If an existing writer for this IP exists, close it (enforcing "one IP - one connection")
    if client_ip in ip_to_writer_map:
        old_writer = ip_to_writer_map[client_ip]
        try:
            old_writer.close()
        except:
            pass
        await old_writer.wait_closed()
        print(f"[Async] Closed old connection for IP={client_ip}")

    # Record the new writer for this IP
    ip_to_writer_map[client_ip] = writer

    # Increase 'count' if this is the first time we see this IP
    if client_ip not in ip_to_id_map:
        count += 1

    try:
        while True:
            # ----------------------------------------------------------
            #  Add exception handling for connection-reset errors
            # ----------------------------------------------------------
            try:
                data = await reader.read(1024)
            except (ConnectionResetError, BrokenPipeError) as e:
                print(f"[Async] Connection error from {client_ip}: {e}")
                break

            if not data:
                # Client closed the connection
                print(f"[Async] Client {client_ip} disconnected.")
                break

            print(f"Received data (hex): {data.hex()}")
            request_id, function, content_len, content_data = handle_data.parse_packet(data)
            print(f"[Parsed] function=0x{function:04x}, content_len={content_len}, content_data={content_data.hex()}")

            # ---------------- Handle function=0x0001 (register ID, send count) ----------------
            if function == 0x0001:
                # Suppose the last 13 bytes are the ID
                if len(content_data) >= 13:
                    new_id_hex = content_data[-13:].hex()
                else:
                    new_id_hex = "TooShort"

                http_client.register_device(new_id_hex)

                ip_to_id_map[client_ip] = new_id_hex
                print(f"[Server] (1) Registered new: IP={client_ip}, ID={new_id_hex}. count={count}")

                # Build a response that contains 'count'
                data_resp = [
                    0x13, 0x01, 0x00, 0x02,
                    (request_id >> 24) & 0xFF,
                    (request_id >> 16) & 0xFF,
                    (request_id >> 8) & 0xFF,
                    (request_id >> 0) & 0xFF,
                    0, 0, 0, 0, 0, 6, 0, 1,
                    (count >> 24) & 0xFF,
                    (count >> 16) & 0xFF,
                    (count >> 8) & 0xFF,
                    (count >> 0) & 0xFF
                ]
                writer.write(bytes(data_resp))
                await writer.drain()
                print(f"[Server] Sent response for function=1 with count={count}")

            # ---------------- Handle function=0x03e8 (28 or 36 bytes of data) ----------------
            elif function == 0x03e8:

                client_id = ip_to_id_map.get(client_ip, "UnknownID")

                if client_id == "UnknownID":
                    data_req = [
                        0x13, 0x01, 0x01, 0x01,
                        (request_id >> 24) & 0xFF,
                        (request_id >> 16) & 0xFF,
                        (request_id >> 8) & 0xFF,
                        (request_id >> 0) & 0xFF,
                        0, 0, 0, 0, 0, 6, 0x04, 0x10,
                        0, 0, 0, 0
                    ]
                    writer.write(bytes(data_req))
                    await writer.drain()
                    print(f"[Server] Sent request for function=0x0410 with count={count}")
                else:
                    if len(content_data) == 28:
                        parsed = handle_data.parse_28_byte_content(content_data)

                        # local_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                        local_time = int(time.time())
                        # Publish the data to MQTT as JSON
                        index_data = {
                            "device_id": client_id,
                            "current_time": local_time,
                        }
                        index_data.update(parsed)  # merges the "parsed" data into index_data

                        json_data = json.dumps(index_data, indent=4)
                        print(json_data)

                        mqtt_client.publish(http_client.generate_topic(client_id), json_data)

                    elif len(content_data) == 36:
                        parsed = handle_data.parse_36_byte_content(content_data)

                        # local_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                        local_time = int(time.time())
                        # Publish the data to MQTT as JSON
                        index_data = {
                            "device_id": client_id,
                            "current_time": local_time,
                        }
                        index_data.update(parsed)  # merges the "parsed" data into index_data

                        json_data = json.dumps(index_data, indent=4)
                        print(json_data)

                        mqtt_client.publish(http_client.generate_topic(client_id), json_data)

                    else:
                        print(f"[!] content_data length={len(content_data)}, expected 28 or 36.")

            elif function == 0x0410:
                new_id_hex = content_data[-13:].hex()
                ip_to_id_map[client_ip] = new_id_hex
                print(f"[Server] (1) Registered new: IP={client_ip}, ID={new_id_hex}. count={count}")

    except Exception as e:
        print(f"[Async] Exception for {client_ip}: {e}")

    finally:
        # Close connection and remove from maps
        writer.close()
        await writer.wait_closed()
        if client_ip in ip_to_id_map:
            del ip_to_id_map[client_ip]
            print(f"[Async] Removed IP={client_ip} from ip_to_id_map.")
        if ip_to_writer_map.get(client_ip) is writer:
            del ip_to_writer_map[client_ip]
        print(f"[Async] Ended for {client_ip}.")

        # Decrease 'count' if close connection
        count = count - 1

async def main():
    global mqtt_client

    # Create and start the background task for refreshing token:
    # asyncio.create_task(refresh_auth_token())
    asyncio.create_task(update_data_environment())
    # asyncio.create_task(verify_aws_connection())
    
    # Wait for all variables in environment.py to be received
    while not environment_data_ready:
        print("[INFO] Waiting for environment setup to complete...")
        await asyncio.sleep(1)
    
    # ---------------- MQTT Setup ----------------
    mqtt_client = mqtt.Client(client_id="AEROSENSEClient")

    # Configure SSL for secure connections to AWS IoT
    mqtt_client.tls_set(ca_certs=environment.root_ca_path,
                   certfile=environment.cert_path,
                   keyfile=environment.private_key_path,
                   tls_version=ssl.PROTOCOL_TLSv1_2)

    # Attempt to connect to AWS
    print(f"Connecting to AWS")
    
    await verify_aws_connection()
    update_status_aws_server(True)
    
    # ---------------- TCP Server Setup (asyncio) ----------------
    server = await asyncio.start_server(
        handle_client,
        host = environment.TCP_SERVER_HOST,
        port = environment.TCP_SERVER_PORT
    )

    addrs = ', '.join(str(sock.getsockname()) for sock in server.sockets)
    print(f"[Async] TCP Server is running on {addrs}")
    async with server:
        # Serve requests until the program is stopped
        await server.serve_forever()

if __name__ == '__main__':
    asyncio.run(main())