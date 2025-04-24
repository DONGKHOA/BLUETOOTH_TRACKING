import json
import os

CONFIG_FILE = "configs.json"
configs = {}

def load_configs():
    global configs
    if os.path.exists(CONFIG_FILE):
        try:
            with open(CONFIG_FILE, "r") as f:
                configs = json.load(f)
        except Exception:
            configs = {}
    else:
        configs = {}

def save_configs():
    with open(CONFIG_FILE, "w") as f:
        json.dump(configs, f, indent=2)