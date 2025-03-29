from flask import Flask, jsonify, request, render_template
import asyncio
import threading
from bleak import BleakScanner, BleakClient

import subprocess
import re

app = Flask(__name__)

client = None
write_char = None
connected = False

# Create a persistent asyncio event loop
ble_loop = asyncio.new_event_loop()
threading.Thread(target=ble_loop.run_forever, daemon=True).start()

#-------------------------------------------------------------
# BLUETOOTH SCAN
#-------------------------------------------------------------
@app.route('/scan')
def scan():
    async def do_scan():
        devices = await BleakScanner.discover(timeout=5)
        return [{
            "name": d.name,
            "address": d.address,
            "rssi": d.rssi
        } for d in devices if d.name and "ESP_GATTS_DEMO" in d.name]
    
    future = asyncio.run_coroutine_threadsafe(do_scan(), ble_loop)
    return jsonify(future.result())


@app.route('/connect/<mac>')
def connect(mac):
    global client, write_char, connected

    async def keep_alive_loop():
        global client, connected
        try:
            while client and await client.is_connected():
                await asyncio.sleep(10)
        except Exception as e:
            print("Keep-alive failed:", e)
        finally:
            connected = False
            print("Disconnected from device.")

    async def do_connect():
        global client, write_char, connected
        try:
            c = BleakClient(mac)
            await c.connect()
            if await c.is_connected():
                client_services = await c.get_services()

                for service in client_services:
                    for char in service.characteristics:
                        if "write" in char.properties:
                            write_char = char.uuid
                            client = c
                            connected = True
                            asyncio.run_coroutine_threadsafe(keep_alive_loop(), ble_loop)
                            return {
                                "connected": True,
                                "write_char": write_char,
                                "services": [str(service.uuid) for service in client_services]
                            }
                return {
                    "connected": True,
                    "write_char": None,
                    "message": "No writable characteristic found"
                }
            else:
                return {"connected": False}
        except Exception as e:
            return {"connected": False, "error": str(e)}

    future = asyncio.run_coroutine_threadsafe(do_connect(), ble_loop)
    return jsonify(future.result())

@app.route('/send', methods=['POST'])
def send():
    global client, write_char, connected
    message = request.json.get("message", "")

    async def do_send():
        if client and connected and write_char:
            try:
                await client.write_gatt_char(write_char, message.encode())
                return {"sent": True, "message": message}
            except Exception as e:
                return {"sent": False, "error": str(e)}
        return {"sent": False, "error": "Not connected or no writable char"}

    future = asyncio.run_coroutine_threadsafe(do_send(), ble_loop)
    return jsonify(future.result())


@app.route('/manage_device/<mac>')
def manage_device(mac):
    return render_template("manage_devices.html", mac=mac)


#-------------------------------------------------------------
# NETWORK TAB
#-------------------------------------------------------------
@app.route("/scan_wifi")
def scan_wifi():
    try:
        # Use tab-separated output to handle SSIDs with spaces
        result = subprocess.check_output(
            ["nmcli", "-t", "-f", "SSID", "dev", "wifi"],
            text=True
        )

        # Split and remove empty + duplicate SSIDs
        raw_ssids = result.strip().split("\n")
        unique_ssids = sorted(set(ssid.strip() for ssid in raw_ssids if ssid.strip()))

        return jsonify(unique_ssids)

    except Exception as e:
        return jsonify({"error": str(e)}), 500


@app.route('/send_wifi', methods=['POST'])
def send_wifi():
    global client, write_char
    data = request.json
    ssid = data.get('ssid')
    password = data.get('password')

    async def do_send_wifi():
        if client and write_char:
            try:
                payload = f"WIFI:{ssid}|{password}"
                await client.write_gatt_char(write_char, payload.encode())
                return {"sent": True}
            except Exception as e:
                return {"sent": False, "error": str(e)}
        return {"sent": False, "error": "Not connected or writable characteristic missing"}

    return jsonify(asyncio.run(do_send_wifi()))


@app.route('/')
def index():
    return render_template('scan_devices.html')


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
