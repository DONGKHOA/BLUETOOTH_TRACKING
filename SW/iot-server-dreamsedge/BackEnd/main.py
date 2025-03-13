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
    get_aws_endpoint,
    update_connection_aws,
    update_start_check_aws,
    get_start_check_aws,
    update_status_tcp_server,
    update_status_aws_server,
    get_status_tcp_server,
    get_start_server,
    get_stop_server,
    update_stop_server,
    update_start_server,
    get_reconfig_server,
    update_reconfig_server,
    get_reconfig_info,
    update_reconfig_info,
    get_reconfig_facility_id,
    update_reconfig_facility_id,
    get_reconfig_facility_list,
    update_reconfig_facility_list,
    get_reconfig_aws_endpoint,
    update_reconfig_aws_endpoint,
    reload_environment,
    update_temp_auth_token,
    get_temp_info_user,
    reset_auth_token,
    get_auth_token
)

# Global condition variables
environment_data_ready = False      # Indicates if environment data is fully updated.
auth_token_received = False         # Tracks whether the authentication token is obtained.
facilities_received = False         # Tracks whether the facility list is fetched.
facility_id_received = False        # Tracks whether the facility ID is updated.
aws_iot_endpoint_connection = False # Tracks whether the connection to AWS IoT endpoint is established.

# Global counter (uint32)
count = np.uint32(0)

# Maps client_ip -> client_id_hex
ip_to_id_map = {}

# Maps client_ip -> current writer (so we can close the old writer if the same IP reconnects)
ip_to_writer_map = {}

# Server variables to control start/stop server
server = None
server_task = None
reconfig_server = None
environment_task_running = False
aws_task_running = False

def on_publish(client, userdata, mid):
    update_status_aws_server(True)
    print(f"[MQTT] Successfully published message ID={mid}")

# -----------------------------------------------
# Start TCP Server
# -----------------------------------------------
async def start_tcp_server():
    global server

    if server is not None:  # Prevent multiple instances
        print("[WARNING] TCP Server is already running.")
        return
    try:
        server = await asyncio.start_server(
            handle_client,
            host=environment.TCP_SERVER_HOST,
            port=environment.TCP_SERVER_PORT
        )

        addrs = ', '.join(str(sock.getsockname()) for sock in server.sockets)
        print(f"[Async] TCP Server is running on {addrs}")

        async with server:
            await server.serve_forever()
    
    except OSError as e:
        print(f"[ERROR] Failed to start TCP Server: {e}")

# -----------------------------------------------
# Stop TCP Server
# -----------------------------------------------
async def stop_tcp_server():
    global server, ip_to_writer_map

    if server is not None:
        print("[INFO] Stopping TCP Server...")

        # Close all active client connections
        for writer in list(ip_to_writer_map.values()):  # Close all active client sockets
            writer.close()
            try:
                await writer.wait_closed()  # Ensure closure
            except Exception as e:
                print(f"[WARNING] Error closing client socket: {e}")
                writer.transport.abort()  # Force close if needed

        ip_to_writer_map.clear()  # Clear all connections

        # Stop the server
        server.close()
        try:
            await asyncio.wait_for(server.wait_closed(), timeout=5)
        except asyncio.TimeoutError:
            print("[WARNING] Server took too long to close, forcing shutdown.")

        server = None
        print("[INFO] TCP Server stopped.")

# -----------------------------------------------
# Check connection aws with server
# -----------------------------------------------
def check_tcp_connection(host, port, timeout=5):
    try:
        # Resolve hostname
        addr_info = socket.getaddrinfo(host, port)
        if not addr_info:
            return False

        # Attempt to create a socket connection
        with socket.create_connection((host, port), timeout=timeout):
            return True
    except socket.gaierror:
        return False
    except (socket.timeout, ConnectionRefusedError, OSError):
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
# This function ensures that the system has:
# - A valid authentication token.
# - An updated facility list.
# - A registered Facility ID.
# -----------------------------------------------            
async def update_data_environment():
    global environment_data_ready, info_user_received, auth_token_received
    global facilities_received, facility_id_received, environment_task_running

    old_auth_token = None

    if environment_task_running:
        print("[WARNING] update_data_environment() is already running! Skipping duplicate execution.")
        return

    environment_task_running = True  # Set flag when function starts

    # Get data at the first time to go into the while
    reconfig_server = get_reconfig_server()
    stop_server = get_stop_server()
    start_server = get_start_server()

    info_update = get_reconfig_info()
    facility_id_update = get_reconfig_facility_id()

    # print(f"[INFO] Initial facility_id_update: {facility_id_update}")

    while not environment_data_ready or (reconfig_server and stop_server and not start_server):

        # get data to break the while when it false
        reconfig_server = get_reconfig_server()
        stop_server = get_stop_server()
        start_server = get_start_server()

        # Get information of user to get Authenticate
        credentials = get_temp_info_user()
        username, password = credentials["temp_username"], credentials["temp_password"]

        if not auth_token_received or info_update:
            print("[INFO] Fetching new auth token...")
            new_auth_token = http_client.login_access_token(username, password)

            if new_auth_token:
                update_auth_token(new_auth_token)
                auth_token_received = True
                facilities_received = False
                update_reconfig_info(False)
                update_reconfig_facility_list(True)
                update_temp_auth_token(None)
                print("[INFO] Auth token received and updated.")
            else:
                print("[WARNING] Failed to obtain auth token. Retrying...")

                if get_auth_token:
                    update_reconfig_info(False)
                    auth_token_received = True

                update_reconfig_info(True)
                update_auth_token(None)
                auth_token_received = False
                

        if auth_token_received and not facilities_received:
            print("[INFO] Fetching facility list...")
            facilities = http_client.get_facility_list()

            if facilities:
                update_facility_list(facilities)
                facilities_received = True
                print("[INFO] Facility list updated.")
            else:
                print("[WARNING] No facilities available.")

        if (facilities_received and not facility_id_received) or facility_id_update:
            facility_id = get_facility_id()

            if facility_id:
                update_reconfig_facility_id(False)
                facility_id_received = True
                print(f"[INFO] Facility ID updated: {facility_id}")

                # Register TCP server
                http_client.register_tcp_server(facility_id, environment.TCP_SERVER_NAME)

                # Update devices with new ID
                for value in ip_to_id_map.values():
                    http_client.register_device(value)

            else:
                print("[WARNING] Facility ID is missing. Retrying...")

        print(f"[INFO] Status - Auth Token: {auth_token_received}, Facilities: {facilities_received}, Facility ID: {facility_id_received}")

        if auth_token_received and facilities_received and facility_id_received:
            environment_data_ready = True
            reconfig_server = False
            print("[INFO] Environment data fully updated. Exiting loop.")
            break
        
        await asyncio.sleep(1)

    environment_task_running = False

# -----------------------------------------------
# Check the connection with AWS IoT Endpoint 
# before connect MQTT
# -----------------------------------------------
async def verify_aws_connection():
    global aws_task_running, aws_iot_endpoint_connection, environment_data_ready

    if aws_task_running:
        print("[WARNING] verify_aws_connection() is already running!")
        return

    aws_task_running = True  # Set flag when function starts

    # Ensure environment is fully set up before proceeding
    while not environment_data_ready:
        print("[INFO] Waiting for environment_data_ready...")
        await asyncio.sleep(1)

    print("[INFO] environment_data_ready is True. Proceeding with AWS IoT verification...")

    while not aws_iot_endpoint_connection or get_reconfig_aws_endpoint():
        if get_start_check_aws():
            print("[BackEnd] Detected START_CHECK_AWS flag. Checking AWS IoT connection...")

            if check_tcp_connection(environment.AWS_IOT_ENDPOINT, environment.MQTT_PORT):
                print("[INFO] MQTT Connected Successfully.")

                # Establish MQTT connection
                # mqtt_client.connect(environment.AWS_IOT_ENDPOINT, environment.MQTT_PORT, 60)
                # mqtt_client.loop_start()
                
                # Update connection status
                update_connection_aws(True)
                aws_iot_endpoint_connection = True
                update_reconfig_aws_endpoint(False)
                break
            else:
                print("[WARNING] Failed to establish TCP connection to AWS IoT.")
                
                # Update failure status
                update_connection_aws(False)
                aws_iot_endpoint_connection = False

            update_start_check_aws(False)  # Reset flag before retrying

        await asyncio.sleep(1)

    aws_task_running = False 

async def handle_client(reader: asyncio.StreamReader, writer: asyncio.StreamWriter):
    global count

    mqtt_client = None

    reload_environment()

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

                # ---------------- MQTT Setup ----------------
                mqtt_client = mqtt.Client(client_id=new_id_hex)

                mqtt_client.on_publish = on_publish

                print(environment.root_ca_path)

                # Configure SSL for secure connections to AWS IoT
                mqtt_client.tls_set(ca_certs=environment.root_ca_path,
                            certfile=environment.cert_path,
                            keyfile=environment.private_key_path,
                            tls_version=ssl.PROTOCOL_TLSv1_2)

                # Connect to broker
                mqtt_client.connect(environment.AWS_IOT_ENDPOINT, environment.MQTT_PORT, 60)

                # Start a background thread to handle the network loop
                mqtt_client.loop_start()

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

                        local_time = int(time.time())
                        # Publish the data to MQTT as JSON
                        index_data = {
                            "device_id": client_id,
                            "current_time": local_time,
                        }
                        index_data.update(parsed)  # merges the "parsed" data into index_data

                        json_data = json.dumps(index_data, indent=4)
                        print(json_data)

                        print(f"[Before Publish] Preparing to publish data for device {client_id}")
                        mqtt_client.publish(http_client.generate_topic(client_id), json_data, qos=1)

                    elif len(content_data) == 36:
                        parsed = handle_data.parse_36_byte_content(content_data)

                        local_time = int(time.time())
                        # Publish the data to MQTT as JSON
                        index_data = {
                            "device_id": client_id,
                            "current_time": local_time,
                        }
                        index_data.update(parsed)  # merges the "parsed" data into index_data

                        json_data = json.dumps(index_data, indent=4)
                        print(json_data)

                        print(f"[Before Publish] Preparing to publish data for device {client_id}")
                        mqtt_client.publish(http_client.generate_topic(client_id), json_data, qos=1)

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
    global server_task, aws_task_running, environment_task_running

    try:
        # Start background task for refreshing authentication tokens
        asyncio.create_task(refresh_auth_token())

        # ---------------- TCP Server Setup (asyncio) ----------------
        while True:
            start_server = get_start_server()
            stop_server = get_stop_server()
            reconfig_server = get_reconfig_server()

            if stop_server and server_task:
                print("[INFO] Stopping TCP Server...")
                asyncio.create_task(stop_tcp_server())  # Correctly run stop task
                server_task = None  # Reset task reference
                environment_task_running = False
                aws_task_running = False
                update_status_tcp_server(False)
                update_reconfig_server(True)

            elif start_server and not server_task:
                print("[INFO] Starting TCP Server...")
                server_task = asyncio.create_task(start_tcp_server())  # Start TCP server
                update_status_tcp_server(True)
                update_reconfig_server(False)

            if reconfig_server:
                if not environment_task_running:
                    print("[INFO] Starting environment update task...")
                    asyncio.create_task(update_data_environment())

                if not aws_task_running:
                    print("[INFO] Starting AWS connection verification task...")
                    asyncio.create_task(verify_aws_connection())

            await asyncio.sleep(1)

    except asyncio.CancelledError:
        print("[INFO] Tasks were cancelled.")

    except KeyboardInterrupt:
        print("\n[INFO] Shutting down gracefully...")

    finally:
        print("[INFO] Cleaning up before exit...")
        
if __name__ == '__main__':
    asyncio.run(main())