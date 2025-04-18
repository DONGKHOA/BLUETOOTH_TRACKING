from datetime import datetime

# Get local time
local_time = datetime.now()


print("Local Time:", local_time)

formatted_time = local_time.strftime("%Y-%m-%d %H:%M:%S")
print("Formatted Local Time:", formatted_time)
