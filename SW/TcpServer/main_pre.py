import socket
import struct

# Server configuration
HOST = '192.168.1.101'
PORT = 8899


# 1. Hàm parse 28 byte (6 float + 1 uint)
def parse_28_byte_content(content_data):
    """
    content_data: 28 byte, gồm 6 float (mỗi float 4 byte = 24 byte) + 1 unsigned int (4 byte) = 28.
    Format unpack: '>ffffffI' (big-endian)
    """
    fields = struct.unpack('>ffffffI', content_data)
    return {
        "Breath BPM":      fields[0],
        "Breath Curve":    fields[1],
        "Heart Rate BPM":  fields[2],
        "Heart Rate Curve":fields[3],
        "Target Distance": fields[4],
        "Signal Strength": fields[5],
        "Valid Bit ID":    fields[6],
    }


# 2. Hàm parse gói tin application-level (header 14 byte + content)
def parse_packet(data):
    """
    Giả sử header 14 byte đầu (big-endian) gồm:
      - proto (1 byte)
      - ver (1 byte)
      - packet_type (1 byte)
      - cmd (1 byte)
      - request_id (4 byte, unsigned int)
      - timeout (2 byte, unsigned short)
      - content_len (4 byte, unsigned int)

    => struct '!BBBBIHI' chiếm 14 byte.

    Sau đó, 2 byte kế tiếp là 'function' (unsigned short).

    Tiếp đó, phần nội dung chính (content_data) có độ dài = content_len - 2 (nếu content_len bao gồm cả 2 byte function),
    hoặc bạn tùy chỉnh theo giao thức thực tế.
    """
    if len(data) < 14:
        raise ValueError("Data length too short to parse the 14-byte header.")

    # Giải nén 14 byte header
    proto, ver, packet_type, cmd, request_id, timeout, content_len = struct.unpack('!BBBBIHI', data[:14])

    # Kiểm tra đủ data cho content (tối thiểu 2 byte function)
    if len(data) < 14 + 2:
        raise ValueError("Data length too short to get 'function' field.")

    # Lấy 2 byte function
    function = struct.unpack('!H', data[14:16])[0]

    # Tùy giao thức, content_len có thể tính cả 2 byte function hay không.
    # Ở đây, ta giả sử content_len đã bao gồm 2 byte function, nên phần content_data = content_len - 2.
    # Do đó, ta cần check đủ (14 + content_len) byte.
    if len(data) < 14 + content_len:
        raise ValueError(
            f"Data length is insufficient for content. Expected {14 + content_len} bytes, got {len(data)} bytes."
        )

    # Lấy phần content sau function
    content_data = data[16:14 + content_len]

    return function, content_len, content_data


# 3. Tạo TCP server (chạy “bình thường”)
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server_socket.bind((HOST, PORT))
server_socket.listen(1)
print(f"Server is running on {HOST}:{PORT}")


while True:
    print("Waiting for a connection from a client...")
    client_socket, client_address = server_socket.accept()
    client_ip, client_port = client_address
    print(f"Connected to client: {client_address}")

    try:
        while True:
            data = client_socket.recv(1024)
            if not data:
                print("Client disconnected.")
                break

            # In dạng hex
            hex_data = data.hex()
            print(f"Received data (hex): {hex_data}")

            # Thử parse theo “header 14 byte” + function (2 byte) + content
            try:
                function, content_len, content_data = parse_packet(data)
                print(f"[Parsed] function=0x{function:04x}, content_len={content_len}, content_data={content_data.hex()}")
            except ValueError as e:
                print(f"[!] Parse error: {e}")
                # Nếu parse sai, có thể bỏ qua hoặc break tuỳ logic
                continue

            if function == 1:
                data1 = [0x13, 0x01, 0x01, 0x01, 0x9b, 0, 0, 0, 0x10, 0x27, 0, 0, 0, 6, 0, 1, 0, 0, 0, 0]
                client_socket.sendall(bytes(data1))
                print(f"[Server] Sent response for function=1")

            # function = 0x03e8 => 1000 decimal
            # hoặc (function & 0xff00) == 0x0300 => byte [14] = 0x03
            if function == 0x03e8 or (function & 0xff00) == 0x0300:
                if len(content_data) == 28:
                    parsed_content = parse_28_byte_content(content_data)
                    print("[Parsed 28-byte content]")
                    for key, value in parsed_content.items():
                        print(f"   {key}: {value}")
                else:
                    print(f"[!] content_data length != 28 (actual={len(content_data)})")

    except Exception as e:
        print(f"[!] Error handling client {client_address}: {e}")
    finally:
        client_socket.close()
