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

    Then 2 bytes for 'function' => total 16 bytes min before content_data.
    We assume content_len includes those 2 function bytes.
    => content_data = data[16 : 14 + content_len]
    """
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


def handle_client(client_socket, client_address):
    global count  # We'll modify the global 'count' here
    client_ip = client_address[0]
    print(f"[Thread] Handling client: {client_address}")

    try:
        while True:
            data = client_socket.recv(1024)
            if not data:
                print(f"Client {client_address} disconnected.")
                break

            print(f"Received data (hex): {data.hex()}")

            try:
                request_id, function, content_len, content_data = parse_packet(data)
                print(f"[Parsed] function=0x{function:04x}, content_len={content_len}, content_data={content_data.hex()}")
            except ValueError as e:
                print(f"[!] Parse error: {e}")
                continue

            # ---------------- LOGIC FOR function == 1 ----------------
            if function == 1:
                # Extract last 13 bytes as the client's ID, if enough data
                if len(content_data) < 13:
                    print("[!] content_data too short for 13-byte client ID.")
                    new_id_hex = "NoID"
                else:
                    new_id_hex = content_data[-13:].hex()

                with lock:
                    # 1) If count==0 or IP not in ip_to_id_map => new client => increment count
                    if count == 0 or client_ip not in ip_to_id_map:
                        count += 1
                        ip_to_id_map[client_ip] = new_id_hex
                        id_to_ip_map[new_id_hex] = client_ip
                        print(f"[Server] (1) Registered new: IP={client_ip}, ID={new_id_hex}. count={count}")

                    else:
                        # 2) IP is in ip_to_id_map => compare stored ID vs. new_id
                        old_id = ip_to_id_map[client_ip]
                        if old_id != new_id_hex:
                            # Overwrite old ID with new
                            ip_to_id_map[client_ip] = new_id_hex
                            # Remove the old_id -> IP from id_to_ip_map if it existed
                            if old_id in id_to_ip_map:
                                del id_to_ip_map[old_id]
                            # Add new_id -> IP
                            id_to_ip_map[new_id_hex] = client_ip
                            print(f"[Server] (2) Updated IP={client_ip} from ID={old_id} to new ID={new_id_hex}")
                        else:
                            print(f"[Server] (2) IP={client_ip} with same ID={old_id}, no update needed.")

                    # 3) Check if new_id_hex is in id_to_ip_map => compare IP
                    if new_id_hex in id_to_ip_map:
                        old_ip = id_to_ip_map[new_id_hex]
                        if old_ip != client_ip:
                            # Overwrite old_ip with client_ip
                            id_to_ip_map[new_id_hex] = client_ip
                            # Remove the old_ip from ip_to_id_map if it pointed to new_id_hex
                            stored_id_for_old_ip = ip_to_id_map.get(old_ip)
                            if stored_id_for_old_ip == new_id_hex:
                                del ip_to_id_map[old_ip]
                            print(f"[Server] (3) Updated ID={new_id_hex} from IP={old_ip} to new IP={client_ip}")
                        else:
                            print(f"[Server] (3) ID={new_id_hex} already mapped to IP={client_ip}, no update needed.")
                    else:
                        # If somehow not in id_to_ip_map, just set it:
                        id_to_ip_map[new_id_hex] = client_ip
                        print(f"[Server] (3) ID={new_id_hex} was not in map, set IP={client_ip}.")

                    # 4) current count after all updates
                    current_count = count

                # Send a response containing the 'current_count'
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
                client_socket.sendall(bytes(data_resp))
                print(f"[Server] Sent response for function=1 with count={current_count}")

            # ---------------- If function == 0x03e8 or 0x03?? => parse more data ----------------
            if function == 0x03e8 or (function & 0xFF00) == 0x0300:
                with lock:
                    # Look up the ID from IP, or "UnknownID" if not found
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
        print(f"[!] Error handling client {client_address}: {e}")
    finally:
        client_socket.close()
        print(f"[Thread] Closed connection to {client_address}")


def main():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind((HOST, PORT))
    server_socket.listen(5)  # Adjust backlog as needed
    print(f"Server is running on {HOST}:{PORT}")

    while True:
        print("Waiting for a connection from a client...")
        client_socket, client_address = server_socket.accept()
        print(f"[Server] Accepted connection from {client_address}")

        t = threading.Thread(target=handle_client, args=(client_socket, client_address), daemon=True)
        t.start()


if __name__ == '__main__':
    main()
