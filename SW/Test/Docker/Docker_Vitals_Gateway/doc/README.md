# DESCRIPTION PROJECT

## Structure of project

- `main.py`: Initialize TCP Server, register TCP Server, register device. Handling data is transmitted from all devices.
- `http_client.py`: Data exchange between TCP Server and DreamsEdge
- `handle_data.py`: Unpack packet and data which device transmits to TCP server.
- `environment.py`: Contain environment variable.