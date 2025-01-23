import socket
import struct

HOST = '192.168.1.101'
PORT = 8899

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

    proto, ver, ptype, cmd, request_id, timeout, content_len = struct.unpack('!BBBBIHI', data[:14])
    function = struct.unpack('!H', data[14:16])[0]

    content_data = data[16 : 14 + content_len]
    return request_id, function, content_len, content_data

def main():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind((HOST, PORT))
    server_socket.listen(5)  # Adjust backlog as needed
    print(f"Server is running on {HOST}:{PORT}")
    print("Waiting for a connection from a client...")
    client_socket, client_address = server_socket.accept()
    print(f"[Server] Accepted connection from {client_address}")

    while True:
        data = client_socket.recv(1024)
        print(f"Received data (hex): {data.hex()}")
        request_id, function, content_len, content_data = parse_packet(data)
        print(f"[Parsed] function=0x{function:04x}, content_len={content_len}, content_data={content_data.hex()}")

        # ---------------- If function == 0x0001 => parse more data ----------------
        if function == 1:
            # Send a response containing the 'current_count'
            data_resp = [
                0x13, 0x01, 0x00, 0x02,
                (request_id >> 24) & 0xFF,
                (request_id >> 16) & 0xFF,
                (request_id >> 8) & 0xFF,
                (request_id >> 0) & 0xFF,
                0, 0, 0, 0, 0, 6, 0, 1,
                0, 0, 0, 0
            ]
            client_socket.sendall(bytes(data_resp))
            print(f"[Server] Sent response for function = 0x0001")

        # ---------------- If function == 0x03e8 => parse more data ----------------
        if function == 0x03e8:
            print(f"[Server] Sent response for function = 0x03e8")
            if len(content_data) == 28:
                parsed = parse_28_byte_content(content_data)
                print(f"[Parsed 28-byte content from IP={client_address[0]},")
                for k, v in parsed.items():
                    print(f"   {k}: {v}")
            elif len(content_data) == 36:
                parsed = parse_36_byte_content(content_data)
                print(f"[Parsed 36-byte content from IP={client_address[0]},")
                for k, v in parsed.items():
                    print(f"   {k}: {v}")
            else:
                print(f"[!] content_data length={len(content_data)}, expected 28 or 36.")

if __name__ == '__main__':
    main()
