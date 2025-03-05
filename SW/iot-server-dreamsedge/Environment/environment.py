# Authentication Token
AUTH_TOKEN = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJleHAiOjE3NDExOTAwMzQsInN1YiI6IjMifQ.dGkwEMideBV7tnagfYqfge2okXqwDux5dnkfNi6wGVs"

#IP for DreamsEdge API
IP_API = "54.252.196.164"

# TCP server information
TCP_SERVER_NAME = "another-tcp-server"

# Deivice name
DEVICE_NAME = "AerosenseDevice"
FACILITY_ID = 1

# Information USER
USERNAME = "iotteam@devcburst.com"
PASSWORD = "dreamsiot@2025"

AWS_IOT_ENDPOINT = "a1lvnt250s0jlf-ats.iot.ap-southeast-1.amazonaws.com"

root_ca_path = "Environment/upload/root-CA.crt"  # Path to root certificate
private_key_path = "Environment/upload/tcp-server.private.key"  # Path to private key
cert_path = "Environment/upload/tcp-server.cert.pem"  # Path to tới certificate

MQTT_PORT = 8883  # Port MQTT SSL

# TCP_SERVER
TCP_SERVER_HOST = '0.0.0.0'
TCP_SERVER_PORT = 8899

# Facility list
FACILITY_LIST = {'data': [{'name': 'Rob_Test', 'address': '', 'timezone': 'Australia/Sydney', 'tcp_server_name': None, 'id': 6, 'created_at': '2025-03-05T11:31:50.493161', 'updated_at': '2025-03-05T11:31:50.493178'}, {'name': 'Khoa facility', 'address': 'Test', 'timezone': 'Australia/Sydney', 'tcp_server_name': 'another-tcp-server', 'id': 5, 'created_at': '2025-03-05T12:52:24.157226', 'updated_at': '2025-03-05T12:52:24.157242'}, {'name': 'Bene St.Paul', 'address': 'St.Paul', 'timezone': 'Australia/Sydney', 'tcp_server_name': 'another-tcp-server', 'id': 1, 'created_at': '2025-03-05T12:58:28.316924', 'updated_at': '2025-03-05T12:58:28.316940'}], 'count': 3}

# Check connection with AWS
START_CHECK_AWS = True
CONNECTION_AWS = None

# Server Status
TCP_SERVER_STATUS = True
AWS_SERVER_STATUS = True

