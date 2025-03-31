import json
import asyncio
import paho.mqtt.client as mqtt
import handle_data

# import threading
# from ui import app 

# MQTT broker details
BROKER = "broker.emqx.io"
PORT = 1883
REQUEST_CLIENT_TOPIC = 'ACCESS_CONTROL/Client/Request'
RESPONSE_SERVER_TOPIC = 'ACCESS_CONTROL/Server/Response'

REQUEST_SERVER_TOPIC = 'ACCESS_CONTROL/Server/Request'
RESPONSE_CLIENT_TOPIC = 'ACCESS_CONTROL/Client/Response'

queue = asyncio.Queue()
event_loop = asyncio.get_event_loop()

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

    # Notify the request handler via async queue
    asyncio.run_coroutine_threadsafe(queue.put(payload), event_loop)

def enqueue_internal_command(command_dict):
    print("[enqueue] sending to queue:", command_dict)
    asyncio.run_coroutine_threadsafe(queue.put(command_dict), event_loop)

async def process_request():
    while True:
        # Wait for a message from the MQTT broker
        payload = await queue.get()
        print(f"Processing payload: {payload}")

        data = json.loads(payload)
        command = data.get("command")
        print(f"Processing command: {command}")

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

            # case "DELETE_USER_DATA":
            #     print("Deleting user data")
            #     user_id = data.get("id")
            #     json_data = json.dumps({
            #         "command": "DELETE_USER_DATA",
            #         "id": user_id
            #     })
            #     mqtt_client.publish(REQUEST_CLIENT_TOPIC, json_data, qos=0)
            #     print("Sent response:", json_data)


async def main():
    global event_loop
    event_loop = asyncio.get_running_loop()

    mqtt_client.loop_start()
    await process_request()

# def run_flask():
#     app.run(host="0.0.0.0", port=9000, debug=True, use_reloader=False)

# Setup MQTT client
mqtt_client = mqtt.Client()
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message
mqtt_client.connect(BROKER, PORT, 60)

if __name__ == "__main__":
    # flask_thread = threading.Thread(target=run_flask)
    # flask_thread.start()

    asyncio.run(main())

