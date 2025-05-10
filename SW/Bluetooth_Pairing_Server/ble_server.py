from flask import Flask, jsonify, request, render_template

import config_stored
config_stored.load_configs()

from wifi_config import WiFiConfigManager
from bluetooth_pair import BluetoothManager
from mqtt_config import MQTTConfigManager
import asyncio
import threading
# import config_stored

app = Flask(__name__)

# Create a persistent asyncio event loop
ble_loop = asyncio.new_event_loop()
threading.Thread(target=ble_loop.run_forever, daemon=True).start()

#--------------------------------------------------------
# Initialize the Bluetooth manager, WiFi config manager
# and MQTT config manager
#--------------------------------------------------------

wifi_config = WiFiConfigManager()
bluetooth = BluetoothManager()
mqtt_config = MQTTConfigManager()

#--------------------------------------------------------
#----------------- Bluetooth API-------------------------
#--------------------------------------------------------
@app.route("/scan")
def scan():
    return jsonify(bluetooth.scan_devices())

@app.route("/connect/<mac>")
def connect(mac):
    return jsonify(bluetooth.connect(mac))

@app.route("/disconnect")
def disconnect():
    return jsonify(bluetooth.disconnect())

#--------------------------------------------------------
#--------------------- Wi-Fi API-------------------------
#--------------------------------------------------------
@app.route("/scan_wifi")
def scan_wifi():
    result = wifi_config.scan_wifi()
    if isinstance(result, dict) and "error" in result:
        return jsonify(result), 500
    return jsonify(result)

@app.route('/send_wifi', methods=['POST'])
def send_wifi_route():
    global client, write_char, ble_loop
    data = request.json
    ssid = data.get('ssid')
    password = data.get('password')
    mac = data.get('mac')

    if not mac:
        return jsonify({"sent": False, "error": "MAC address missing"}), 400

    result = wifi_config.send_wifi_credentials(
        bluetooth.client,
        bluetooth.write_char,
        bluetooth.ble_loop,
        ssid,
        password,
        mac
    )
    return jsonify(result)

@app.route("/get_wifi_config/<mac>")
def get_wifi_config(mac):
    return jsonify(wifi_config.get_config(mac))

#--------------------------------------------------------
#----------------- MQTT API------------------------------
#--------------------------------------------------------
@app.route("/send_mqtt_server", methods=["POST"])
def send_mqtt_server_route():
    global client, write_char, ble_loop

    data = request.json
    server = data.get("server")
    mac = data.get('mac')

    if not server:
        return jsonify({"error": "Missing server"}), 400

    result = mqtt_config.send_mqtt_server(
        bluetooth.client,
        bluetooth.write_char,
        bluetooth.ble_loop,
        server,
        mac
    )

    return jsonify({"saved": True})

@app.route("/send_mqtt_topic", methods=["POST"])
def send_mqtt_topic():
    global client, write_char, ble_loop

    data = request.json
    topic = data.get("topic")
    mac = data.get('mac')

    if not topic:
        return jsonify({"error": "Missing topic"}), 400
        
    result = mqtt_config.send_mqtt_topic(
        bluetooth.client,
        bluetooth.write_char,
        bluetooth.ble_loop,
        topic,
        mac
    )

    return jsonify({"saved": True})

@app.route("/send_room", methods=["POST"])
def send_room():
    global client, write_char, ble_loop

    data = request.json
    room = data.get("room")
    mac = data.get('mac')

    if not room:
        return jsonify({"error": "Missing room"}), 400
        
    result = mqtt_config.send_room(
        bluetooth.client,
        bluetooth.write_char,
        bluetooth.ble_loop,
        room,
        mac
    )

    return jsonify({"saved": True})

@app.route("/get_mqtt_config/<mac>")
def get_mqtt_config(mac):
    return jsonify(mqtt_config.get_config(mac))

#--------------------------------------------------------
#----------------- Home Screen API-----------------------
#--------------------------------------------------------
@app.route('/manage_device/<mac>')
def manage_device(mac):
    return render_template("manage_devices.html", mac=mac)

@app.route('/')
def index():
    return render_template('scan_devices.html')

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
