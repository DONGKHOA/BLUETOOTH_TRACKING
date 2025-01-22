import socket
import struct
import numpy as np
import threading

HOST = '192.168.1.101'
PORT = 8899

lock = threading.Lock()

# Global counter (uint32)
count = np.uint32(0)

# Two “hash tables” for both directions:
ip_to_id_map = {}  # Maps client_ip -> client_id_hex
id_to_ip_map = {}  # Maps client_id_hex -> client_ip

client_handlers = {}

class ClientHandler(threading.Thread):
    def __init__(self, client_socket, client_address):
        super().__init__(daemon=True)
        self.client_socket = client_socket
        self.client_address = client_address
        self.stop_event = threading.Event()

    def run(self):
        global count
        client_ip = self.client_address[0]
        print(f"[Thread] Handling client: {self.client_address}")

        try:
            while not self.stop_event.is_set():
                data = self.client_socket.recv(1024)
                if not data:
                    print(f"Client {self.client_address} disconnected.")
                    break

                print(f"Received data (hex): {data.hex()}")

                try:
                    request_id, function, content_len, content_data = parse_packet(data)
                    print(f"[Parsed] function=0x{function:04x}, content_len={content_len}, content_data={content_data.hex()}")
                except ValueError as e:
                    print(f"[!] Parse error: {e}")
                    continue

                if function == 1:
                    if len(content_data) < 13:
                        print("[!] content_data too short for 13-byte client ID.")
                        new_id_hex = "NoID"
                    else:
                        new_id_hex = content_data[-13:].hex()

                    with lock:
                        if count == 0 or client_ip not in ip_to_id_map:
                            count += 1
                            ip_to_id_map[client_ip] = new_id_hex
                            id_to_ip_map[new_id_hex] = client_ip
                            print(f"[Server] (1) Registered new: IP={client_ip}, ID={new_id_hex}. count={count}")
                        else:
                            old_id = ip_to_id_map[client_ip]
                            if old_id != new_id_hex:
                                ip_to_id_map[client_ip] = new_id_hex
                                if old_id in id_to_ip_map:
                                    del id_to_ip_map[old_id]
                                id_to_ip_map[new_id_hex] = client_ip
                                print(f"[Server] (2) Updated IP={client_ip} from ID={old_id} to new ID={new_id_hex}")
                            else:
                                print(f"[Server] (2) IP={client_ip} with same ID={old_id}, no update needed.")

                        if new_id_hex in id_to_ip_map:
                            old_ip = id_to_ip_map[new_id_hex]
                            if old_ip != client_ip:
                                id_to_ip_map[new_id_hex] = client_ip
                                stored_id_for_old_ip = ip_to_id_map.get(old_ip)
                                if stored_id_for_old_ip == new_id_hex:
                                    del ip_to_id_map[old_ip]
                                print(f"[Server] (3) Updated ID={new_id_hex} from IP={old_ip} to new IP={client_ip}")
                            else:
                                print(f"[Server] (3) ID={new_id_hex} already mapped to IP={client_ip}, no update needed.")
                        else:
                            id_to_ip_map[new_id_hex] = client_ip
                            print(f"[Server] (3) ID={new_id_hex} was not in map, set IP={client_ip}.")

                        current_count = count

                    data_resp = [
                        0x13, 0x01, 0x00, 0x02,
                        (request_id >> 24) & 0xFF,
                        (request_id >> 16) & 0xFF,
                        (request_id >> 8) & 0xFF,
                        (request_id >> 0) & 0xFF,
                        0, 0, 0, 0, 0, 6, 0, 1,
                        (current_count >> 24) & 0xFF,
                        (current_count >> 16) & 0xFF,
                        (current_count >> 8) & 0xFF,
                        (current_count >> 0) & 0xFF
                    ]
                    self.client_socket.sendall(bytes(data_resp))
                    print(f"[Server] Sent response for function=1 with count={current_count}")

                if function == 0x03e8 or (function & 0xFF00) == 0x0300:
                    with lock:
                        client_id = ip_to_id_map.get(client_ip, "UnknownID")

                    if len(content_data) == 28:
                        parsed = parse_28_byte_content(content_data)
                        print(f"[Parsed 28-byte content from IP={client_ip}, ID={client_id}]")
                        for k, v in parsed.items():
                            print(f"   {k}: {v}")
                    elif len(content_data) == 36:
                        parsed = parse_36_byte_content(content_data)
                        print(f"[Parsed 36-byte content from IP={client_ip}, ID={client_id}]")
                        for k, v in parsed.items():
                            print(f"   {k}: {v}")
                    else:
                        print(f"[!] content_data length={len(content_data)}, expected 28 or 36.")

        except Exception as e:
            print(f"[!] Error handling client {self.client_address}: {e}")
        finally:
            self.client_socket.close()
            print(f"[Thread] Closed connection to {self.client_address}")

    def stop(self):
        self.stop_event.set()
        self.join()


def parse_28_byte_content(data_28):
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
    if len(data) < 16:
        raise ValueError("Data is too short (< 14-byte header + 2-byte function).")

    proto, ver, ptype, cmd, request_id, timeout, content_len = struct.unpack('!BBBBIHI', data[:14])
    function = struct.unpack('!H', data[14:16])[0]

    if len(data) < 14 + content_len:
        raise ValueError(
            f"Content incomplete. Needed {14 + content_len} bytes, got {len(data)} bytes."
        )

    content_data = data[16 : 14 + content_len]
    return request_id, function, content_len, content_data


def main():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind((HOST, PORT))
    server_socket.listen(5)
    print(f"Server is running on {HOST}:{PORT}")

    while True:
        print("Number of active threads: ", threading.active_count())
        print("Waiting for a connection from a client...")
        client_socket, client_address = server_socket.accept()
        print(f"[Server] Accepted connection from {client_address}")

        if client_address[0] in client_handlers:
            print(f"[Server] Cleaning up existing handler for {client_address}.")
            client_handlers[client_address[0]].stop()
            del client_handlers[client_address[0]]

        print("Create new thread")
        handler = ClientHandler(client_socket, client_address)
        client_handlers[client_address[0]] = handler
        handler.start()

if __name__ == '__main__':
    main()
