import paho.mqtt.client
import sys
import json
from DistanceRepository import DistanceRepository
from LightRepository import LightRepository
from RainRepository import RainRepository
from SmokeRepository import SmokeRepository
from TemperatureRepository import TemperatureRepository
from PressureRepository import PressureRepository
from HumidityRepository import HumidityRepository

def on_connect(client, userdata, flags, rc):
    print('Connected to MQTT Broker (%s)' % client._client_id)
    #topic receives a JSON formatted message
    client.subscribe(topic = 'EQ8/sensors/data', qos = 1)
    print('Subscribed to EQ8/sensors/data')

def on_message(client, userdata, message):
    try:
        payload_str = message.payload.decode('utf-8').strip()
        print(f'Received: {payload_str}')
        
        # Parse JSON
        data = json.loads(payload_str)
        
        # Save all sensor data
        if 'distance' in data and data['distance'] > 0 and data['distance'] < 8190:
            DistanceRepository().insert_data(data['distance'])
            
        if 'light' in data:
            LightRepository().insert_data(data['light'])
            
        if 'rain' in data:
            RainRepository().insert_data(data['rain'])
            
        if 'smoke' in data:
            SmokeRepository().insert_data(data['smoke'])
            
        if 'temperature' in data:
            TemperatureRepository().insert_data(data['temperature'])
            
        if 'pressure' in data:
            PressureRepository().insert_data(data['pressure'])
            
        if 'humidity' in data:
            HumidityRepository().insert_data(data['humidity'])
            
        print('Data inserted into DB')
        
    except json.JSONDecodeError as e:
        print(f'Error JSON decoding: {e}')
    except Exception as e:
        print(f'Error: {e}')



def main():
    client = paho.mqtt.client.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect('test.mosquitto.org', port=1883, keepalive=60)
    client.loop_forever()

if __name__ == '__main__':
    main()

sys.exit(0)