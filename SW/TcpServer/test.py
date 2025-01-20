import struct

def parse_28_byte_content(content_data):

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

# Dữ liệu hex
hex_data = "41610000bf283218429000003cda28603f4ccccd4260e70500000000"

# Chuyển đổi dữ liệu hex thành bytes và giải mã
content_data = bytes.fromhex(hex_data)
result = parse_28_byte_content(content_data)

# Hiển thị kết quả
print(result)