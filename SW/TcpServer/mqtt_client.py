import time
import paho.mqtt.client as mqtt

# Broker address (could be an IP or hostname)
MQTT_BROKER = "broker.emqx.io"  # Example using a public broker
MQTT_PORT = 1883
MQTT_TOPIC_PUBLISH = "test/topic_pub/python"

# Callback function called when the client successfully connects to the broker
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Successfully connected to the MQTT broker using MQTT v3.1.1!")
    else:
        print(f"Connection failed. Return code: {rc}")

# Callback function called when the client disconnects from the broker
def on_disconnect(client, userdata, rc):
    print("Disconnected from the MQTT broker.")

# Callback function called when there is a new message on subscribed topics
def on_message(client, userdata, msg):
    print(f"Received message from topic {msg.topic}: {msg.payload.decode('utf-8')}")

# Create a client instance for MQTT v3.1.1
client = mqtt.Client(client_id="PythonMQTTClient", protocol=mqtt.MQTTv311)

# Assign the callback functions
client.on_connect = on_connect
client.on_message = on_message
client.on_disconnect = on_disconnect

try:
    # Attempt to connect to the MQTT broker
    print(f"Connecting to broker {MQTT_BROKER}:{MQTT_PORT} using MQTT v3.1.1 ...")
    client.connect(MQTT_BROKER, MQTT_PORT, 60)

    # Start the client loop (non-blocking)
    client.loop_start()

    # Publish messages to the broker
    while True:
        message = f"Hello MQTT"
        client.publish(MQTT_TOPIC_PUBLISH, message)
        print(f"Sent: {message} -> {MQTT_TOPIC_PUBLISH}")
        time.sleep(5)

except KeyboardInterrupt:
    print("Program interrupted by user.")
finally:
    client.loop_stop()
    client.disconnect()
