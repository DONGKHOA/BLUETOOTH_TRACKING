import socket

# Server configuration
HOST = '192.168.1.109'
PORT = 8899

# Create a TCP socket
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
        # # Step 1: SYN and SYN-ACK
        # data = client_socket.recv(1024)
        # if data == b'SYN':
        #     print(f"[{client_address[0]}] Received SYN packet from client")
        #     client_socket.sendall(b'SYN-ACK')
        #     print(f"[{client_address[0]}] Sent SYN-ACK to client")
        #
        #     data = client_socket.recv(1024).decode('utf-8').strip()
        #     if data == "ACK":
        #         print(f"[{client_address[0]}] Received ACK from client")

        # Receive data from the client
        while True:
            data = client_socket.recv(1024)
            if not data:
                break
            # Convert raw bytes to hex string
            hex_data = "".join(f"{byte:02x}" for byte in data)

            # Print the hexadecimal representation
            print(f"Received data (hex): {hex_data}")

            if data[15] == 1:
                data1 = [0x13, 0x01, 0x01, 0x01, 0x9b, 0, 0, 0, 0x10, 0x27, 0, 0, 0, 6, 0, 1, 0, 0, 0, 0]

                data1_bytes = bytes(data1)
                client_socket.sendall(data1_bytes)
                print(f"Sent: {data1_bytes.hex()}")

                data = client_socket.recv(1024)  # Receive ACK
                if data.strip() == b"ACK":
                    print(f"[{client_address[0]}] Received ACK from client")

            else :
                client_socket.sendall("ACK".encode('utf-8'))

    except Exception as e:
        print(f"Error handling client {client_address}: {e}")
