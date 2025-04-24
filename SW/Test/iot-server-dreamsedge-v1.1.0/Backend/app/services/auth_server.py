import time
import app.environment.environment_manager as environment_manager
import app.services.http_client as http_client

# --------------------------------------------------
# Handles authentication process and updates session
# --------------------------------------------------
def authenticate_user(username, password):

    auth_token = http_client.login_access_token(username, password)

    if auth_token:
        environment_manager.update_auth_token(auth_token)

        print(f"[INFO] AUTH_TOKEN received for user '{username}': {auth_token}")
        return True, "/server_configuration"
    else:
        environment_manager.update_auth_token(None)
        print(f"[ERROR] Authentication failed: Invalid username/password for '{username}'")
        return False, "/server_configuration"  # Redirect back to login page
        
def facility_list():
    auth_token = environment_manager.get_auth_token()

    if not auth_token:
        print("[ERROR] Authentication token is missing. Cannot fetch facilities.")
        return None

    facilities = http_client.get_facility_list()

    if facilities:
        environment_manager.update_facility_list(facilities)
        print("[INFO] Successfully retrieved facility list.")
        return facilities   
    else:
        print("[ERROR] No facilities found or API request failed.")
        return None

    