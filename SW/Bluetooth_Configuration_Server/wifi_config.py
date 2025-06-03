import subprocess
import asyncio
from config_stored import configs, save_configs

import re

class WiFiConfigManager:
    def save_config(self, mac, ssid=None, password=None):
        if mac not in configs:
            configs[mac] = {}

        if ssid is not None:
            configs[mac]["ssid"] = ssid

        if password is not None:
            configs[mac]["password"] = password

        save_configs()

    def get_config(self, mac):
        data = configs.get(mac, {})
        return {
            "ssid": data.get("ssid", ""),
            "password": data.get("password", "")
        }

    def clear_config(self, mac):
        if mac in configs:
            del configs[mac]
            save_configs()

    def scan_wifi(self):
        try:
            result = subprocess.check_output(
                ["nmcli", "-t", "-f", "SSID", "dev", "wifi"],
                text=True
            )
            
            # result = subprocess.check_output(
            #     ["netsh", "wlan", "show", "networks", "mode=Bssid"],
            #     text=True,
            #     encoding="utf-8"
            # )

            raw_ssids = result.strip().split("\n")
            # raw_ssids = re.findall(r"SSID \d+ : (.*)", result)
            unique_ssids = sorted(set(ssid.strip() for ssid in raw_ssids if ssid.strip()))
            
            return unique_ssids
        except Exception as e:
            return {"error": str(e)}

    def send_wifi_credentials(self, client, write_char, ble_loop, ssid, password, mac):
        
        async def do_send_wifi():
            if client and write_char:
                
                try:
                    payload = f"WIFI:{ssid}|{password}\n"

                    # Check the length of the payload
                    if len(payload) < 20:
                        payload += "\n" * (20 - len(payload))
                    
                    await client.write_gatt_char(write_char, payload.encode())
                    
                    self.save_config(mac, ssid=ssid, password=password)
                    return {"sent": True}
                
                except Exception as e:
                    return {"sent": False, "error": str(e)}
            
            return {"sent": False, "error": "Not connected or writable characteristic missing"}

        return asyncio.run_coroutine_threadsafe(do_send_wifi(), ble_loop).result()