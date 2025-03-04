import socket
import struct
import numpy as np
import threading
import json
import paho.mqtt.client as mqtt

HOST = '192.168.137.91'
PORT = 8899

# Broker address
MQTT_BROKER = "broker.emqx.io"  # Example using a public broker
MQTT_PORT = 1883
MQTT_TOPIC_PUBLISH = "IOT/dataTopic/AEROSENSE"

# Global counter (uint32)
count = np.uint32(0)

# Maps client_ip -> client_id_hex
ip_to_id_map = {}

# Maps client_ip -> current socket (to close the old socket when the same IP reconnects)
ip_to_socket_map = {}

# Create a client
client = mqtt.Client(client_id="AEROSENSEClient")


# Callback function called when the client successfully connects to the broker
def mqtt_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Successfully connected to the MQTT broker using MQTT v3.1.1!")
    else:
        print(f"Connection failed. Return code: {rc}")


# Callback function called when the client disconnects from the broker
def mqtt_disconnect(client, userdata, rc):
    print("Disconnected from the MQTT broker.")


# Callback function called when there is a new message on subscribed topics
def mqtt_message(client, userdata, msg):
    print(f"Received message from topic {msg.topic}: {msg.payload.decode('utf-8')}")


def parse_28_byte_content(data_28):
    """
    data_28: 28 bytes => 6 floats (24 bytes) + 1 uint (4 bytes).
    Big-endian format: '>ffffffI'
    """
    fields = struct.unpack('>ffffffI', data_28)
    return {
        "Breath BPM": fields[0],
        "Breath Curve": fields[1],
        "Heart Rate BPM": fields[2],
        "Heart Rate Curve": fields[3],
        "Target Distance": fields[4],
        "Signal Strength": fields[5],
        "Valid Bit ID": fields[6],
    }


def parse_36_byte_content(data_36):
    """
    data_36: 36 bytes => 6 floats + 1 uint + 2 floats = 9 fields.
    Big-endian format: '>ffffffIff'
    """
    fields = struct.unpack('>ffffffIff', data_36)
    return {
        "Breath BPM": fields[0],
        "Breath Curve": fields[1],
        "Heart Rate BPM": fields[2],
        "Heart Rate Curve": fields[3],
        "Target Distance": fields[4],
        "Signal Strength": fields[5],
        "Valid Bit ID": fields[6],
        "Body Move Energy": fields[7],
        "Body Move Range": fields[8],
    }

def parse_packet(data):
    """
    Header (14 bytes, big-endian):
      - proto (1 byte)
      - ver   (1 byte)
      - ptype (1 byte)
      - cmd   (1 byte)
      - request_id (4 bytes, unsigned int)
      - timeout    (2 bytes, unsigned short)
      - content_len(4 bytes, unsigned int)

    => struct.unpack('!BBBBIHI') => 14 bytes

    Then 2 bytes for 'function' => a total of 16 bytes (minimum) before content_data.
    => content_data = data[16 : 14 + content_len]
    """
    if len(data) < 16:
        return (0, 0, 0, b"")  # In case the data is too short, return empty

    proto, ver, ptype, cmd, request_id, timeout, content_len = struct.unpack('!BBBBIHI', data[:14])
    function = struct.unpack('!H', data[14:16])[0]

    content_data = data[16: 14 + content_len]
    return request_id, function, content_len, content_data

def handle_client(client_socket, client_address):
    global count
    global client
    client_ip = client_address[0]

    print(f"[Thread] Handling client: {client_address}")

    try:
        while True:
            data = client_socket.recv(1024)
            if not data:
                # If the client has closed the connection (received a FIN packet) => empty string => exit
                print(f"[Thread] Client {client_ip} disconnected.")
                break

            print(f"Received data (hex): {data.hex()}")
            request_id, function, content_len, content_data = parse_packet(data)
            print(f"[Parsed] function=0x{function:04x}, content_len={content_len}, content_data={content_data.hex()}")

            # ---------------- Handle function=0x0001 (register ID, send count) ----------------
            if function == 1:
                # Suppose the last 13 bytes are the ID
                if len(content_data) >= 13:
                    new_id_hex = content_data[-13:].hex()
                else:
                    new_id_hex = "TooShort"

                ip_to_id_map[client_ip] = new_id_hex
                print(f"[Server] (1) Registered new: IP={client_ip}, ID={new_id_hex}. count={count}")

                # Send response containing the 'count' variable
                data_resp = [
                    0x13, 0x01, 0x00, 0x02,
                    (request_id >> 24) & 0xFF,
                    (request_id >> 16) & 0xFF,
                    (request_id >> 8) & 0xFF,
                    (request_id >> 0) & 0xFF,
                    0, 0, 0, 0, 0, 6, 0, 1,
                    (count >> 24) & 0xFF,
                    (count >> 16) & 0xFF,
                    (count >> 8) & 0xFF,
                    (count >> 0) & 0xFF
                ]
                client_socket.sendall(bytes(data_resp))
                print(f"[Server] Sent response for function=1 with count={count}")

            # ---------------- Handle function=0x03e8 (28 or 36 bytes data) ----------------
            elif function == 0x03e8:
                # Get ID from map, if not found then use "UnknownID"
                client_id = ip_to_id_map.get(client_ip, "UnknownID")

                if len(content_data) == 28:
                    parsed = parse_28_byte_content(content_data)
                    print(f"[Parsed 28-byte content from IP={client_ip}, ID={client_id}]")
                    for k, v in parsed.items():
                        print(f"   {k}: {v}")

                    index_data = {"ID": client_id}
                    json_data = json.dumps([index_data, parsed], indent=4)
                    print(json_data)

                    client.publish(MQTT_TOPIC_PUBLISH, json_data)

                elif len(content_data) == 36:
                    parsed = parse_36_byte_content(content_data)
                    print(f"[Parsed 36-byte content from IP={client_ip}, ID={client_id}]")
                    for k, v in parsed.items():
                        print(f"   {k}: {v}")

                    index_data = {"ID": client_id}
                    json_data = json.dumps([index_data, parsed], indent=4)
                    print(json_data)

                    client.publish(MQTT_TOPIC_PUBLISH, json_data)

                else:
                    print(f"[!] content_data length={len(content_data)}, expected 28 or 36.")

    except Exception as e:
        print(f"[Thread] Exception for {client_ip}: {e}")
    finally:
        # When exiting the loop (client disconnected or error):
        client_socket.close()
        # If you want to remove the IP from the map to treat the next connection as "new":
        if client_ip in ip_to_id_map:
            del ip_to_id_map[client_ip]
            print(f"[Thread] Removed IP={client_ip} from ip_to_id_map.")
        # If the socket map still points to this socket, remove it as well
        if ip_to_socket_map.get(client_ip) == client_socket:
            del ip_to_socket_map[client_ip]
        print(f"[Thread] Ended for {client_ip}.")

def main():
    global count
    global client

    # Assign the callback functions
    client.on_connect = mqtt_connect
    client.on_message = mqtt_message
    client.on_disconnect = mqtt_disconnect

    # Attempt to connect to the MQTT broker
    print(f"Connecting to broker {MQTT_BROKER}:{MQTT_PORT} using MQTT v3.1.1 ...")
    client.connect(MQTT_BROKER, MQTT_PORT, 60)

    # Start the client loop (non-blocking)
    client.loop_start()

    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind((HOST, PORT))
    server_socket.listen(5)
    print(f"Server is running on {HOST}:{PORT}")

    while True:
        print("Number of active threads:", threading.active_count())
        print("Waiting for a connection from a client...")
        client_socket, client_address = server_socket.accept()
        client_ip = client_address[0]
        print(f"[Server] Accepted connection from {client_address}")

        # If there's already an existing connection from this IP, close it
        # (to enforce "one IP - one connection")
        if client_ip in ip_to_socket_map:
            old_socket = ip_to_socket_map[client_ip]
            try:
                old_socket.shutdown(socket.SHUT_RDWR)
            except:
                pass
            old_socket.close()
            print(f"[Server] Closed old socket for IP={client_ip}")

        # Record the new socket for this IP
        ip_to_socket_map[client_ip] = client_socket

        # Increment count if this IP hasn't appeared in ip_to_id_map
        if client_ip not in ip_to_id_map:
            count += 1

        t = threading.Thread(target=handle_client, args=(client_socket, client_address), daemon=True)
        t.start()

if __name__ == '__main__':
    main()
