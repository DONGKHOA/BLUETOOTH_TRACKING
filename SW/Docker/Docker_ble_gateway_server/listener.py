import paho.mqtt.client as mqtt
import json
import time
import os
import redis
from dotenv import load_dotenv
from system_log import log_to_redis

load_dotenv()

# Change the host IP address
BROKER_HOST = os.getenv("BROKER_HOST", "nanomq")
BROKER_PORT = int(os.getenv("BROKER_PORT", 1883))
QUEUE_HOST = os.getenv("QUEUE_HOST", "redis")
QUEUE_PORT = int(os.getenv("QUEUE_PORT", 6379))

redis_client = redis.Redis(host=QUEUE_HOST, port=QUEUE_PORT, db=0)

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print(f"Connected to {BROKER_HOST}:{BROKER_PORT}")
        log_to_redis(f"Connected to {BROKER_HOST}:{BROKER_PORT}", source="listener")
        client.subscribe("bluetooth/+/data")
    else:   
        print(f"Connection failed with code {rc}")
        log_to_redis(f"Connection failed with code {rc}", source="listener")

def on_message(client, userdata, msg):
    try:

        topic = msg.topic
        payload = json.loads(msg.payload.decode())

        #Extract Gateway ID from Topic
        gateway_id = topic.split("/")[1]
        print(f"Extracted Gateway ID: {gateway_id}")  # Debugging gateway_id
        
        # Extract beacon data
        dev_list = payload.get("dev_list", [])

        for i, device in enumerate(dev_list, start=1):
            tag_id = device.get("mac", "N/A")  # MAC address is the tag_id
            rssi = device.get("rssi", "N/A")
            timestamp = int(time.time())

            print(f"Gateway ID: {gateway_id}, Beacon {i}: Tag_ID: {tag_id}, RSSI: {rssi}, Timestamp: {timestamp}")
            log_to_redis(f"Gateway ID: {gateway_id}, Beacon {i}: Tag_ID: {tag_id}, RSSI: {rssi}, Timestamp: {timestamp}", source="listener")
            # Store in Redis
            redis_client.rpush("beacon_data", json.dumps({
                "gateway_id": gateway_id,
                "tag_id": tag_id,
                "rssi": rssi,
                "timestamp": timestamp,
                "flag_timeout": 1
            }))

            print(f"Pushed {len(dev_list)} beacon(s) from {gateway_id} to Redis")
    except json.JSONDecodeError:
        print(f"Received non-JSON message on '{topic}': {msg.payload.decode()}")
        log_to_redis(f"Received non-JSON message on '{topic}': {msg.payload.decode()}", source="listener")
    except Exception as e:
        print(f"Error processing message: {e}")
        log_to_redis(f"Error processing message: {e}", source="listener")

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

try:
    client.connect(BROKER_HOST, BROKER_PORT, 60)
    client.loop_forever()
except ConnectionRefusedError:
    print(f"Failed to connect to {BROKER_HOST}:{BROKER_PORT}. Ensure NanoMQ is running.")
except Exception as e:
    print(f"Error: {e}")