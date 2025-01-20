import socket
import struct

# Server configuration
HOST = '192.168.1.109'
PORT = 8899

def parse_28_byte_content(content_data):
    """Parse the 28-byte content data into a dictionary."""
    fields = struct.unpack('>ffffffI', content_data)
    return {
        "Breath BPM": fields[0],
        "Breath Curve": fields[1],
        "Heart Rate BPM": fields[2],
        "Heart Rate Curve": fields[3],
        "Target Distance": fields[4],
        "Signal Strength": fields[5],
        "Valid Bit ID": fields[6],
    }

def parse_packet(data):
    """Parse the incoming packet and return the function and content data."""
    if len(data) < 14:
        raise ValueError("Data length is insufficient to parse the header.")

    # Parse the header
    proto, ver, packet_type, cmd, request_id, timeout, content_len = struct.unpack('!BBBBIHI', data[:14])

    if len(data) < 14 + content_len:
        raise ValueError(f"Data length is insufficient for content. Expected {14 + content_len} bytes, got {len(data)} bytes.")

    # Extract function and content
    function = struct.unpack('!H', data[14:16])[0]
    content_data = data[16:14 + content_len]
    return function, content_data

def handle_client(client_socket, client_address):
    """Handle communication with a connected client."""
    print(f"Connected to client: {client_address}")
    try:
        while True:
            # Receive data from the client
            data = client_socket.recv(1024)
            if not data:
                print(f"Client {client_address} disconnected.")
                break

            # Log the received data in hexadecimal format
            hex_data = "".join(f"{byte:02x}" for byte in data)
            print(f"Received data (hex): {hex_data}")

            try:
                if data[15] == 1:
                    response = [0x13, 0x01, 0x01, 0x01, 0x9b, 0, 0, 0, 0x10, 0x27, 0, 0, 0, 6, 0, 1, 0, 0, 0, 0]
                    client_socket.sendall(bytes(response))

                    ack = client_socket.recv(1024)
                    if ack.strip() == b"ACK":
                        print(f"[{client_address[0]}] Received ACK from client")

                if data[14] == 0x03 or data[15] == 0xe8:
                    function, content_data = parse_packet(data)
                    parsed_content = parse_28_byte_content(content_data)
                    print("Parsed 28-byte content:")
                    for key, value in parsed_content.items():
                        print(f"{key}: {value}")

                    client_socket.sendall(b"ACK")
                else:
                    client_socket.sendall(b"ACK")

            except Exception as e:
                print(f"Error parsing data from client {client_address}: {e}")
                client_socket.sendall(b"ERROR")

    except Exception as e:
        print(f"Error handling client {client_address}: {e}")
    finally:
        client_socket.close()

# Main server loop
def start_server():
    """Start the TCP server."""
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_socket.bind((HOST, PORT))
        server_socket.listen(1)
        print(f"Server is running on {HOST}:{PORT}")

        while True:
            print("Waiting for a connection from a client...")
            client_socket, client_address = server_socket.accept()
            handle_client(client_socket, client_address)

if __name__ == "__main__":
    start_server()
