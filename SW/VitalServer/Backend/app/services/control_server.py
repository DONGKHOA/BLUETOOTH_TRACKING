# -----------------------------------------------
# Import libraries to initialize TCP Server
# -----------------------------------------------
import app.services.http_client as http_client
import app.services.handle_data as handle_data
import app.services.http_thingsboard as http_thingsboard

import asyncio
import numpy as np
import json
import ssl
import paho.mqtt.client as mqtt
import os

from datetime import datetime
import time

import redis

# -----------------------------------------------
# Import 
# -----------------------------------------------
import app.environment.environment as environment
import importlib
import socket

import app.services.control_server as control_server
import app.environment.environment_manager as environment_manager
import app.routes.api.system_log as system_log
# -----------------------------------------------
# Global variables
# -----------------------------------------------
server = None  # Store server reference

# Global counter (uint32)
count = np.uint32(0)

# Maps client_ip -> client_id_hex
ip_to_id_map = {}

# Maps client_ip -> current writer (so we can close the old writer if the same IP reconnects)
ip_to_writer_map = {}

server = None  # Stores the running TCP server task
server_task = None  # Stores the running TCP server task

data_queue = None

mqtt_clients = {}

r = redis.Redis(host='localhost', port=6379, db=0)

connected_clients = set()

# -----------------------------------------------
# Function to initialize the Redis
# -----------------------------------------------

def initialize_redis():
    if not r.exists("tcp_server_status"):
        r.set("tcp_server_status", "stopped")  # Initial state
    if not r.exists("aws_server_status"):
        r.set("aws_server_status", "disconnected")
    if not r.exists("auth_token"):
        r.set("auth_token", "")
    if not r.exists("facility_list"):
        r.set("facility_list", json.dumps(environment_manager.get_facility_list()))
    if not r.exists("aws_iot_endpoint"):
        r.set("aws_iot_endpoint", environment.AWS_IOT_ENDPOINT)
    if not r.exists("root_ca_path"):
        r.set("root_ca_path", environment.root_ca_path)
    if not r.exists("private_key_path"):
        r.set("private_key_path", environment.private_key_path)
    if not r.exists("cert_path"):
        r.set("cert_path", environment.cert_path)
    if not r.exists("username"):
        r.set("username", environment.USERNAME)
    if not r.exists("password"):
        r.set("password", environment.PASSWORD)

# -----------------------------------------------
# Function to start the TCP server
# -----------------------------------------------
async def start_tcp_server():
    global server_task
    if server_task is None:  # Prevent multiple instances
        print("[INFO] Starting TCP Server...")
        system_log.log_to_redis("[INFO] Starting TCP Server...")
        server_task = asyncio.create_task(run_tcp_server())
        r.set("tcp_server_status", "running")

# -----------------------------------------------
# TCP Server Runner
# -----------------------------------------------
async def run_tcp_server():
    global server, server_task

    if server is not None:  # Prevent multiple instances
        print("[WARNING] TCP Server is already running.")
        system_log.log_to_redis("[WARNING] TCP Server is already running.")
        return
    try:

        server = await asyncio.start_server(
            control_server.handle_client,
            host=environment.TCP_SERVER_HOST,
            port=environment.TCP_SERVER_PORT
        )
        addrs = ', '.join(str(sock.getsockname()) for sock in server.sockets)

        print(f"[Async] TCP Server is running on {addrs}")
        system_log.log_to_redis(f"[Async] TCP Server is running on {addrs}")

        async with server:
            await server.serve_forever()

        print("[DEBUG] Waiting 2 seconds before starting server...")
        system_log.log_to_redis("[DEBUG] Waiting 2 seconds before starting server...")

        await asyncio.sleep(2)

    except OSError as e:
        print(f"[ERROR] Failed to start TCP Server: {e}")
        system_log.log_to_redis(f"[ERROR] Failed to start TCP Server: {e}")

# -----------------------------------------------
# Function to stop the TCP server
# -----------------------------------------------
async def stop_tcp_server():
    global server, server_task, ip_to_writer_map
    if server is not None:
        print("[INFO] Stopping TCP Server...")
        system_log.log_to_redis("[INFO] Stopping TCP Server...")

        # Close all active client connections
        for writer in list(ip_to_writer_map.values()):  # Close all active client sockets
            writer.close()
            try:
                await writer.wait_closed()  # Ensure closure
            except Exception as e:
                print(f"[WARNING] Error closing client socket: {e}")
                system_log.log_to_redis(f"[WARNING] Error closing client socket: {e}")
                writer.transport.abort()  # Force close if needed

        ip_to_writer_map.clear()  # Clear all connections

        # Stop the server
        server.close()
        try:
            await asyncio.wait_for(server.wait_closed(), timeout=5)
        except asyncio.TimeoutError:
            print("[WARNING] Server took too long to close, forcing shutdown.")
            system_log.log_to_redis("[WARNING] Server took too long to close, forcing shutdown.")

        server = None
        server_task = None
            
        for device_id, client in mqtt_clients.items():
            try:
                client.loop_stop()
                client.disconnect()
                print(f"[MQTT] Disconnected client for device {device_id}")
            except Exception as e:
                print(f"[WARNING] Failed to disconnect client {device_id}: {e}")
        mqtt_clients.clear()

        r.set("tcp_server_status", "stopped")
    
        system_log.log_to_redis("[INFO] TCP Server stopped.")
        print("[INFO] TCP Server stopped.")

# -----------------------------------------------
# Background task to refresh the AUTH_TOKEN
# every 3 hours (10800 seconds)
# -----------------------------------------------
async def refresh_auth_token():
    while True:
        await asyncio.sleep(10800)  # 3 hours in seconds
        try:
            # auth_token = http_client.login_access_token(r.get("username").decode(), r.get("password").decode())
            auth_token = http_thingsboard.login_auth_token(r.get("username").decode(), r.get("password").decode())
            r.set("auth_token", auth_token)
            system_log.log_to_redis("[Auth Refresh] Successfully refreshed AUTH_TOKEN.")
            print("[Auth Refresh] Successfully refreshed AUTH_TOKEN.")
        except Exception as e:
            system_log.log_to_redis(f"[Auth Refresh] Exception while refreshing token: {e}")
            print(f"[Auth Refresh] Exception while refreshing token: {e}")
# -----------------------------------------------
# Callback for logging connection
# -----------------------------------------------
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        client_id = client._client_id.decode()

        if client_id not in connected_clients:
            
            print("Connected to ThingsBoard!")
            system_log.log_to_redis(f"Connected to ThingsBoard! ({client_id})")
            connected_clients.add(client_id)
        
        client.connected_flag = True    
        
    else:
        print("Connection failed with code", rc)

# -----------------------------------------------
# Handle MQTT Publish
# -----------------------------------------------
def on_publish(client, userdata, mid):

    r.set("aws_server_status", "connected")

    print(f"[MQTT] Successfully published message ID={mid}")
    system_log.log_to_redis(f"[MQTT] Successfully published message ID={mid}")

# -----------------------------------------------
# Function to publish data to MQTT
# ----------------------------------------------- 
async def publish_to_mqtt():
    global data_queue
    while True:
        json_data = await data_queue.get()
        device_data = json.loads(json_data)
        device_id = device_data.get("device_id")

        client = mqtt_clients.get(device_id)

        if client and client.is_connected():
            result = client.publish(environment.TOPIC, json_data, qos=1)
            result.wait_for_publish()
        else:
            print(f"[ERROR] No MQTT client or not connected for device: {device_id}")
            system_log.log_to_redis(f"[ERROR] No MQTT client or not connected for device: {device_id}")
    
# -----------------------------------------------
# Function to handle client connections
# -----------------------------------------------        
async def handle_client(reader: asyncio.StreamReader, writer: asyncio.StreamWriter):
    global count, data_queue, connected_flag

    condition_regis_mqtt = True

    client_address = writer.get_extra_info('peername')
    if not client_address:
        # Some cases where writer.get_extra_info('peername') could return None (if the socket closed quickly)
        print("[handle_client] Could not retrieve peername (client_address)")
        system_log.log_to_redis("[handle_client] Could not retrieve peername (client_address)")

        writer.close()
        await writer.wait_closed()
        return

    client_ip = client_address[0]

    print(f"[Async] Handling new client from IP={client_ip}")
    system_log.log_to_redis(f"[Async] Handling new client from IP={client_ip}")

    # If an existing writer for this IP exists, close it (enforcing "one IP - one connection")
    if client_ip in ip_to_writer_map:
        old_writer = ip_to_writer_map[client_ip]
        try:
            old_writer.close()
        except:
            pass
        await old_writer.wait_closed()

        print(f"[Async] Closed old connection for IP={client_ip}")
        system_log.log_to_redis(f"[Async] Closed old connection for IP={client_ip}")

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
                system_log.log_to_redis(f"[Async] Connection error from {client_ip}: {e}")
                break

            if not data:
                # Client closed the connection
                print(f"[Async] Client {client_ip} disconnected.")
                system_log.log_to_redis(f"[Async] Client {client_ip} disconnected.")
                break

            print(f"Received data (hex): {data.hex()}")
            system_log.log_to_redis(f"Received data (hex): {data.hex()}")

            request_id, function, content_len, content_data = handle_data.parse_packet(data)

            print(f"[Parsed] function=0x{function:04x}, content_len={content_len}, content_data={content_data.hex()}")
            system_log.log_to_redis(f"[Parsed] function=0x{function:04x}, content_len={content_len}, content_data={content_data.hex()}")
            # ---------------- Handle function=0x0001 (register ID, send count) ----------------
            if function == 0x0001:
                # Suppose the last 13 bytes are the ID
                if len(content_data) >= 13:
                    new_id_hex = content_data[-13:].hex()
                else:
                    new_id_hex = "TooShort"

                # ---------------- MQTT Setup ----------------
                if condition_regis_mqtt == True:
                    condition_regis_mqtt = False

                    # new_device = await http_client.register_device(new_id_hex)
                    
                    new_device, created = await http_thingsboard.register_device(new_id_hex)
                    
                    if new_device:
                        token = await http_thingsboard.get_access_token(new_device)
                        
                        client = mqtt.Client(client_id=client_ip)
                        client.connected_flag = False
                        client.on_connect = on_connect
                        client.on_publish = on_publish
                        client.username_pw_set(token)

                        client.connect(environment.MQTT_BROKER, environment.MQTT_PORT, 60)
                        client.loop_start()
                        
                        for i in range(10):
                            if client.connected_flag:
                                break
                            print("Waiting for connection...")
                            time.sleep(1)
                            
                        mqtt_clients[new_id_hex] = client
                        
                        print(f"[Server] Device successfully registered: {new_device}")
                        system_log.log_to_redis(f"[Server] Device successfully registered: {new_device}")

                ip_to_id_map[client_ip] = new_id_hex

                print(f"[Server] (1) Registered new: IP={client_ip}, ID={new_id_hex}. count={count}")
                system_log.log_to_redis(f"[Server] (1) Registered new: IP={client_ip}, ID={new_id_hex}. count={count}")

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
                system_log.log_to_redis(f"[Server] Sent response for function=1 with count={count}")

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
                    system_log.log_to_redis(f"[Server] Sent request for function=0x0410 with count={count}")

                else:
                    if len(content_data) == 28:
                        parsed = handle_data.parse_28_byte_content(content_data)

                        handle_data.check_in_out_of_bed(parsed)

                        local_time = int(time.time())
                        # Publish the data to MQTT as JSON
                        index_data = {
                            "device_id": client_id,
                            "current_time": local_time,
                        }
                        index_data.update(parsed)  # merges the "parsed" data into index_data

                        json_data = json.dumps(index_data, indent=4)
                        print(json_data)
                        system_log.log_to_redis(json_data)

                        print(f"[Before Publish] Preparing to publish data for device {client_id}")
                        system_log.log_to_redis(f"[Before Publish] Preparing to publish data for device {client_id}")
                        
                        await data_queue.put(json_data)

                    elif len(content_data) == 36:
                        parsed = handle_data.parse_36_byte_content(content_data)

                        handle_data.check_in_out_of_bed(parsed)

                        local_time = int(time.time())
                        # Publish the data to MQTT as JSON
                        index_data = {
                            "device_id": client_id,
                            "current_time": local_time,
                        }
                        index_data.update(parsed)  # merges the "parsed" data into index_data

                        json_data = json.dumps(index_data, indent=4)
                        print(json_data)
                        system_log.log_to_redis(json_data)

                        print(f"[Before Publish] Preparing to publish data for device {client_id}")
                        system_log.log_to_redis(f"[Before Publish] Preparing to publish data for device {client_id}")

                        await data_queue.put(json_data)

                    else:
                        print(f"[!] content_data length={len(content_data)}, expected 28 or 36.")
                        system_log.log_to_redis(f"[!] content_data length={len(content_data)}, expected 28 or 36.")

            elif function == 0x0410:
                new_id_hex = content_data[-13:].hex()
                ip_to_id_map[client_ip] = new_id_hex

                # ---------------- MQTT Setup ----------------
                if condition_regis_mqtt == True:
                    condition_regis_mqtt = False

                    # new_device = await http_client.register_device(new_id_hex)
                    
                    new_device, created = await http_thingsboard.register_device(new_id_hex)
                    
                    if new_device:
                        token = await http_thingsboard.get_access_token(new_device)
                        
                        client = mqtt.Client(client_id=client_ip)
                        client.connected_flag = False
                        client.on_connect = on_connect
                        client.on_publish = on_publish
                        client.username_pw_set(token)

                        client.connect(environment.MQTT_BROKER, environment.MQTT_PORT, 60)
                        client.loop_start()
                        
                        for i in range(10):
                            if client.connected_flag:
                                break
                            print("Waiting for connection...")
                            time.sleep(1)
                            
                        mqtt_clients[new_id_hex] = client
                        
                        print(f"[Server] Device successfully registered: {new_device}")
                        system_log.log_to_redis(f"[Server] Device successfully registered: {new_device}")

                print(f"[Server] (1) Registered new: IP={client_ip}, ID={new_id_hex}. count={count}")
                system_log.log_to_redis(f"[Server] (1) Registered new: IP={client_ip}, ID={new_id_hex}. count={count}")

    except Exception as e:
        print(f"[Async] Exception for {client_ip}: {e}")
        system_log.log_to_redis(f"[Async] Exception for {client_ip}: {e}")

    finally:
        # Close connection and remove from maps
        writer.close()
        await writer.wait_closed()
        if client_ip in ip_to_id_map:
            del ip_to_id_map[client_ip]

            print(f"[Async] Removed IP={client_ip} from ip_to_id_map.")
            system_log.log_to_redis(f"[Async] Removed IP={client_ip} from ip_to_id_map.")

        if ip_to_writer_map.get(client_ip) is writer:
            del ip_to_writer_map[client_ip]

        print(f"[Async] Ended for {client_ip}.")
        system_log.log_to_redis(f"[Async] Ended for {client_ip}.")

        # Decrease 'count' if close connection
        count = count - 1
     
async def main():
    global data_queue
    
    try:
        r.flushall()

        initialize_redis()
        
        data_queue = asyncio.Queue()

        # auth_token = http_client.login_access_token(r.get("username").decode(),r.get("password").decode())
        
        auth_token = http_thingsboard.login_auth_token(r.get("username").decode(),r.get("password").decode())

        if auth_token:
            r.set("auth_token", auth_token)
            asyncio.create_task(refresh_auth_token())
        
        # Create a publish task
        asyncio.create_task(publish_to_mqtt())
        
        while True:
            status = r.get("tcp_server_status").decode()
            if status == "running":
                await start_tcp_server()
            else:
                await stop_tcp_server()

            await asyncio.sleep(1)
            
    except asyncio.CancelledError:
        print("[INFO] Tasks were cancelled.")
        system_log.log_to_redis("[INFO] Tasks were cancelled.")
    except KeyboardInterrupt:
        print("\n[INFO] Shutting down gracefully...")
        system_log.log_to_redis("\n[INFO] Shutting down gracefully...")
    finally:
        print("[INFO] Cleaning up before exit...")
        system_log.log_to_redis("[INFO] Cleaning up before exit...")