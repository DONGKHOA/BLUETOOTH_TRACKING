import struct

def parse_28_byte_content(data_28):
    """
    data_28: 28 bytes => 6 floats (24 bytes) + 1 uint (4 bytes).
    Big-endian format: '>ffffffI'
    """
    fields = struct.unpack('>ffffffI', data_28)
    return {
        "breath_bpm": fields[0],
        "breath_curve": fields[1],
        "heart_bpm": fields[2],
        "heart_rate_curve": fields[3],
        "target_distance": fields[4],
        "signal_strength": fields[5],
        "valid_bit_id": fields[6]
    }

def parse_36_byte_content(data_36):
    """
    data_36: 36 bytes => 6 floats (24 bytes) + 1 uint (4 bytes) + 2 floats (8 bytes) = 9 fields.
    Big-endian format: '>ffffffIff'
    """
    fields = struct.unpack('>ffffffIff', data_36)
    return {
        "breath_bpm": fields[0],
        "breath_curve": fields[1],
        "heart_bpm": fields[2],
        "heart_rate_curve": fields[3],
        "target_distance": fields[4],
        "signal_strength": fields[5],
        "valid_bit_id": fields[6],
        "body_move_energy": fields[7],
        "body_move_range": fields[8],
    }

def parse_packet(data):
    """
    Packet layout:
      - Header (14 bytes, big-endian):
          proto (1 byte)
          ver   (1 byte)
          ptype (1 byte)
          cmd   (1 byte)
          request_id (4 bytes, unsigned int)
          timeout    (2 bytes, unsigned short)
          content_len (4 bytes, unsigned int)

        => struct.unpack('!BBBBIHI') => 14 bytes

      - Then 2 bytes for 'function' => total 16 bytes minimum before content_data.

    content_data = data[16 : 14 + content_len]
    """
    if len(data) < 16:
        return (0, 0, 0, b"")  # Not enough data

    proto, ver, ptype, cmd, request_id, timeout, content_len = struct.unpack('!BBBBIHI', data[:14])
    function = struct.unpack('!H', data[14:16])[0]

    content_data = data[16 : 14 + content_len]
    return request_id, function, content_len, content_data