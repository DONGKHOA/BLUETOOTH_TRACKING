import struct

content_data  = 429334E8

# Unpack as float
float_value = struct.unpack('!f', content_data[:4])[0]

print(float_value)