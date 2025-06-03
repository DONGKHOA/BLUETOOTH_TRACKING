import http_thingsboard as http_thingsboard
import redis
import paho.mqtt.client as mqtt
import asyncio
import json
import environment
import os
from dotenv import load_dotenv

data_queue = None
mqtt_clients = {}
connected_clients = set()

load_dotenv()
QUEUE_HOST = os.getenv("QUEUE_HOST", "localhost")
QUEUE_PORT = int(os.getenv("QUEUE_PORT", 6380))
redis_client = redis.Redis(host=QUEUE_HOST, port=QUEUE_PORT, db=0)

# -----------------------------------------------
# Callback for logging connection
# -----------------------------------------------
def on_connect(client, userdata, flags, rc):
    client_id = client._client_id.decode()
    if rc == 0:
        print("Connected to ThingsBoard!")
        connected_clients.add(client_id)
        client.connected_flag = True
    else:
        print(f"[MQTT] Failed to connect: {rc}")
        client.connected_flag = False    

# -----------------------------------------------
# Handle MQTT Publish
# -----------------------------------------------
def on_publish(client, userdata, mid):
    print(f"[MQTT] Successfully published message ID={mid}")

# -----------------------------------------------
# Register MQTT client 
# -----------------------------------------------
async def register_and_connect_beacon(beacon_id):
    if beacon_id in mqtt_clients:
        return mqtt_clients[beacon_id]

    print(f"[INFO] Registering new beacon device: {beacon_id}")
    device, created = await http_thingsboard.register_device(beacon_id)

    if not device:
        print(f"[ERROR] Could not register beacon: {beacon_id}")
        return None

    token = await http_thingsboard.get_access_token(device)

    client = mqtt.Client(client_id=beacon_id)
    client.on_connect = on_connect
    client.on_publish = on_publish
    client.username_pw_set(token)

    client.connect(environment.THINGSBOARD_BROKER, environment.THINGSBOARD_PORT, 60)
    client.loop_start()

    for _ in range(10):
        if getattr(client, "connected_flag", False):
            break
        await asyncio.sleep(1)

    if not getattr(client, "connected_flag", False):
        print(f"[ERROR] MQTT connection failed for {beacon_id}")
        return None

    mqtt_clients[beacon_id] = client
    return client


# Listen to Redis queue and publish
async def redis_to_mqtt():
    while True:
        try:
            _, raw_data = redis_client.blpop("thingsboard_queue")
            data = json.loads(raw_data)
            gateways = data.get("gateways", [])
            beacon_id = data.get("beacon_id", "")
            timestamp = data.get("timestamp", "")
            
            print(f"[INFO] Processing data for beacon_id: {beacon_id}, timestamp: {timestamp}, gateways: {gateways}")

            for gw in gateways:
                client = await register_and_connect_beacon(beacon_id)
                if client:
                    payload = {
                        "beacon_id": beacon_id,
                        "timestamp": timestamp,
                        "gateway": gw
                    }
                    client.publish(environment.THINGSBOARD_TOPIC, json.dumps(payload), qos=1)
        except Exception as e:
            print(f"[ERROR] Failed to process MQTT publish: {e}")
        await asyncio.sleep(0.1)
    
# -----------------------------------------------
# Receive data from TCP server and push to queue
# -----------------------------------------------             
async def main():
    try:
        auth_token = http_thingsboard.login_auth_token(environment.USERNAME,environment.PASSWORD)
        redis_client.set("auth_token", auth_token)
        
        print("[INFO] Starting MQTT publisher loop...")
        await redis_to_mqtt()
            
    except asyncio.CancelledError:
        print("[INFO] Tasks were cancelled.")
    except KeyboardInterrupt:
        print("\n[INFO] Shutting down gracefully...")
    finally:
        for client in mqtt_clients.values():
            client.loop_stop()
            client.disconnect()
        print("[INFO] Cleaning up before exit...")
        
if __name__ == "__main__":
    asyncio.run(main())