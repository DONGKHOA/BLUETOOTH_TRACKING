import paho.mqtt.client as mqtt
import json

# MQTT configuration
BROKER = "demo.thingsboard.io"
PORT = 1883
ACCESS_TOKEN = "N2lKUCqnCYu8KejXOIuF"
TOPIC = "v1/devices/me/telemetry"

# The message to publish (match your mosquitto_pub)
# payload = '{"temperature":23}'  # send raw string exactly like in command

payload = {
    "temperature": 25
}

# Callback for logging connection
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to ThingsBoard!")
        client.connected_flag = True
    else:
        print("Connection failed with code", rc)

# Callback for logging publish
def on_publish(client, userdata, mid):
    print("Data published.")

# Setup client
mqtt.Client.connected_flag = False
client = mqtt.Client()

# Set username (ThingsBoard uses access token as username)
client.username_pw_set(ACCESS_TOKEN)

# Attach callbacks
client.on_connect = on_connect
client.on_publish = on_publish

# Connect to broker
client.connect(BROKER, PORT, 60)
client.loop_start()

# Wait until connected
import time
for i in range(10):
    if client.connected_flag:
        break
    print("Waiting for connection...")
    time.sleep(1)

# Publish message if connected
if client.connected_flag:
    result = client.publish(TOPIC, json.dumps(payload), qos=1)
    result.wait_for_publish()
else:
    print("Failed to connect. Cannot publish.")

# Cleanup
time.sleep(1)
client.loop_stop()
client.disconnect()
