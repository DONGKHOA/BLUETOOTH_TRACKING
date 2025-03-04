import asyncio
import struct
import time
import os
import random

SERVER_HOST = "192.168.0.123"
SERVER_PORT = 8899

count = 0x00000001

DEVICE_ID_BYTES = bytes.fromhex("2cb1b08ea72c54eb886b37dd01")

async def test_client():
    global count
    reader, writer = await asyncio.open_connection(SERVER_HOST, SERVER_PORT)
    print(f"🔗 Kết nối tới {SERVER_HOST}:{SERVER_PORT}")

    try:
        # 🛠 Gửi gói đăng ký
        reg_packet = create_registration_packet()
        print(f"📤 Gửi gói đăng ký ({len(reg_packet)} bytes): {reg_packet.hex()}")
        writer.write(reg_packet)
        await writer.drain()

        count = count + 1

        # 📥 Nhận phản hồi từ server
        response = await reader.read(20)
        print(f"📥 Phản hồi đăng ký ({len(response)} bytes): {response.hex()}")

        while (True):
            # 🛠 Gửi gói sensor data
            sensor_packet = create_sensor_packet()
            print(f"📤 Gửi gói sensor ({len(sensor_packet)} bytes): {sensor_packet.hex()}")
            writer.write(sensor_packet)
            await writer.drain()

            count = count + 1

            time.sleep(random.randint(1, 3))  
            

    except Exception as e:
        print(f"⚠️ Lỗi: {e}")

    finally:
        print("🔌 Đóng kết nối")
        writer.close()
        await writer.wait_closed()


def create_registration_packet():
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

    global count

    proto = 0x13  # 1 byte
    ver = 0x01  # 1 byte
    ptype = 0x01  # 1 byte
    cmd = 0x01  # 1 byte
    request_id = count  # 4 bytes
    timeout = 0x000a  # 2 bytes (H)
    content_len = 0x00000014  # 4 bytes (I)
    function_code = 0x0001  # 2 bytes (H)
    radar_type = 0x01  # 1 byte (B)
    hw_version = 0x0200001d  # 4 bytes (I)

    # Ensure DEVICE_ID_BYTES is exactly 13 bytes
    assert len(DEVICE_ID_BYTES) == 13, "DEVICE_ID_BYTES must be 13 bytes"

    # Construct the packet in big-endian format
    packet = struct.pack(
        ">BBBBIHIHBI13s",
        proto, ver, ptype, cmd,
        request_id, timeout, content_len,
        function_code, radar_type,
        hw_version, DEVICE_ID_BYTES
    )
    return packet


def create_sensor_packet():
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

    global count

    proto = 0x13  # 1 byte
    ver = 0x01  # 1 byte
    ptype = 0x02  # 1 byte
    cmd = 0x01  # 1 byte
    request_id = count  # 4 bytes
    timeout = 0x000a  # 2 bytes (H)
    content_len = 0x0000001e  # 4 bytes (I)
    function_code = 0x03e8  # 2 bytes (H)
    data = bytes.fromhex("95374820519683740256830194756283019475638291047563829104")  # 28 bytes

    # Construct the packet in big-endian format
    packet = struct.pack(
        ">BBBBIHIH28s",
        proto, ver, ptype, cmd,
        request_id, timeout, content_len,
        function_code, data
    )
    return packet


if __name__ == "__main__":
    asyncio.run(test_client())