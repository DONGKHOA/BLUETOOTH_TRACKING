import sys
import os

# Ensure Backend directory is in Python path BEFORE importing anything
backend_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "../Backend"))
sys.path.insert(0, backend_path)

from flask import Flask, render_template
from app.routes.api.login import login_bp
from app.routes.api.server_management import server_manage_bp
from app.routes.api.server_configuration import server_config_bp
from app.routes.api.system_log import system_log_bp

frontend_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "../Frontend"))

app = Flask(__name__, template_folder=os.path.join(frontend_path, "pages"), static_folder=os.path.join(frontend_path, "static"))
app.secret_key = "your-very-secret-key"  # Replace with a secure, random string!

# Register Flask Blueprints
app.register_blueprint(login_bp)
app.register_blueprint(server_manage_bp)
app.register_blueprint(server_config_bp)
app.register_blueprint(system_log_bp)

@app.route("/")
def home():
    return render_template("login.html")

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=False, use_reloader=False)
