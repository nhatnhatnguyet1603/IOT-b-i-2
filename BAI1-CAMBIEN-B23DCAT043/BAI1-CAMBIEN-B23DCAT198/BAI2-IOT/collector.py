import json
import paho.mqtt.client as mqtt


MQTT_BROKER = "localhost"
MQTT_PORT = 1883
MQTT_TOPIC = "iot/esp32/esp32_01/sensor"


def validate_data(data):
    required_fields = [
        "device_id",
        "temperature",
        "humidity",
        "sequence",
        "uptime_s"
    ]

    # Kiểm tra có đủ field không
    for field in required_fields:
        if field not in data:
            print(f"[INVALID] Missing field: {field}")
            return False

    # Kiểm tra kiểu dữ liệu
    if not isinstance(data["device_id"], str):
        print("[INVALID] device_id must be string")
        return False

    if not isinstance(data["temperature"], (int, float)):
        print("[INVALID] temperature must be number")
        return False

    if not isinstance(data["humidity"], (int, float)):
        print("[INVALID] humidity must be number")
        return False

    # Kiểm tra range cơ bản
    if not -40 <= data["temperature"] <= 80:
        print("[INVALID] temperature out of range")
        return False

    if not 0 <= data["humidity"] <= 100:
        print("[INVALID] humidity out of range")
        return False

    return True


def on_connect(client, userdata, flags, reason_code, properties=None):
    print("Connected to MQTT Broker")

    client.subscribe(MQTT_TOPIC)

    print(f"Subscribed to: {MQTT_TOPIC}")


def on_message(client, userdata, msg):
    print("\n----- MQTT MESSAGE -----")

    print("Topic:", msg.topic)

    try:
        # Chuyển bytes → string → JSON
        payload = msg.payload.decode("utf-8")
        data = json.loads(payload)

    except UnicodeDecodeError:
        print("[INVALID] Payload is not UTF-8")
        return

    except json.JSONDecodeError:
        print("[INVALID] Payload is not valid JSON")
        return

    print("Data:", data)

    if validate_data(data):
        print("[VALID] Data accepted")
    else:
        print("[INVALID] Data rejected")


client = mqtt.Client(
    mqtt.CallbackAPIVersion.VERSION2,
    client_id="python-collector"
)

client.on_connect = on_connect
client.on_message = on_message

print("Connecting to MQTT Broker...")

client.connect(
    MQTT_BROKER,
    MQTT_PORT,
    60
)

client.loop_forever()
