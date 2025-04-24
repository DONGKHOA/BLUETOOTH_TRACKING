from flask import Flask, render_template
from app.routes.api.login import login_bp
from app.routes.api.server_management import server_manage_bp
from app.routes.api.server_configuration import server_config_bp

import app.services.control_server as control_server
import asyncio
import os

import threading

frontend_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "../Frontend"))
app = Flask(__name__, template_folder=os.path.join(frontend_path, "pages"), static_folder=os.path.join(frontend_path, "static"))


# Register Blueprint for dashboard
app.register_blueprint(login_bp, url_prefix="/api")
app.register_blueprint(server_manage_bp)
app.register_blueprint(server_config_bp)

@app.route("/")
def home():
    return render_template("login.html")

def run_flask():
    app.run(host="0.0.0.0", debug=True, use_reloader=False)

if __name__ == "__main__":
    # Start Flask in a separate thread
    flask_thread = threading.Thread(target=run_flask, daemon=True)
    flask_thread.start()
    
    # Now run the async code in the main thread
    asyncio.run(control_server.main())