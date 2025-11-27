import paho.mqtt.client
import sys
import DistanceRepository from DistanceRepository
from LightRepository import LightRepository
from RainRepository import RainRepository
from SmokeRepository import SmokeRepository
from TemperatureRepository import TemperatureRepository
from PressureRepository import PressureRepository
from HumidityRepository import HumidityRepository

def on_connect(client, userdata, flags, rc):
    print('connected (%s)' % client._client_id)
    client.subscribe(topic = 'EQ8/dist/message', qos = 2)
    client.subscribe(topic = 'EQ8/light/message', qos = 2)
    client.subscribe(topic = 'EQ8/rain/message', qos = 2)
    client.subscribe(topic = 'EQ8/smoke/message', qos = 2)
    client.subscribe(topic = 'EQ8/temp/message', qos = 2)
    client.subscribe(topic = 'EQ8/press/message', qos = 2)
    client.subscribe(topic = 'EQ8/humid/message', qos = 2)

def on_message(client, userdata, message):
    print('---------------------')
    print('topic: %s' % message.topic)
    print('payload: %s' % message.payload)
    print('qos: %d' % message.qos)

    topic_str = message.topic.strip()

    payload_str = message.payload.decode('utf-8').strip()
    payload_float = float(payload_str)

    if (topic_str == 'EQ8/dist/message'):
        distance_repo = DistanceRepository()
        if (payload_float < 8190):
            distance_repo.insert_data(payload_float)
    elif (topic_str == 'EQ8/light/message'):
        light_repo = LightRepository()
        light_repo.insert_data(payload_float)
    elif (topic_str == 'EQ8/rain/message'):
        rain_repo = RainRepository()
        rain_repo.insert_data(payload_float)
    elif (topic_str == 'EQ8/smoke/message'):
        smoke_repo = SmokeRepository()
        smoke_repo.insert_data(payload_float)
    elif (topic_str == 'EQ8/temp/message'):
        temp_repo = TemperatureRepository()
        temp_repo.insert_data(payload_float)
    elif (topic_str == 'EQ8/press/message'):
        press_repo = PressureRepository()
        press_repo.insert_data(payload_float)
    elif (topic_str == 'EQ8/humid/message'):
        humid_repo = HumidityRepository()
        humid_repo.insert_data(payload_float)



  distance_repo = DistanceRepository()



def main():
    client = paho.mqtt.client.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect('test.mosquitto.org', port=1883, keepalive=60)
    client.loop_forever()

if __name__ == '__main__':
    main()

sys.exit(0)