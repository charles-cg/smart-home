# python 3.11
import uuid
import json
from paho.mqtt import client as mqtt_client

# Import all repositories
from DistanceRepository import DistanceRepository
from TemperatureRepository import TemperatureRepository
from PressureRepository import PressureRepository
from LightRepository import LightRepository
from HumidityRepository import HumidityRepository
from RainRepository import RainRepository

broker = 'broker.mqtt.cool'
port = 1883
topic = "/EQ8/test/message"  # Changed to the topic Arduino publishes to
# Generate a Client ID with the subscribe prefix.
client_id = f'python-mqtt-client-{uuid.getnode()}'
# username = 'emqx'
# password = 'public'


def connect_mqtt() -> mqtt_client.Client:
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("✓ Connected to MQTT Broker!")
        else:
            print(f"✗ Failed to connect, return code {rc}")

    client = mqtt_client.Client(client_id=client_id)
    # client.username_pw_set(username, password)
    client.on_connect = on_connect
    client.connect(broker, port)
    return client


def subscribe(client: mqtt_client.Client):
    def on_message(client, userdata, msg):
        try:
            payload = msg.payload.decode('utf-8')
            print(f"📨 Received from `{msg.topic}`: {payload}")
            
            # Parse JSON data from Arduino
            data = json.loads(payload)
            
            # Save distance data
            if 'distance' in data and data['distance'] > 0:
                distance_repo = DistanceRepository()
                distance_repo.insert_data(data['distance'])
                print(f"  ✓ Distance: {data['distance']} mm")
            
            # Save temperature data
            if 'temperature' in data and data['temperature'] != -999:
                temp_repo = TemperatureRepository()
                temp_repo.insert_data(data['temperature'])
                print(f"  ✓ Temperature: {data['temperature']} °C")
            
            # Save pressure data
            if 'pressure' in data and data['pressure'] != -999:
                pressure_repo = PressureRepository()
                pressure_repo.insert_data(data['pressure'])
                print(f"  ✓ Pressure: {data['pressure']} hPa")
            
            # Save light data (LDR sensor)
            if 'light' in data:
                light_repo = LightRepository()
                light_repo.insert_data(data['light'])
                print(f"  ✓ Light: {data['light']}")

            if 'rain' in data:
                rain_repo = RainRepository()
                rain_repo.insert_data(data['rain'])
                print(f"  ✓ Rain: {data['rain']}")

            #if 'humidity' in data:
             #   humidity_repo = HumidityRepository()
              #  humidity_repo.insert_data(data['humidity'])
               # print(f"  ✓ Humidity: {data['humidity']}")
                
            print("  ✅ All data saved successfully!")
            
        except json.JSONDecodeError as e:
            print(f"  ✗ JSON parse error: {e}")
        except Exception as e:
            print(f"  ✗ Error processing message: {e}")

    client.subscribe(topic)
    client.on_message = on_message
    print(f"✓ Subscribed to topic: {topic}")


def run():
    print("🚀 Starting MQTT Client...")
    client = connect_mqtt()
    subscribe(client)
    print("👂 Listening for messages...")
    client.loop_forever()


if __name__ == '__main__':
    run()
