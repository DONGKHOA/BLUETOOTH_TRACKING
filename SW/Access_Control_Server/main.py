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
    else:   
        print(f"Connection failed with code {rc}")

def on_message(client, userdata, msg):
    payload = msg.payload.decode()
    print(f"Received message on {msg.topic}: {payload}")

    # Notify the request handler via redis queue
    redis_client.rpush("mqtt_queue", payload)

async def process_request():
    loop = asyncio.get_event_loop()
    while True:
        # Wait for a message from the MQTT broker
        _, raw_payload = await loop.run_in_executor(None, redis_client.blpop, "mqtt_queue")
        payload_str = raw_payload.decode()  # convert from bytes to string

        try:
            data = json.loads(payload_str)
            command = data.get("command")
            print(f"Processing command: {command}")
        except Exception as e:
            print(f"Error while parsing JSON: {e}")

        # Process the request
        match command:
            case "USER_DATA":
                json_data = json.dumps(handle_data.reponse_user_data())
                mqtt_client.publish(RESPONSE_CLIENT_TOPIC, json_data, qos=0)
                print("Sent response:", json_data)
            
            case "ENROLL_FACE":
                user_id = data.get("id")
                json_data = json.dumps(handle_data.reponse_enroll_face(user_id))
                mqtt_client.publish(RESPONSE_CLIENT_TOPIC, json_data, qos=0)
                print("Sent response:", json_data)

            case "ENROLL_FINGER":
                user_id = data.get("id")
                json_data = json.dumps(handle_data.reponse_enroll_finger(user_id))
                mqtt_client.publish(RESPONSE_CLIENT_TOPIC, json_data, qos=0)
                print("Sent response:", json_data)

            case "AUTHENTICATE":
                user_id = data.get("id")
                json_data = json.dumps(handle_data.reponse_authenticate(user_id))
                mqtt_client.publish(RESPONSE_CLIENT_TOPIC, json_data, qos=0)
                print("Sent response:", json_data)

            case "ATTENDANCE":
                user_id = data.get("id")
                json_data = json.dumps(handle_data.reponse_attendance(user_id))
                mqtt_client.publish(RESPONSE_CLIENT_TOPIC, json_data, qos=0)
                print("Sent response:", json_data)

            case "DELETE_USER_DATA":
                print("Deleting user data")
                user_id = data.get("id")
                json_data = json.dumps({
                    "command": "DELETE_USER_DATA",
                    "id": user_id
                })
                mqtt_client.publish(REQUEST_CLIENT_TOPIC, json_data, qos=0)
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

