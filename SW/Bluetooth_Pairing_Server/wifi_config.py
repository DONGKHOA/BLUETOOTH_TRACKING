import subprocess

class WiFiConfigManager:
    def __init__(self):
        self.configs = {}

    def save_config(self, mac, ssid, password):
        self.configs[mac] = {
            "ssid": ssid,
            "password": password
        }

    def get_config(self, mac):
        return self.configs.get(mac, {"ssid": "", "password": ""})

    def clear_config(self, mac):
        if mac in self.configs:
            del self.configs[mac]

    def scan_wifi(self):
        try:
            result = subprocess.check_output(
                ["nmcli", "-t", "-f", "SSID", "dev", "wifi"],
                text=True
            )
            raw_ssids = result.strip().split("\n")
            unique_ssids = sorted(set(ssid.strip() for ssid in raw_ssids if ssid.strip()))
            return unique_ssids
        except Exception as e:
            return {"error": str(e)}

    def send_wifi_credentials(self, client, write_char, ble_loop, ssid, password, mac):
        import asyncio

        async def do_send_wifi():
            if client and write_char:
                try:
                    payload = f"WIFI:{ssid}|{password}"
                    await client.write_gatt_char(write_char, payload.encode())
                    self.save_config(mac, ssid, password)
                    return {"sent": True}
                except Exception as e:
                    return {"sent": False, "error": str(e)}
            return {"sent": False, "error": "Not connected or writable characteristic missing"}

        future = asyncio.run_coroutine_threadsafe(do_send_wifi(), ble_loop)
        return future.result()