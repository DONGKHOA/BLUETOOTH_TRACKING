import socket

if __name__ == "__main__":
    host = '192.168.1.145'
    port = 8899
    # Create a TCP socket
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((host, port))
    server_socket.listen(5)  # Listen for up to 5 connections
    print(f"Server is listening on {host}:{port}")

    while True:
        # Accept a connection from the client
        client_socket, client_address = server_socket.accept()
        print(f"Connection established with {client_address}")

        try:
            # Handshake process
            data = client_socket.recv(1024).decode('utf-8').strip()
            print(f"Received: {data}")

            if data == "SYN":
                # Respond with SYN-ACK
                client_socket.sendall("SYN-ACK".encode('utf-8'))
                print("Sent: SYN-ACK")

                # Receive ACK from the client
                data = client_socket.recv(1024).decode('utf-8').strip()
                if data == "ACK":
                    print("Received: ACK")
                    client_socket.sendall("Connection established!".encode('utf-8'))
                    print("Sent: Connection established!")

            # Receive data from the client
            while True:
                data = client_socket.recv(1024).decode('utf-8').strip()
                if not data:
                    break
                print(f"Received data: {data}")

                # Respond to the client
                response = f"Server received data: {data}"
                client_socket.sendall(response.encode('utf-8'))
                print(f"Sent: {response}")

        except Exception as e:
            print(f"Error: {e}")
        finally:
            # Close the connection
            client_socket.close()
            print(f"Connection closed with {client_address}")
