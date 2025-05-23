import asyncio
from config_stored import configs, save_configs

class MQTTConfigManager:
    def save_config(self, mac, mqtt_server=None, topic=None, room=None):
        if mac not in configs:
            configs[mac] = {}

        if mqtt_server is not None:
            configs[mac]["mqtt_server"] = mqtt_server

        if topic is not None:
            configs[mac]["topic"] = topic

        if room is not None:
            configs[mac]["room"] = room

        save_configs()

    def get_config(self, mac):
        data = configs.get(mac, {})
        return {
            "mqtt_server": data.get("mqtt_server", ""),
            "topic": data.get("topic", ""),
            "room": data.get("room", "")
        }


    def clear_config(self, mac):
        if mac in configs:
            del configs[mac]
            save_configs()

    def send_mqtt_server(self, client, write_char, ble_loop, mqtt_server, mac):
        
        async def do_send():
            if client and write_char:
                
                try:
                    payload = f"MQTTSERVER:{mqtt_server}"
                    
                    # Check the length of the payload
                    if len(payload) < 20:
                        payload += "\n" * (20 - len(payload))
                        
                    await client.write_gatt_char(write_char, payload.encode())
                    
                    topic = self.get_config(mac).get("topic", "")
                    self.save_config(mac, mqtt_server=mqtt_server, topic=topic)
                    return {"sent": True}
                
                except Exception as e:
                    return {"sent": False, "error": str(e)}
            
            return {"sent": False, "error": "Not connected or writable characteristic missing"}

        return asyncio.run_coroutine_threadsafe(do_send(), ble_loop).result()

    def send_mqtt_topic(self, client, write_char, ble_loop, topic, mac):
        
        async def do_send():
            if client and write_char:
                
                try:
                    payload = f"MQTTTOPIC:{topic}"
                    
                    # Check the length of the payload
                    if len(payload) < 20:
                        payload += "\n" * (20 - len(payload))
                        
                    await client.write_gatt_char(write_char, payload.encode())
                    
                    server = self.get_config(mac).get("mqtt_server", "")
                    self.save_config(mac, mqtt_server=server, topic=topic)
                    
                    return {"sent": True}
                
                except Exception as e:
                    return {"sent": False, "error": str(e)}
            
            return {"sent": False, "error": "Not connected or writable characteristic missing"}

        return asyncio.run_coroutine_threadsafe(do_send(), ble_loop).result()
    
    def send_room(self, client, write_char, ble_loop, room, mac):
        
        async def do_send():
            if client and write_char:
                
                try:
                    payload = f"ROOM:{room}\n"
                    
                    # Check the length of the payload
                    if len(payload) < 20:
                        payload += "\n" * (20 - len(payload))
                        
                    await client.write_gatt_char(write_char, payload.encode())
                    
                    server = self.get_config(mac).get("room", "")
                    self.save_config(mac, room=room)
                    
                    return {"sent": True}
                
                except Exception as e:
                    return {"sent": False, "error": str(e)}
            
            return {"sent": False, "error": "Not connected or writable characteristic missing"}

        return asyncio.run_coroutine_threadsafe(do_send(), ble_loop).result()
                    

    