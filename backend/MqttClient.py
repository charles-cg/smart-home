import paho.mqtt.client as mqtt
import json
from DistanceRepository import DistanceRepository
import threading
import time

class MqttClientHandler:
    def __init__(self, broker_address="broker.mqtt.cool", port=1883):
        self.broker_address = broker_address
        self.port = port
        self.client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1)
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        self.client.on_disconnect = self.on_disconnect
        self.is_connected = False
        
    def on_connect(self, client, userdata, flags, rc):
        """Callback when client connects to broker"""
        if rc == 0:
            print("✓ Connected to MQTT Broker!")
            self.is_connected = True
            # Subscribe to the distance topic from Arduino
            client.subscribe("/EQ8/test/message")
            print("✓ Subscribed to /EQ8/test/message")
        else:
            print(f"✗ Failed to connect, return code {rc}")
            self.is_connected = False
    
    def on_message(self, client, userdata, msg):
        """Callback when message is received from broker"""
        try:
            payload = msg.payload.decode('utf-8')
            print(f"Message received on topic {msg.topic}: {payload}")
            
            # Parse the distance value from Arduino message
            # Arduino sends: "Distance in (mm)XXX" where XXX is the distance value
            if "Distance" in payload:
                # Extract the numeric value
                distance_str = payload.replace("Distance in (mm)", "").strip()
                distance_value = float(distance_str)
                
                # Save to database
                distance_repository = DistanceRepository()
                distance_repository.insert_data(distance_value)
                print(f"✓ Distance data saved: {distance_value} mm")
        except Exception as e:
            print(f"✗ Error processing message: {e}")
    
    def on_disconnect(self, client, userdata, rc):
        """Callback when client disconnects from broker"""
        self.is_connected = False
        if rc != 0:
            print(f"✗ Unexpected disconnection. Return code: {rc}")
        else:
            print("Disconnected from MQTT Broker")
    
    def connect(self):
        """Connect to MQTT broker"""
        try:
            print(f"Connecting to MQTT Broker at {self.broker_address}:{self.port}...")
            self.client.connect(self.broker_address, self.port, 60)
            self.client.loop_start()
            print("✓ MQTT client loop started")
        except Exception as e:
            print(f"✗ Error connecting to MQTT broker: {e}")
    
    def disconnect(self):
        """Disconnect from MQTT broker"""
        try:
            self.client.loop_stop()
            self.client.disconnect()
            print("MQTT client disconnected")
        except Exception as e:
            print(f"✗ Error disconnecting: {e}")

# Global MQTT client instance
mqtt_handler = None

def initialize_mqtt():
    """Initialize MQTT client"""
    global mqtt_handler
    mqtt_handler = MqttClientHandler()
    mqtt_handler.connect()
    return mqtt_handler

def get_mqtt_handler():
    """Get the global MQTT handler instance"""
    global mqtt_handler
    return mqtt_handler
