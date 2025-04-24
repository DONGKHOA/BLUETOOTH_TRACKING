import asyncio
import app.services.control_server as control_server

if __name__ == "__main__":
    asyncio.run(control_server.main())  # Only run control_server without Flask
