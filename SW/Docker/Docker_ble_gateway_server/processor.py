import json
import time
import os
from collections import deque
import redis
from dotenv import load_dotenv

from system_log import log_to_redis

import asyncio

load_dotenv()

WINDOW_SIZE = 20  # Only consider tags received within the last 20 seconds
MAX_FREQ = 10  # Maximum frequency for score calculation
W1 = 0.8  # Weight for RSSI in score calculation
W2 = 0.2  # Weight for frequency in score calculation
FREQ_THRESHOLD = 1
RSSI_THRESHOLD = -80

QUEUE_HOST = os.getenv("QUEUE_HOST", "redis")
QUEUE_PORT = int(os.getenv("QUEUE_PORT", 6379))

redis_client = redis.Redis(host=QUEUE_HOST, port=QUEUE_PORT, db=0)

#-------------------------------------------------------
# Global variables
#-------------------------------------------------------

gateways = {}

queue = asyncio.Queue()

#-------------------------------------------------------
# Class definitions
#-------------------------------------------------------

class Gateway:
    def __init__(self, gateway_id):
        self.gateway_id = gateway_id
        self.num_tags = 0
        self.tags = {}  # Dictionary to store tags {tag_id: Beacon object}

    # Add or update a beacon in the gateway
    def add_beacon(self, tag_id, rssi, timestamp, flag_timeout):
        if tag_id in self.tags:
            # Update existing tag's data
            self.tags[tag_id].update_data(rssi, timestamp, flag_timeout)
        else:
            # Create new tag and store it
            self.tags[tag_id] = Tag(tag_id, rssi, timestamp, flag_timeout)

    # Check and remove tags based on flag_timeout
    def remove_expired_tags(self):
        expired_tags = [tag_id for tag_id, tag in self.tags.items() if tag.flag_timeout == 0]

        # Remove expired tags
        for tag_id in expired_tags:
            print(f"Removing expired Tag: {tag_id} from Gateway {self.gateway_id}")
            log_to_redis(f"Removing expired Tag: {tag_id} from Gateway {self.gateway_id}", source="processor")
            del self.tags[tag_id]

        # Set remaining tags' flag_timeout to 0 for the next check
        for tag in self.tags.values():
            tag.flag_timeout = 0

class Tag:
    def __init__(self, tag_id, rssi, timestamp, flag_timeout):
        self.tag_id = tag_id
        self.rssi = rssi
        self.timestamp = timestamp
        self.flag_timeout = flag_timeout
        self.history = deque(maxlen=100)

    # Update the tag data
    def update_data(self, rssi, timestamp, flag_timeout):
        self.rssi = rssi
        self.timestamp = timestamp
        self.flag_timeout = flag_timeout
        self.history.append((rssi, timestamp))
        
    # Get data from the last WINDOW_SIZE seconds"
    def get_filtered_data(self, current_time):
        return [(r, t) for r, t in self.history if current_time - t <= WINDOW_SIZE]

#-------------------------------------------------------
# Store or update gateway status dynamically in Redis.
#-------------------------------------------------------
def update_gateway_status(gateway_id, status, ip="Unknown"):
    redis_client.hset("gateway_status", gateway_id, json.dumps({
        "status": status,
        "ip": ip,
        "last_seen": time.time()
    }))

#-------------------------------------------------------
# Calculate score to detect beacon.
#-------------------------------------------------------
def calculate_score(rssi_avg, freq):
    rssi_normalized = 100 - abs(rssi_avg)   # Convert RSSI into a positive normalized value
    freq_normalized = min(freq, MAX_FREQ) / MAX_FREQ    # Normalize frequency
    return (W1 * rssi_normalized) + (W2 * freq_normalized * 100)

#-------------------------------------------------------
# Process tag data to determine the nearest gateway
#-------------------------------------------------------
def process_tag(beacons_to_process):
    scores = {}
    current_time = int(time.time())  # Get current timestamp

    # Iterate over all gateways
    for gateway_id, gateway in gateways.items():
        for tag_id in list(gateway.tags.keys()):
            tag = gateway.tags[tag_id]
            filtered_data = tag.get_filtered_data(current_time)

            if len(filtered_data) < FREQ_THRESHOLD:
                continue  # Not enough data for evaluation

            rssi_avg = sum(r for r, _ in filtered_data) / len(filtered_data)

            if rssi_avg > RSSI_THRESHOLD:
                scores[tag_id] = scores.get(tag_id, {})  # Create entry if not exists
                scores[tag_id][gateway_id] = calculate_score(rssi_avg, len(filtered_data))
                update_gateway_status(gateway_id, "Online")

            else:
                print(f"Skipping {gateway_id} - RSSI too low: {rssi_avg}")
                log_to_redis(f"Skipping {gateway_id} - RSSI too low: {rssi_avg}", source="processor")

    if not scores:
        print(f"DEBUG: No gateways above threshold for any tag")
        return

    # Determine the nearest gateway per tag
    for tag_id, gateway_scores in scores.items():
        nearest_gw = max(gateway_scores, key=gateway_scores.get)  # Find best gateway
        detected_gateways = sorted(gateway_scores.keys(), key=lambda g: gateway_scores[g], reverse=True)

        print(f"Beacon {tag_id} detected at {nearest_gw} with score {gateway_scores[nearest_gw]}")
        log_to_redis(f"Beacon {tag_id} detected at {nearest_gw} with score {gateway_scores[nearest_gw]}", source="processor")

        beacon_entry = {
                "gateways": detected_gateways,  # List of detected gateways
                "rssi_scores": {gw: gateway_scores[gw] for gw in detected_gateways},  # RSSI scores
                "timestamp": current_time
            }

        redis_client.hset("beacon_state", tag_id, json.dumps(beacon_entry))

        detected_event_key = f"{tag_id}:{nearest_gw}"
        last_detected = redis_client.hget("beacon_last_event", detected_event_key)

        if last_detected is None or last_detected.decode() == "lost":
            redis_client.rpush("aws_queue", json.dumps({
                "event": "detected",
                "beacon_id": tag_id,
                "gateway": nearest_gw,
                "timestamp": current_time
            }))
            redis_client.hset("beacon_last_event", detected_event_key, "detected")

        # Send "lost" for all other gateways where this tag was previously detected
        for gw in gateway_scores:
            if gw != nearest_gw:
                lost_event_key = f"{tag_id}:{gw}"
                last_lost = redis_client.hget("beacon_last_event", lost_event_key)

                if last_lost is None or last_lost.decode() != "lost":
                    redis_client.rpush("aws_queue", json.dumps({
                        "event": "lost",
                        "beacon_id": tag_id,
                        "gateway": gw,
                        "timestamp": current_time
                    }))
                    redis_client.hset("beacon_last_event", lost_event_key, "lost")
                    log_to_redis(f"Beacon {tag_id} lost at {gw}", source="processor")
                    
#-------------------------------------------------------
# Soft timer to check flag_timeout and remove expired 
# tags every second
#-------------------------------------------------------
async def soft_timer():
    while True:
        await asyncio.sleep(30)  # Run every 30 seconds
        current_time = int(time.time())

        expired_tags = []
        for gateway_id, gateway in gateways.items():
            for tag_id, tag in list(gateway.tags.items()):
                # Handle `flag_timeout`
                if tag.flag_timeout == 1:
                    tag.flag_timeout = 0  # Reset it to 0 (next cycle will check again)
                elif tag.flag_timeout == 0:
                    expired_tags.append((gateway_id, tag_id))  # Remove this tag

        # Remove expired tags
        for gateway_id, tag_id in expired_tags:
            print(f"Removing expired Tag: {tag_id} from Gateway {gateway_id}")
            log_to_redis(f"Removing expired Tag: {tag_id} from Gateway {gateway_id}", source="processor")

            last_event = redis_client.hget("beacon_last_event", tag_id)

            # Only log "lost" if the last event was "detected"
            if last_event is None or last_event.decode() == "detected":
                redis_client.rpush("aws_queue", json.dumps({
                    "event": "lost",
                    "beacon_id": tag_id,
                    "gateway": gateway_id,
                    "timestamp": current_time
                }))
                redis_client.hset("beacon_last_event", tag_id, "lost")  # Store last event

            # Remove from Redis storage
            redis_client.hdel("beacon_state", tag_id)
            del gateways[gateway_id].tags[tag_id]
            
#-------------------------------------------------------
# Queue processor to process beacons in batches
#-------------------------------------------------------
async def process_queue():
    while True:
        await asyncio.sleep(1)  # Process every second

        beacons_to_process = []
        while not queue.empty():
            beacons_to_process.append(await queue.get())

        if beacons_to_process:
            print(f"Processing {len(beacons_to_process)} beacons...")
            process_tag(beacons_to_process)  # Send batch to `process_tag()`

#-------------------------------------------------------
# Main function to receive beacons from Redis queue
# Also create 
#-------------------------------------------------------
async def main():
    while True:
        beacon_json = redis_client.lpop("beacon_data")  # Get the beacon data from queue
        
        if beacon_json is None:
            await asyncio.sleep(0.5)   # Wait before checking again
            continue 
        try: 
            beacon_data = json.loads(beacon_json)
            
            gateway_id = beacon_data["gateway_id"]
            tag_id = beacon_data["tag_id"]
            rssi = beacon_data["rssi"]
            timestamp = beacon_data["timestamp"]
            flag_timeout = beacon_data["flag_timeout"]

            # Check if the gateway exists
            if gateway_id not in gateways:
                gateways[gateway_id] = Gateway(gateway_id) #Create a new gateway object

            # Add or update the beacon in the gateway
            gateways[gateway_id].add_beacon(tag_id, rssi, timestamp, flag_timeout)

            await queue.put(beacon_data)

        except json.JSONDecodeError:
            print(f"Received non-JSON message: {beacon_json}")
        except Exception as e:
            print(f"Error processing message: {e}")


if __name__ == "__main__":
    loop = asyncio.get_event_loop()
    loop.create_task(main())  # Start main() to receive beacons
    loop.create_task(process_queue())  # Start queue processor
    loop.create_task(soft_timer())  # Start soft timer

    loop.run_forever()