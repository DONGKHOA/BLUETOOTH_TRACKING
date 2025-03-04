# Authentication Token (Replace with the correct token)
AUTH_TOKEN = None

#IP for DreamsEdge API
IP_API = "54.252.196.164"

# TCP server information
# TCP_SERVER_NAME = "TCP_SERVER_Aerosense"
TCP_SERVER_NAME = "another-tcp-server"

# Deivice name
DEVICE_NAME = "AerosenseDevice"
FACILITY_ID = 1

# Information USER
USERNAME = "iotteam@devcburst.com"
PASSWORD = "dreamsiot@2025"

#AWS
# AWS_IOT_ENDPOINT = "a1xu2macchahf7-ats.iot.ap-southeast-2.amazonaws.com" # Replace with your endpoint
#
# root_ca_path = "aws/root-CA.crt"  # Path to root certificate
# private_key_path = "aws/private.pem.key"  # Path to private key
# cert_path = "aws/certificate.pem.crt"  # Path to tới certificate

AWS_IOT_ENDPOINT = "a1lvnt250s0jlf-ats.iot.ap-southeast-1.amazonaws.com"

root_ca_path = "root-CA.crt"  # Path to root certificate
private_key_path = "tcp-server.private.key"  # Path to private key
cert_path = "tcp-server.cert.pem"  # Path to tới certificate

MQTT_PORT = 8883  # Port MQTT SSL

# TCP_SERVER
TCP_SERVER_HOST = '0.0.0.0'
TCP_SERVER_PORT = 8899