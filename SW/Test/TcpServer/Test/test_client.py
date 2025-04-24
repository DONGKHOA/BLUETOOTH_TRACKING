import asyncio
import struct
import secrets  # for random bytes

SERVER_HOST = '192.168.1.12'
SERVER_PORT = 8899

def generate_device_id_bytes(client_id: int) -> bytes:
    """
    Returns a unique 13-byte device ID.
    This example just uses 4 random bytes + 4 bytes client_id + 5 more random bytes.
    You can customize this however you prefer.
    """
    # 4 random bytes
    rnd_prefix = secrets.token_bytes(4)
    # 4 bytes representing client_id (big-endian)
    cid_bytes = struct.pack('>I', client_id)
    # remaining 5 random bytes
    rnd_suffix = secrets.token_bytes(5)
    return rnd_prefix + cid_bytes + rnd_suffix  # total = 4 + 4 + 5 = 13 bytes

async def test_client(client_id, local_ip):
    """
    Each client uses a unique device_id_bytes.
    """
    # Generate a unique device ID for this client
    device_id = generate_device_id_bytes(client_id)

    count = 0
    print(f"[Client {client_id}] 🔗 Connecting to {SERVER_HOST}:{SERVER_PORT}")

    try:
        reader, writer = await asyncio.open_connection(SERVER_HOST, SERVER_PORT, local_addr=(local_ip, 0))

        # 🛠 Send registration packet
        reg_packet = create_registration_packet(count, device_id)
        print(f"[Client {client_id}] 📤 Send registration ({len(reg_packet)} bytes): {reg_packet.hex()}")
        writer.write(reg_packet)
        await writer.drain()
        count += 1

        # 📥 Receive server response
        response = await reader.read(20)
        print(f"[Client {client_id}] 📥 Registration response ({len(response)} bytes): {response.hex()}")

        # Now keep sending sensor packets
        while True:
            sensor_packet = create_sensor_packet(count)
            print(f"[Client {client_id}] 📤 Send sensor ({len(sensor_packet)} bytes): {sensor_packet.hex()}")
            writer.write(sensor_packet)
            await writer.drain()
            count += 1

            await asyncio.sleep(1)

    except Exception as e:
        print(f"[Client {client_id}] ⚠️ Error: {e}")

    finally:
        print(f"[Client {client_id}] 🔌 Closing connection")
        writer.close()
        await writer.wait_closed()

def create_registration_packet(count, device_id_bytes):
    """
    Header (14 bytes, big-endian):
      - proto (1 byte)
      - ver   (1 byte)
      - ptype (1 byte)
      - cmd   (1 byte)
      - request_id (4 bytes, unsigned int)
      - timeout    (2 bytes, unsigned short)
      - content_len(4 bytes, unsigned int)

    Then 2 bytes for 'function' => a total of 16 bytes (minimum) before content_data.
    => content_data = data[16 : 14 + content_len]
    """
    proto = 0x13         # 1 byte
    ver = 0x01           # 1 byte
    ptype = 0x01         # 1 byte
    cmd = 0x01           # 1 byte
    request_id = count   # 4 bytes
    timeout = 0x000A     # 2 bytes (H)
    content_len = 0x00000014  # 4 bytes (I)
    function_code = 0x0001    # 2 bytes (H)
    radar_type = 0x01         # 1 byte (B)
    hw_version = 0x0200001D   # 4 bytes (I)

    # Now pass in the dynamically generated 13-byte device_id
    packet = struct.pack(
        ">BBBBIHIHBI13s",
        proto, ver, ptype, cmd,
        request_id, timeout, content_len,
        function_code, radar_type,
        hw_version, device_id_bytes
    )
    return packet

def create_sensor_packet(count):
    """
    Header (14 bytes, big-endian):
      - proto (1 byte)
      - ver   (1 byte)
      - ptype (1 byte)
      - cmd   (1 byte)
      - request_id (4 bytes, unsigned int)
      - timeout    (2 bytes, unsigned short)
      - content_len(4 bytes, unsigned int)

    Then 2 bytes for 'function' => a total of 16 bytes (minimum) before content_data.
    => content_data = data[16 : 14 + content_len]
    """
    proto = 0x13       # 1 byte
    ver = 0x01         # 1 byte
    ptype = 0x02       # 1 byte
    cmd = 0x01         # 1 byte
    request_id = count # 4 bytes
    timeout = 0x000A   # 2 bytes (H)
    content_len = 0x0000001E   # 4 bytes (I)
    function_code = 0x03E8     # 2 bytes (H)

    # 28-byte sensor data
    data = bytes.fromhex("4143d555bf1e24bc4273c0003eb5ec0c3fa66666422f29880000000242f3e27a3fa66666")

    packet = struct.pack(
        ">BBBBIHIH28s",
        proto, ver, ptype, cmd,
        request_id, timeout, content_len,
        function_code, data
    )
    return packet

async def main():
    tasks = []
    # Start 100 concurrent clients
    for client_id in range(100):
        local_ip = f"192.168.1.{20+client_id}"
        task = asyncio.create_task(test_client(client_id, local_ip))
        tasks.append(task)

    # This will block forever unless there's an error
    await asyncio.gather(*tasks)

if __name__ == "__main__":
    asyncio.run(main())
