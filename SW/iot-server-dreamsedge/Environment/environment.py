# Authentication Token
AUTH_TOKEN = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJleHAiOjE3NDE0NTYwNTksInN1YiI6IjMifQ.8vS3OCFZC2DONMHNdoyQFOIy1skyapaNDCQxcETzMQI"

#IP for DreamsEdge API
IP_API = "54.252.196.164"

# TCP server information
TCP_SERVER_NAME = "another-tcp-server"

# Deivice name
DEVICE_NAME = "AerosenseDevice"
FACILITY_ID = 1

# Login Information
LOGIN_USERNAME = "admin"
LOGIN_PASSWORD = "123"

# Information USER
USERNAME = "iotteam@devcburst.com"
PASSWORD = "dreamsiot@2025"

AWS_IOT_ENDPOINT = "a1lvnt250s0jlf-ats.iot.ap-southeast-1.amazonaws.com"

# Path to root certificate
root_ca_path = "Environment/upload/root-CA.crt"
# Path to private key
private_key_path = "Environment/upload/tcp-server.private.key"
# Path to tới certificate  
cert_path = "Environment/upload/tcp-server.cert.pem"  

MQTT_PORT = 8883  # Port MQTT SSL

# TCP_SERVER
TCP_SERVER_HOST = '0.0.0.0'
TCP_SERVER_PORT = 8899

# Facility list
FACILITY_LIST = {'data': [{'name': 'Bene St.Paul', 'address': 'St.Paul', 'timezone': 'Australia/Sydney', 'tcp_server_name': 'another-tcp-server', 'id': 1, 'created_at': '2025-03-05T12:58:28.316924', 'updated_at': '2025-03-05T12:58:28.316940'}, {'name': 'Rob_Test', 'address': '', 'timezone': 'Australia/Sydney', 'tcp_server_name': 'another-tcp-server', 'id': 6, 'created_at': '2025-03-05T11:31:50.493161', 'updated_at': '2025-03-06T04:04:02.621634'}], 'count': 2}

# Check connection with AWS
START_CHECK_AWS = True
CONNECTION_AWS = True

# Server Status
TCP_SERVER_STATUS = None
AWS_SERVER_STATUS = None

START_SERVER = True
STOP_SERVER = False
RECONFIG_SERVER = False


# Check the change
INFO_UPDATED = False
FACILITY_ID_UPDATED = False
AWS_IOT_ENDPOINT_UPDATED = False
