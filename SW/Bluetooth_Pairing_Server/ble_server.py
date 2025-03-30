from flask import Flask, jsonify, request, render_template
from wifi_config import WiFiConfigManager
from bluetooth_pair import BluetoothManager
import asyncio
import threading

app = Flask(__name__)

# Create a persistent asyncio event loop
ble_loop = asyncio.new_event_loop()
threading.Thread(target=ble_loop.run_forever, daemon=True).start()

wifi_config = WiFiConfigManager()
bluetooth = BluetoothManager()

@app.route("/scan_wifi")
def scan_wifi():
    result = wifi_config.scan_wifi()
    if isinstance(result, dict) and "error" in result:
        return jsonify(result), 500
    return jsonify(result)

@app.route('/send_wifi', methods=['POST'])
def send_wifi():
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

@app.route("/scan")
def scan():
    return jsonify(bluetooth.scan_devices())

@app.route("/connect/<mac>")
def connect(mac):
    return jsonify(bluetooth.connect(mac))

@app.route("/disconnect")
def disconnect():
    return jsonify(bluetooth.disconnect())


@app.route('/manage_device/<mac>')
def manage_device(mac):
    return render_template("manage_devices.html", mac=mac)

@app.route('/')
def index():
    return render_template('scan_devices.html')


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
