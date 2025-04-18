import socket

def get_network_ip():
    try:
        # Kết nối giả định để lấy IP trong mạng
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        network_ip = s.getsockname()[0]
        s.close()
        print(f"Địa chỉ IP trong mạng: {network_ip}")
    except Exception as e:
        print(f"Không thể lấy địa chỉ IP mạng: {e}")

if __name__ == "__main__":
    get_network_ip()