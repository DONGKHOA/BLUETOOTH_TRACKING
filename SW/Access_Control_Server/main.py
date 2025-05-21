import json
import asyncio
import paho.mqtt.client as mqtt
import handle_data
import redis

# MQTT broker details
BROKER = "broker.emqx.io"
PORT = 1883

REQUEST_SERVER_TOPIC = 'ACCESS_CONTROL/Server/Request'
RESPONSE_SERVER_TOPIC = 'ACCESS_CONTROL/Server/Response'

REQUEST_CLIENT_TOPIC = 'ACCESS_CONTROL/Client/Request'
RESPONSE_CLIENT_TOPIC = 'ACCESS_CONTROL/Client/Response'

redis_client = redis.Redis(host="127.0.0.1", port=6379, db=0)

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print(f"Connected to {BROKER}:{PORT}")
        mqtt_client.subscribe(REQUEST_SERVER_TOPIC)
        mqtt_client.subscribe(RESPONSE_SERVER_TOPIC)
        
        client.subscribe("+/Server/Request")
        client.subscribe("+/Server/Response")
    else:   
        print(f"Connection failed with code {rc}")

def on_message(client, userdata, msg):
    payload = msg.payload.decode()
    print(f"Received message on {msg.topic}: {payload}")

    try:
        device_id = msg.topic.split('/')[0] # Extract device ID from topic
        data = json.loads(payload)
        data["device_id"] = device_id
        redis_client.rpush("mqtt_queue", json.dumps(data))
    except Exception as e:
        print(f"Failed to process message: {e}")

async def process_request():
    loop = asyncio.get_event_loop()
    while True:
        # Wait for a message from the MQTT broker
        _, raw_payload = await loop.run_in_executor(None, redis_client.blpop, "mqtt_queue")
        payload_str = raw_payload.decode()  # convert from bytes to string

        try:
            data = json.loads(payload_str)
            device_id = data.get("device_id")
            command = data.get("command")
            print(f"Processing command: {command} from {device_id}")
        except Exception as e:
            print(f"Error while parsing JSON: {e}")
            continue

        # Process the request
        match command:
            case "SYN":
                print("SYNC Command")
                json_data = json.dumps({
                    "command": "SYN",
                    "response": "success"
                })
                mqtt_client.publish(f"{device_id}/Client/Response", json_data, qos=1)
                print("Sent response:", json_data)
                
            case "SYNC_DATA":
                json_data = json.dumps(handle_data.reponse_user_data(data, device_id))
                
            case "ATTENDANCE_DATA":
                json_data = json.dumps(handle_data.response_attendance_data(data, device_id))
            
            case "ENROLL_FACE":
                user_id = data.get("id")
                json_data = json.dumps(handle_data.response_enroll_face(user_id, device_id))
                
                mqtt_client.publish(f"{device_id}/Client/Response", json_data, qos=1)
                print("Sent response:", json_data)

            case "ENROLL_FINGERPRINT":
                user_id = data.get("id")
                json_data = json.dumps(handle_data.response_enroll_finger(user_id, device_id))
                
                mqtt_client.publish(f"{device_id}/Client/Response", json_data, qos=1)
                print("Sent response:", json_data)

            case "ATTENDANCE":
                user_id = data.get("id")
                time = data.get("timestamp")
                json_data = json.dumps(handle_data.response_attendance(user_id, device_id, time))
                
                mqtt_client.publish(f"{device_id}/Client/Response", json_data, qos=1)
                print("Sent response:", json_data)

            case "ADD_USER_DATA":
                print("Adding user data")
                json_data = json.dumps({
                    "command": "ADD_USER",
                    "id": data.get("id"),
                    "name": data.get("name"),
                })
                mqtt_client.publish(f"{device_id}/Client/Response", json_data, qos=1)
                print("Sent response:", json_data)
                
            case "DELETE_USER_DATA":
                print("Deleting user data")
                json_data = json.dumps({
                    "command": "DELETE_USER_DATA",
                    "id": data.get("id")
                })
                mqtt_client.publish(f"{device_id}/Client/Response", json_data, qos=1)
                print("Sent response:", json_data)

            case "SET_ROLE":
                print("Set new role of user")
                json_data = json.dumps({
                    "command": "SET_ROLE",
                    "id": data.get("id"),
                    "role": data.get("role")
                })
                mqtt_client.publish(f"{device_id}/Client/Response", json_data, qos=1)
                print("Sent response:", json_data)

            case "DELETE_FINGER_USER":
                print("Deleting finger user data")
                json_data = json.dumps({
                    "command": "DELETE_FINGER_USER",
                    "id": data.get("id")
                })
                mqtt_client.publish(f"{device_id}/Client/Response", json_data, qos=1)
                print("Sent response:", json_data)
                
            case "DELETE_FACEID_USER":
                print("Deleting faceid user data")
                json_data = json.dumps({
                    "command": "DELETE_FACEID_USER",
                    "id": data.get("id")
                })
                mqtt_client.publish(f"{device_id}/Client/Response", json_data, qos=1)
                print("Sent response:", json_data)


async def main():
    mqtt_client.loop_start()
    await process_request()

# Setup MQTT client
mqtt_client = mqtt.Client()
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message
mqtt_client.connect(BROKER, PORT, 60)

if __name__ == "__main__":
    asyncio.run(main())

