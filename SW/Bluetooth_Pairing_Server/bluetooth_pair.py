import asyncio
import threading
from bleak import BleakScanner, BleakClient

class BluetoothManager:
    def __init__(self):
        self.client = None
        self.write_char = None
        self.connected = False
        self.ble_loop = asyncio.new_event_loop()
        threading.Thread(target=self.ble_loop.run_forever, daemon=True).start()

    def run_async(self, coro):
        return asyncio.run_coroutine_threadsafe(coro, self.ble_loop).result()

    def scan_devices(self):
        async def do_scan():
            devices = await BleakScanner.discover(timeout=5)
            return [{
                "name": d.name,
                "address": d.address,
                "rssi": d.rssi
            } for d in devices if d.name and ("BLUETOOTH_GATEWAY" in d.name or "ACCESS_CONTROL" in d.name)]
        return self.run_async(do_scan())

    def connect(self, mac):
        async def keep_alive_loop():
            try:
                while self.client and await self.client.is_connected():
                    await asyncio.sleep(10)
            except Exception as e:
                print("Keep-alive failed:", e)
            finally:
                self.connected = False
                print("Disconnected from device.")

        async def do_connect():
            try:
                c = BleakClient(mac)
                await c.connect()
                if await c.is_connected():
                    client_services = await c.get_services()
                    for service in client_services:
                        for char in service.characteristics:
                            if "write" in char.properties:
                                self.write_char = char.uuid
                                self.client = c
                                self.connected = True
                                asyncio.run_coroutine_threadsafe(keep_alive_loop(), self.ble_loop)
                                return {
                                    "connected": True,
                                    "write_char": self.write_char,
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

        return self.run_async(do_connect())

    def disconnect(self):
        async def do_disconnect():
            if self.client:
                try:
                    await self.client.disconnect()
                    print("Disconnected from ESP32.")
                except Exception as e:
                    print("Error during disconnect:", e)
            self.client = None
            self.connected = False
            return {"disconnected": True}

        return self.run_async(do_disconnect())