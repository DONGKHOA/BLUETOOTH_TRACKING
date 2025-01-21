import socket
import struct
import numpy as np

HOST = '192.168.1.101'
PORT = 8899

# --- 1) Function to parse 28 bytes (6 floats + 1 uint) ---
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


# --- 2) Function to parse 36 bytes (8 floats + 1 uint = 9 fields) ---
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


# --- 3) Function to parse the application-level packet (14-byte header + 2-byte function + content) ---
def parse_packet(data):
    """
    Header (14 bytes, big-endian) consists of:
      - proto (1 byte)
      - ver (1 byte)
      - packet_type (1 byte)
      - cmd (1 byte)
      - request_id (4 bytes, unsigned int)
      - timeout (2 bytes, unsigned short)
      - content_len (4 bytes, unsigned int)

    => struct.unpack('!BBBBIHI') => 14 bytes.

    Then, 2 bytes for 'function' (unsigned short).
    => a total of 16 bytes minimum before extracting content_data.

    We assume content_len already includes the 2 bytes of function.
    => content_data = data[16 : 14 + content_len]
    """
    if len(data) < 16:
        raise ValueError("Data is too short (less than 14-byte header + 2-byte function).")

    # Unpack the 14-byte header
    proto, ver, ptype, cmd, request_id, timeout, content_len = struct.unpack('!BBBBIHI', data[:14])

    # Extract the function (2 bytes)
    function = struct.unpack('!H', data[14:16])[0]

    # Check if we have enough data for the content
    if len(data) < 14 + content_len:
        raise ValueError(f"Content is incomplete. Needed {14 + content_len} bytes, got {len(data)} bytes.")

    # Extract the content
    content_data = data[16: 14 + content_len]

    return request_id, function, content_len, content_data


# --- 4) A simple TCP server ---
def main():
    count = np.uint32(0)
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind((HOST, PORT))
    server_socket.listen(1)
    print(f"Server is running on {HOST}:{PORT}")

    while True:
        print("Waiting for a connection from a client...")
        client_socket, client_address = server_socket.accept()
        print(f"Connected to client: {client_address}")

        try:
            while True:
                data = client_socket.recv(1024)
                if not data:
                    print("Client disconnected.")
                    break

                # Print incoming data in hex form
                print(f"Received data (hex): {data.hex()}")

                # Parse the packet
                try:
                    request_id, function, content_len, content_data = parse_packet(data)
                    print(
                        f"[Parsed] function=0x{function:04x}, content_len={content_len}, content_data={content_data.hex()}")
                except ValueError as e:
                    print(f"[!] Parse error: {e}")
                    continue

                # 5) Example handling for function=1
                if function == 1:
                    count = count + 1
                    # Construct a response:
                    #   - 4 bytes of request_id (big-endian)
                    #   - etc. depending on your logic
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
                    print(f"[Server] Sent response for function=1")

                # 6) Example handling for function=0x03e8 or 0x03??
                if function == 0x03e8 or (function & 0xFF00) == 0x0300:
                    if len(content_data) == 28:
                        parsed = parse_28_byte_content(content_data)
                        print("[Parsed 28-byte content]")
                        for k, v in parsed.items():
                            print(f"   {k}: {v}")
                    elif len(content_data) == 36:
                        parsed = parse_36_byte_content(content_data)
                        print("[Parsed 36-byte content]")
                        for k, v in parsed.items():
                            print(f"   {k}: {v}")
                    else:
                        print(f"[!] content_data length = {len(content_data)}, expected 28 or 36.")

        except Exception as e:
            print(f"[!] Error handling client {client_address}: {e}")
        finally:
            client_socket.close()


if __name__ == '__main__':
    main()
